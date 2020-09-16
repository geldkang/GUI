#include "gui.h"
#include <wx/listbox.h>

char* waddr;
vector<wifi::wifi_info> vec_wi;

int select_dongle();
int AP_scan(char* ifname, wxListBox *dev_list);

int select_dongle()
{
	printf("\n[+] Please select the Wi-Fi dongle to send.\n");
	printf("[+] The dongle should support MONITORING mode.\n\n");

	interface_scan();

	if(adapters.size() < 1)
	{
		printf("[-] No Wi-Fi dongle (lan card) found. Please connect the dongle.\n");
		return 0;
	}

	//int device_num = -1;
	int device_num = 0; //use pre-determined adapter only
	//int device_num = 0; //for raspberry pie

	// display wireless adapters and select
	printf("	Your Wi-Fi Device List\n");
	printf("	[No.]\t\t[Device name]\n");
	for (int i = 0; i < adapters.size(); i++)
		printf("	%02d\t\t%s\n", i, adapters[i].c_str());
	
	/*printf("	Total : %ld\n\n", adapters.size());
	printf("\tSelect device: ");
	scanf("%d", &device_num);
	while (device_num < 0 || device_num >= adapters.size())
	{
		printf("\n\t[-] Wrong device number. Please select agian.\n");
		printf("\tSelect device: ");
		scanf("%d", &device_num);
	}*/

	waddr = (char*)adapters[device_num].c_str();

	memset(&adapters,0,sizeof(adapters));//initialize adapter list for next scan
	return 0;
}

int AP_scan(char* ifname, wxListBox *dev_list)
{
	//dprintf("\n[+] Changing interface mode to managed mode...\n");
	if (set_mode(ifname, "managed")){
		printf("[-] Error while changing to interface mode \"managed\".\n");
		printf("[-] Please check your dongle.\n");
		return -1;
	}
	printf("\n");
	printf("[+] Interface's mode has changed to managed mode\n");
	
	/* start AP scanning */
	printf("\n");
	printf("[+] Start scanning APs (%s)...\n", ifname);

	// FOR FAST TESTING	
	// wifi::wifi_info tmp;
	// tmp.ESSID = "ccs_iptime_test";
	// tmp.channel = 3;
	// tmp.setaddr((char*)"88:36:6C:21:52:AA");

	// vec_wi.push_back(tmp);

	memset(&vec_wi,0,sizeof(vec_wi));//initialize AP list for next scan
	
	wifi::wifi_scan(ifname, vec_wi);
	if (vec_wi.size() == 0) {
	fprintf(stderr, "[-] Cannot find any APs. Please try again.\n");
	printf("[ ] Evenif APs are nearby, it might happen several times... Take your time.\n");
	printf("[ ] (Hint) How about eject your dongle and re-insert?\n");
	return -1;
	}

	/* show result */	
	for (int i = 0; i < vec_wi.size(); i++)
		dev_list->Append(vec_wi[i].ESSID.c_str());
	return 0;
}

BEGIN_EVENT_TABLE(gui_scan, wxFrame)
EVT_BUTTON(ID_FUZZ, gui_scan::OnFuzz)
EVT_BUTTON(ID_QUIT, gui_scan::OnQuit)
END_EVENT_TABLE()

gui_scan::gui_scan(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title)
{

	wxPanel* panel = new wxPanel(this, -1);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	//wxTextCtrl *dev_list = new wxTextCtrl(panel, wxID_ANY, wxT(""),wxPoint(-1, -1), wxSize(-1, -1), wxTE_MULTILINE | wxTE_READONLY);
	dev_list = new wxListBox(panel, wxID_ANY, wxPoint(-1, -1), wxSize(-1, -1));
	hbox->Add(dev_list, 1, wxEXPAND | wxALL, 10);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxButton* fuzz = new wxButton(panel, ID_FUZZ, "&Fuzz");
	vbox->Add(fuzz, 1, wxEXPAND | wxALL, 10);
	wxButton* quit = new wxButton(panel, ID_QUIT, "&Quit");
	vbox->Add(quit, 1, wxEXPAND | wxALL, 10);

	hbox->Add(vbox, 1, wxEXPAND, 10);

	panel->SetSizer(hbox);
	Centre();
	select_dongle();
	AP_scan(waddr, dev_list);
}

gui_scan::~gui_scan()
{
}
void onSelect(wxCommandEvent& event)
{

}
void gui_scan::OnQuit(wxCommandEvent& event)
{
	wxFrame* frame = new gui_frame("Main");
	//frame->Show(true);
	frame->ShowFullScreen(true);
	this->Close();
}

void gui_scan::OnFuzz(wxCommandEvent& event)
{
	int i;
	for(i = 0; i < vec_wi.size(); i++)
	{
		if(dev_list->IsSelected(i))
			break;
	}
	wxFrame *fuzz_window = new gui_fuzz("Fuzz", waddr, vec_wi, i);
	//fuzz_window->Show(TRUE);
	fuzz_window->ShowFullScreen(true);
	this->Close();
}
