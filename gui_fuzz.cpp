#include "gui.h"
#include <wx/listbox.h>

#include "gui_Fuzzer.h"
#include "dprint.h"
#include "packet.h"
#include "global.h"


namespace wifi
{
void pp(wifi::packet p);
int debug_pp(wifi::packet p);

Fuzzer::Fuzzer(sender &s)
	: s(s), schler(new scheduler(s.states.numstate)), prev_state(-1)
{
	//schler(s.ss.numstate);
	this->set_default_opt();
}

Fuzzer::~Fuzzer()
{
	delete schler;
}

int Fuzzer::set_default_opt()
{
	/* default option values are defined in global.h */
	opt_sleep_ms = DEFAULT_SLEEP_MS;
	opt_iteration = DEFAULT_ITERATION;
}

int Fuzzer::replay() throw (std::runtime_error)
{
	//char payload[PAYLOAD_SIZE];
	//int payld_size;
	static int timeout_count = 0;

	print_start();
	
	if (opt_sleep_ms)
		set_timespec();
		
	if (s.open())
	{
		fprintf(stderr, "[-] s.open() failed\n");
		print_end();
		throw std::runtime_error("[-] s.open() failed");
	}

	// Get the packet
	packet pkt;
	int start_state = 0;
	int pkt_id = 10;
	pkt = igen.getReplayInput(s.proto, start_state, pkt_id);

	if (pkt.id == -1)
		dprint("[ ] getReplayInput(%d, %d): pkt.id==-1\n", s.proto, pkt_id);
	
	// pkt's option must be set in here
	if (pkt.head.size() < 5)
		dprint("[-] pkt parsing error?\n");
	else if (!s.proto)
	{ // AP
		pkt.head[2].value.assign(this->s.wi.addr, this->s.wi.addr + 6);
		pkt.head[3].value.assign(this->s.i.addr, this->s.i.addr + 6);
		pkt.head[4].value.assign(this->s.wi.addr, this->s.wi.addr + 6);
	}
	else
	{ // STA
		if (pkt.head[0].value[0] != 0x80)
			pkt.head[2].value.assign(this->s.wi.addr, this->s.wi.addr + 6);
		pkt.head[3].value.assign(this->s.i.addr, this->s.i.addr + 6);
		pkt.head[4].value.assign(this->s.i.addr, this->s.i.addr + 6);
	}

	std::vector<uint8_t> tmp = pkt.get();
	dprint("replay packet : %x\n\n\n", tmp);
	return 0;

	s.send((char*)&tmp[0], tmp.size(), 0);

	if (s.cchk(0)) // crash check
	{
		s.close();
		
		throw crash_found("crash. cchk() failed", start_state, pkt);
	}
	else
	{
		dprint("[ ] @@@@@@@@ Target is alive.\n");
	}
}

int Fuzzer::fuzz(gui_fuzz* gui_fuzz) throw (std::runtime_error)
{
	char payload[PAYLOAD_SIZE];
	int payld_size;
	static int timeout_count = 0;
	int start_state = -1;
	
	print_start();
	
	if (opt_sleep_ms)
		set_timespec();
		
	if (s.open())
	{
		fprintf(stderr, "[-] s.open() failed\n");
		print_end();
		throw std::runtime_error("[-] s.open() failed");
		//return -1;
	}
	
	timeout_count = 0; // timeout(crash occur)->fuzz finish->fuzz again... then?

	for (int i = 0; i < opt_iteration; i++)
	{
		//bool is_first = true;
		pkt_cnt += 1;

		gui_fuzz->packet_count = pkt_cnt;
		wxQueueEvent(gui_fuzz->GetEventHandler(), new wxThreadEvent());

		if (s.proto == 1)
			start_state = select_start_state(); // sta
		else
			start_state = schler->next(); // ap - auto scheduling
		
		packet pkt;
		vpkt = false;
		pkt = igen.getInput(s.proto, start_state, payload, &payld_size, this->s.wi.addr, pkt_cnt, start_state);

		if (pkt.id == -1)
			dprint("[ ] getInput(%d, %d): pkt.id==-1\n", s.proto, start_state);
		
		// pkt's option must be set in here
		if (pkt.head.size() < 5)
			dprint("[-] pkt parsing error?\n");
		else if (!s.proto)
		{ // AP
			pkt.head[2].value.assign(this->s.wi.addr, this->s.wi.addr + 6);
			pkt.head[3].value.assign(this->s.i.addr, this->s.i.addr + 6);
			pkt.head[4].value.assign(this->s.wi.addr, this->s.wi.addr + 6);
		}
		else
		{ // STA
			if (pkt.head[0].value[0] != 0x80)
				pkt.head[2].value.assign(this->s.wi.addr, this->s.wi.addr + 6);
			pkt.head[3].value.assign(this->s.i.addr, this->s.i.addr + 6);
			pkt.head[4].value.assign(this->s.i.addr, this->s.i.addr + 6);
		}

		std::vector<uint8_t> temp_pkt = pkt.get();

		// _fprint_payld((char*)&temp_pkt[0], temp_pkt.size(), pkt.msg); // make a log file in current directory

		int debug_pp(packet p);
		int print_payload(char* payld, int size);
		dprint("mutated packet: \n");
		debug_pp(pkt);

		printf("\n[ ] Try transition to start state: %d.\n", start_state);
		try
		{
			s.state_transition(start_state, !s.proto);

			gui_fuzz->state_curr = s.current_state;
			wxQueueEvent(gui_fuzz->GetEventHandler(), new wxThreadEvent());

			std::vector<uint8_t> tmp = pkt.get();
			if (tmp.size())
			{
				//printf("[+] Try sending mutated %s packet in state %d.\n", pkt.msg.c_str(), start_state);
				//printf("[+] Sending packet data (Before send())\n");
				//print_payload((char*)&tmp[0], tmp.size());
				
				s.send((char*)&tmp[0], tmp.size(), 0);
				prev_pkt = pkt; // save previous packet
				prev_state = start_state; // save previous state
			}
			else
				dprint("[-] tmp.size() is zero.\n");

			dprint("[+] Packet sending success.\n");

			if (s.cchk(start_state)) // crash check
			{
				s.close();
				if (prev_pkt.id == -1 || prev_state == -1)
					throw std::runtime_error("crash w/o packet. cchk() failed");
				
				_fprint_payld((char*)&prev_pkt.get()[0], prev_pkt.get().size(), prev_pkt.msg, 1, pkt_cnt, start_state); // make a log file in current directory

				packet tmp_packet = prev_pkt;
				int tmp_state = prev_state;
				prev_pkt.id = -1; // clear prev_pkt
				prev_state = -1; // clear prev_state
				
				throw crash_found("crash. cchk() failed", tmp_state, tmp_packet);
				//printf("[+] ****** NO RESPONSE *******\n");
				//std::vector<uint8_t> tmp = pkt.get();
				//print_payload((char*)&tmp[0], tmp.size());
				//break;
			}
			else
			{
				_fprint_payld((char*)&tmp[0], tmp.size(), pkt.msg, 0, pkt_cnt, start_state); // make a log file in current directory

				dprint("[ ] @@@@@@@@ Target is alive.\n");
			}
						
			timeout_count = 0;
			
			if (s.proto == 1)
			{
				s.state_transition(0);

				gui_fuzz->state_curr = s.current_state;
				wxQueueEvent(gui_fuzz->GetEventHandler(), new wxThreadEvent());

			
			}
		}
		catch (state_check_timeout &e)
		{
			if (s.cchk(start_state))
			{
				s.close();
				if (prev_pkt.id == -1 || prev_state == -1) 
					throw std::runtime_error("crash w/o packet. cchk() failed");
				_fprint_payld((char*)&prev_pkt.get()[0], prev_pkt.get().size(), prev_pkt.msg, 1, pkt_cnt, start_state); // make a log file in current directory				

				packet tmp_packet = prev_pkt;
				int tmp_state = prev_state;
				prev_pkt.id = -1;
				prev_state = -1;
				throw crash_found("crash. cchk() failed", tmp_state, tmp_packet);
			}
			
			dprint("[-] state_check_timeout, state: %d\n", e.s);
			dprint("    [ ");
			while(!e.transition_queue.empty())
			{
				dprint("%d ", e.transition_queue.front());
				e.transition_queue.pop();
			}
			dprint("]\n");
			if (timeout_count >= 1)
			{
				s.close();
				if (prev_pkt.id == -1 || prev_state == -1)
					throw std::runtime_error("timeout w/o packet");
				
				packet tmp_packet = prev_pkt;
				int tmp_state = prev_state;
				prev_pkt.id = -1; // clear prev_pkt
				prev_state = -1;
				throw crash_found("crash. timeout twice", tmp_state, tmp_packet);
			}
			timeout_count++;
		}
		catch (st_error &e)
		{
			fprintf(stderr, "[-] st_error: %s\n", e.what());
			fprintf(stderr, "    %d, [ ", e.s);
			while (!e.transition_queue.empty())
			{
				fprintf(stderr, "%d ", e.transition_queue.front());
				e.transition_queue.pop();
			}
			fprintf(stderr, "]\n");
		}
		
		//printf("    payld_size: %d\n", payld_size);
		
		//memcpy(prev_payload, payload, payld_size);
		//prev_size = payld_size;
		
		if (opt_sleep_ms)
			sleep_ms();
	}
	printf("\n");
	print_end();
	s.close();
	
	//if (prev_pkt.id == -1 || prev_state == -1)
	//	throw std::runtime_error("informing w/o packet");
	//throw informing_packet("informing", prev_state, prev_pkt);
	
	return 0;
}

int Fuzzer::select_start_state()
{
	static int state = -1;
	int ret;
	
	s._bchk(0, 0);
	if ((ret = s._chk(0, 0)) <= 1)
	{
		if (ret < 1)
			dprint("chk less then 1\n");
		s.current_state = 1;
		return 1;
	}
	
	s.current_state = 2;
	state = ((state + 1) % 3);
	
	return (state + 2);
}

int _fprint_payld(char* payld, int size, std::string type, int chk, int pkt_no, int state)
{
	if (!fi)
	{
		fprintf(stderr, "[-] Failed to open log file\n");
		return -1;
	}

	if (is_print_origin || pkt_no != 1){
		fprintf(fi, ",\n");
		is_print_origin = false;
	}
	fprintf(fi, "\t\t{\"no\" : %d, ", pkt_no);
	if(vpkt==true) {
		fprintf(fi, "\"org\" : \"k\", ");
	}
	else {
		fprintf(fi, "\"org\" : \"n\", ");
	}
	fprintf(fi, "\"state\" : %d, ", state); // state number
	if(chk==1) {
		fprintf(fi, "\"crash\" : \"y\" , ");
	} else {
		fprintf(fi, "\"crash\" : \"n\" , ");
	}

	fprintf(fi, "\"payload\" : {");
	
	fprintf(fi, "\"type\" : \"0x%02x\" , ", (unsigned char)payld[0]);
	fprintf(fi, "\"flags\" : \"0x%02x\" , ", (unsigned char)payld[1]);\
	fprintf(fi, "\"duration\" : \"%d\" , ", (((int)(((unsigned char)payld[3])&0xFF) << 8) +  (int)(((unsigned char)payld[2])&0xFF)));
	fprintf(fi, "\"dest\" : \"%02x:%02x:%02x:%02x:%02x:%02x\" , ", (unsigned char)payld[4], (unsigned char)payld[5], 
		(unsigned char)payld[6], (unsigned char)payld[7], (unsigned char)payld[8], (unsigned char)payld[9]);
	fprintf(fi, "\"source\" : \"%02x:%02x:%02x:%02x:%02x:%02x\" , ", (unsigned char)payld[10], (unsigned char)payld[11], 
		(unsigned char)payld[12], (unsigned char)payld[13], (unsigned char)payld[14], (unsigned char)payld[15]);
	fprintf(fi, "\"bss_id\" : \"%02x:%02x:%02x:%02x:%02x:%02x\" , ", (unsigned char)payld[16], (unsigned char)payld[17], 
		(unsigned char)payld[18], (unsigned char)payld[19], (unsigned char)payld[20], (unsigned char)payld[21]);
	fprintf(fi, "\"seq\" : \"%d\" , " , (((int)(((unsigned char)payld[23])&0xFF) << 4) + ((int)(((unsigned char)payld[22])&0xF0) >> 4)));
	fprintf(fi, "\"frag\" : \"%d\" , " , (int)((unsigned char)payld[22])&0x0F);
	fprintf(fi, "\"data\" : \"0x");
	for (int data=24; data < size; data++) {
		fprintf(fi, "%02x", (unsigned char)payld[data]);
	}
	fprintf(fi, "\"");
	fprintf(fi, "}");

	// for (int i = 0; i < size; i++)
	// {
	// 	if (i % 16 == 0)
	// 		fprintf(fi, "[+] ");
	// 	fprintf(fi, "%02X ", (unsigned char)payld[i]);
	// 	if (i % 16 == 15)
	// 		fprintf(fi, "\n");
	// }

	fprintf(fi, "}"); // end of packet object
}

int Fuzzer::print_payload(char* payld, int size)
{
	printf("[+] %s might be crashed. Check it out\n", this->s.wi.ESSID.c_str());
	//printf("[+] size: %d\n", size); // testcode
	for (int i = 0; i < size; i++)
	{
		if (i % 16 == 0)
			printf("[+] ");
		printf("%02X ", (unsigned char)payld[i]);
		if (i % 16 == 15)
			printf("\n");
	}
	printf("\n\n");
	return 0;
}

int Fuzzer::set_timespec()
{
	ts.tv_sec = opt_sleep_ms / 1000;
	ts.tv_nsec = (opt_sleep_ms % 1000) * 10e6;
}

int Fuzzer::sleep_ms()
{
	return nanosleep(&ts, NULL);
}

int Fuzzer::print_start()
{
	if(is_replay){
		printf("-----------------------------------------------------------\n");
		printf("[+] Replay start <%s>...\n", this->s.wi.ESSID.c_str());
	}
	else{
		printf("-----------------------------------------------------------\n");
		printf("[+] Fuzzing start <%s>...\n", this->s.wi.ESSID.c_str());
	}

}

int Fuzzer::print_end()
{
	printf("[+] Fuzzing end\n");
	printf("-----------------------------------------------------------\n");
	printf("\n");
}

int Fuzzer::setSchler(scheduler *sch)
{
	schler = sch;
}

scheduler* Fuzzer::getSchler()
{
	return schler;
}
} // namespace wifi


















BEGIN_EVENT_TABLE(gui_fuzz, wxFrame)
EVT_BUTTON(ID_QUIT, gui_fuzz::OnQuit)
END_EVENT_TABLE()

gui_fuzz::gui_fuzz(const wxString& title, char* ifname, vector<wifi::wifi_info> vec_wi, int AP)
	: wxFrame(NULL, wxID_ANY, title)
{	
	packet_count = 0;
	crash_count = 0;
	state_curr = 0;
	Bind(wxEVT_THREAD, &gui_fuzz::OnThreadUpdate, this);

	wxPanel *panel = new wxPanel(this, -1);

    	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

    	wxBoxSizer *hbox1 = new wxBoxSizer(wxHORIZONTAL);
    	wxStaticText *status =  new wxStaticText(panel, wxID_ANY,wxT("Status"));
    	hbox1->Add(status, 0, wxRIGHT, 10);
	statusbar = new wxStaticText(panel, wxID_STATIC, "\t0", wxPoint(-1, -1), wxSize(-1, -1));
    	hbox1->Add(statusbar, 1);
    	vbox->Add(hbox1, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);

    	wxBoxSizer *hbox2 = new wxBoxSizer(wxHORIZONTAL);
	crashpacket = new wxListBox(panel, wxID_ANY, wxPoint(-1, -1), wxSize(-1, -1));
    	hbox2->Add(crashpacket, 1, wxEXPAND | wxALL, 10);

    	wxBoxSizer *vbox2 = new wxBoxSizer(wxVERTICAL);
	
    	packetnumber = new wxStaticText(panel, wxID_STATIC, "\tcrash : 0\n\t\t/\n\ttotal : 0", wxPoint(-1, -1), wxSize(-1, -1));

    	vbox2->Add(packetnumber, 1, wxEXPAND | wxALL, 10);
    	wxButton* stop = new wxButton(panel, ID_QUIT, "&Stop");
    	vbox2->Add(stop, 1, wxEXPAND | wxALL, 10);

    	hbox2->Add(vbox2, 1, wxEXPAND, 10);

    	vbox->Add(hbox2, 1, wxALL | wxEXPAND, 10);

    	panel->SetSizer(vbox);

    	Centre();

	_ifname = ifname;
	_vec_wi = vec_wi;
	_AP = AP;

	
	printf("wait for creating thread...\n\n");
	if (CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR)
	{
		printf("failed to start process\n\n");
		return;
	}
	if (GetThread()->Run() != wxTHREAD_NO_ERROR) //goto exitcode entry
	{
		printf("failed to run process\n\n");
	        return;
	}
}

gui_fuzz::~gui_fuzz()
{
}

void gui_fuzz::OnQuit(wxCommandEvent& event)
{
	//sigint_handler(SIGINT);
	//if (GetThread() && GetThread()->IsRunning())
	
		fprintf(fi, "\n\t]\n}");
		fclose(fi);
		signal(SIGINT, SIG_DFL);

		printf("[+] Fuzzing end\n");
		printf("-----------------------------------------------------------\n");
		printf("\n");
        	//GetThread()->Wait();
	
	wxFrame* frame = new gui_frame("Main");
	//frame->Show(true);
	frame->ShowFullScreen(true);
	this->Close();
    	GetThread()->Kill();
}

wxThread::ExitCode gui_fuzz::Entry()
{
	// select AP
	int ap_num = _AP;

	// Register signal (stop signal : Ctrl + C)
	signal(SIGINT, sigint_handler); 

	printf("\n[+] Wait for the Fuzzing to be started...\n");
	printf("[+] Please hit \"Ctrl + C\" to finish...\n");
	
	wifi::interface iface(_ifname);
	wifi::sender_ap psta(iface, _vec_wi[ap_num]);
	
	// Automatic channel setting
	set_channel(_ifname, _vec_wi[ap_num].channel);

	// dprint("[+] Changing interface mode to monitor mode...\n");
	if (set_mode(_ifname, "monitor")){
		wxMessageDialog *dial = new wxMessageDialog(NULL, wxT("Error while changing to interface mode\nPlease check your dongle."), wxT("Info"), wxOK);
  		dial->ShowModal();
		wxFrame* frame = new gui_frame("Main");
		//frame->Show(true);
		frame->ShowFullScreen(true);
		this->Close();

	}
	printf("[+] Interface's mode has changed to monitor mode\n");

	pkt_cnt = 0;
	wifi::Fuzzer fuzz(psta);

	//If replay, just lookup the packets and send the replay packet
	if(is_replay == true){
		fuzz.replay();
		wxFrame* frame = new gui_frame("Main");
		//frame->Show(true);
		frame->ShowFullScreen(true);
		this->Close();
	}

	//Fuzzer fuzz(s); // sender inheritance
	int file_count = 0;

	const char* logfile_name = get_logfile_name().c_str();
	fi = fopen(logfile_name, "w");
	fprintf(fi,"{\n\t\"toolVer\" : \"%s\", \n\t\"interface\" : \"Wi-Fi\", \n\t\"waddr\" : \"%s\",", VERSION, _vec_wi[ap_num].szaddr.c_str());
	//fprintf(fi,"{\n\t\"toolVer\" : \"%s\", \n\t\"interface\" : \"Wi-Fi\", \n\t\"waddr\" : \"%s\",", VERSION, tmp.szaddr.c_str());
	fprintf(fi, "\n\t\"starting_time\" : ");
	getTime();
	fprintf(fi, " ,\n\t\"protocol\" : \"802.11_AP\"");
	fprintf(fi, " ,\n\t\"packet\" : [\n");

//wxMessageDialog *dialog = new wxMessageDialog(NULL, wxT("test"), wxT("Info"), wxOK);
//dialog->ShowModal();

	for (int cnt = 0; cnt < 2; cnt++)
	{
		try
		{
			fuzz.fuzz(this);
		}
		catch (wifi::crash_found &e)
		{			
			printf("\n[+] ******** Crash found (%s) ***********\n", e.what());
			printf("[+] state: %d, proto: %d, id: %d\n", e.s, e.p.proto, e.p.id);
			printf("[+] Crash packet\n");

			memset(&buf, 0, sizeof(buf));
			sprintf(buf,"[+] Crash packet\n[+] state: %d, proto: %d, id: %d", e.s, e.p.proto, e.p.id);
			crashpacket->Append(buf);

			crash_count += 1; //count crashed packet number
			wxQueueEvent(GetEventHandler(), new wxThreadEvent());

			//printf("proto: %d\n", fuzz.s.proto);
			wifi::pp(e.p);
			
			char org_pkt[1024];
			int size;
			wifi::Database db;
			db.open();
			
			/* db.get_original(e.p.proto, e.p.id, org_pkt, &size);
			printf("[+] size: %d\n", size);
			for (int i = 0; i < size; i++)
			{
				if (i % 16 == 0)
			 		printf("[+] ");
			 	printf("%02X ", (unsigned char)org_pkt[i]);
			 	if (i % 16 == 15)
			 		printf("\n");
			}*/

			db.close();
		}
		catch (std::runtime_error &e)
		{
			printf("[-] ******* runtime_error **********\n");
			printf("what(): %s\n", e.what());
		}
	}
	
	fprintf(fi, "\n\t]\n}");
	printf("[+] fuzzing finished\n");
	fclose(fi);
	return (wxThread::ExitCode)0;
}

void gui_fuzz::OnThreadUpdate(wxThreadEvent& evt)
{
	memset(&buf, 0, sizeof(buf));
	if(state_curr == 0)
		sprintf(buf,"\t\t\t%d", state_curr);
	else if(state_curr == 1)
		sprintf(buf,"\t\t\t\t\t%d", state_curr);
	else if(state_curr == 2)
		sprintf(buf,"\t\t\t\t\t\t\t%d", state_curr);
	else if(state_curr == 3)
		sprintf(buf,"\t\t\t\t\t\t\t\t\t%d", state_curr);
	else
		sprintf(buf,"\t%d", state_curr); //state exception
	statusbar->SetLabel(buf);
	
	memset(&buf, 0, sizeof(buf));
	//sprintf(buf,"\tcrash : %d\t",crash_count);
	sprintf(buf,"\tcrash : %d\t\n\t\t/\n\ttotal : %d\t",crash_count, packet_count);
	packetnumber->SetLabel(buf);
	packetnumber->Refresh();
}
