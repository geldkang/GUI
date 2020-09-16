#include "gui.h"

#include "dprint.h"
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <ctime>

#include "global.h"
#include "interface_scan.h"
#include "wifi_info.h"
#include "wifi_scan.h"
#include "replayer.h"
//#include "sender.h" // sender inheritance

#include "sender_sta.h"
#include "sender_ap.h"
#include "interface.h"
#include "gui_Fuzzer.h"
#include "Database.h"
#include "dprint.h"
#include "exceptions.h"

int pkt_cnt;
FILE* fi;
bool is_replay;

IMPLEMENT_APP(gui_main)

using namespace std;

namespace wifi
{
void pp(wifi::packet p);
int debug_pp(wifi::packet p);
}

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
{
    size_t newLength = size*nmemb;
    size_t oldLength = s->size();
    try
    {
        s->resize(oldLength + newLength);
    }
    catch(std::bad_alloc &e)
    {
        //handle memory problem
        return 0;
    }

    std::copy((char*)contents,(char*)contents+newLength,s->begin()+oldLength);
    return size*nmemb;
}

// Notify if the new version is available
int version_check(char* version)
{
	std::string out_string;

	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, VURL);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); //only for https
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); //only for https
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out_string);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "IoTcube_Wireless_Fuzzer(G)");
	const CURLcode rc = curl_easy_perform(curl);
	
	int r_version_numeric = 0; // remote version
	int l_version_numeric = 0; // local version

	if (CURLE_OK == rc){
		if(out_string[1] == '.'){
			// convert string to 3 digit value
			r_version_numeric += (out_string[0] - '0') * 100;
			r_version_numeric += (out_string[2] - '0') * 10;
			r_version_numeric += (out_string[4] - '0');
			l_version_numeric += (VERSION[0] - '0') * 100;
			l_version_numeric += (VERSION[2] - '0') * 10;
			l_version_numeric += (VERSION[4] - '0');
			
			if(l_version_numeric >= r_version_numeric){
				printf("[+] You are using the latest version (Current version : Ver. %s)\n", VERSION);
			}
			else if(l_version_numeric < r_version_numeric){
				printf("[!] Ver. %s is available. (Your version : Ver. %s)\n", out_string.c_str(), VERSION);
				printf("[!] Please visit and downlaod the latest version in IoTcube. \n");
			}
			else{
				printf("[-] Version check error. (-1)\n");
			}
		}
		else{
			printf("[-] Version check error. (-2)\n");
		}
	}

	else if (CURLE_COULDNT_RESOLVE_HOST){
		printf("[-] Version check failed, but proceed anyway ... (Is network enabled?)\n");
	}
	else{
		std::cerr << "Error from cURL: " << curl_easy_strerror(rc) << std::endl;
		exit(EXIT_FAILURE);
	}

	curl_easy_cleanup(curl);

	return 0;
}

void sigint_handler(int signo){
	fprintf(fi, "\n\t]\n}");
	fclose(fi);
	signal(SIGINT, SIG_DFL);

	printf("[+] Fuzzing end\n");
	printf("-----------------------------------------------------------\n");
	printf("\n");

}

int is_root()
{
	return !getuid();
}

int set_mode(const char *ifname, const char* mode)
{
	FILE *fp;
	char *cmd;
	char buf[1024];
	
	if (!ifname)
		return -1;
	
	cmd = (char*)malloc(strlen(ifname) + strlen("iwconfig  mode managed 2>&1") + 1);
	
	sprintf(cmd, "ifconfig %s down 2>&1", ifname);
	if (!(fp = popen(cmd, "r")))
	{
		free(cmd);
		return -2;
	}
	
	if (fgets(buf, sizeof(buf), fp) != NULL)
	{
		printf("[-] set_managed(%s, %s) error: %s", ifname, mode, buf);
		while (fgets(buf, sizeof(buf), fp) != NULL)
			printf("[-]\t%s", buf);
		
		pclose(fp);
		free(cmd);
		return -3;
	}
	pclose(fp);
	
	sprintf(cmd, "iwconfig %s mode %s 2>&1", ifname, mode);
	if (!(fp = popen(cmd, "r")))
	{
		free(cmd);
		return -4;
	}
	
	if (fgets(buf, sizeof(buf), fp) != NULL)
	{
		printf("[-] set_managed(%s, %s) error: %s", ifname, mode, buf);
		while (fgets(buf, sizeof(buf), fp) != NULL)
			printf("[-]\t%s", buf);
		
		pclose(fp);
		free(cmd);
		return -5;
	}
	pclose(fp);
	
	sprintf(cmd, "ifconfig %s up 2>&1", ifname);
	if (!(fp = popen(cmd, "r")))
	{
		free(cmd);
		return -6;
	}
	
	if (fgets(buf, sizeof(buf), fp) != NULL)
	{
		printf("[-] set_managed(%s, %s) error: %s", ifname, mode, buf);
		while (fgets(buf, sizeof(buf), fp) != NULL)
			printf("[-]\t%s", buf);
		
		pclose(fp);
		free(cmd);
		return -7;
	}
	pclose(fp);
	free(cmd);
	
	// check whether the mode of interface has changed or not
	return 0;
}

int set_channel(const char *ifname, const int ch)
{
	FILE *fp;
	char *cmd;
	char buf[1024];
	
	if (!ifname)
		return -1;

	cmd = (char*)malloc(strlen(ifname) + strlen("ifconfig  down"));
	
	sprintf(cmd, "ifconfig %s down", ifname);
	if(!(fp = popen(cmd, "r")))
		return -2;

	if (fgets(buf, sizeof(buf), fp) != NULL)
	{
		printf("[-] ifconfig down (%s) error: %s", ifname, buf);
		pclose(fp);
		free(cmd);
		return -3;
	}
	pclose(fp);
	free(cmd);

	set_mode(ifname, "monitor");

	cmd = (char*)malloc(strlen(ifname) + strlen("ifconfig  up"));
	
	sprintf(cmd, "ifconfig %s up", ifname);
	if(!(fp = popen(cmd, "r")))
		return -2;

	if (fgets(buf, sizeof(buf), fp) != NULL)
	{
		printf("[-] ifconfig up (%s) error: %s", ifname, buf);
		pclose(fp);
		free(cmd);
		return -3;
	}
	pclose(fp);
	free(cmd);

	cmd = (char*)malloc(strlen(ifname) + strlen("iwconfig  channel 12 2>&1") + 1);
	
	sprintf(cmd, "iwconfig %s channel %d 2>&1", ifname, ch);
	if (!(fp = popen(cmd, "r")))
		return -2;
	
	if (fgets(buf, sizeof(buf), fp) != NULL)
	{
		printf("[-] set_channel(%s, %d) error: %s", ifname, ch, buf);
		while (fgets(buf, sizeof(buf), fp) != NULL)
		printf("[-]\t%s", buf);
		
		pclose(fp);
		free(cmd);
		return -3;
	}
	pclose(fp);
	free(cmd);
	
	// check whether the channel of interface has changed or not
	return 0;
}

void getTime(){
    struct timeval val;
    struct tm *ptm;

    gettimeofday(&val, NULL);
    ptm = localtime(&val.tv_sec);
  
    fprintf(fi, "\"%04d-%02d-%02d %02d:%02d:%02d.%06ld\""
            ,ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday
            ,ptm->tm_hour,ptm->tm_min,ptm->tm_sec
            ,val.tv_usec);
}

string get_logfile_name(){
	time_t t = time(0);
	struct tm * now = localtime(&t);
	char buf[sizeof "log_000000.wfl"];

	sprintf(buf, "log_%02d%02d%02d.wfl", now->tm_hour, now->tm_min, now->tm_sec);
	string str(buf);

	return str;
}


int AP_fuzz(char* ifname)
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

	vector<wifi::wifi_info> vec_wi;
	// vec_wi.push_back(tmp);
	
	wifi::wifi_scan(ifname, vec_wi);
	if (vec_wi.size() == 0) {
	fprintf(stderr, "[-] Cannot find any APs. Please try again.\n");
	printf("[ ] Evenif APs are nearby, it might happen several times... Take your time.\n");
	printf("[ ] (Hint) How about eject your dongle and re-insert?\n");
	return -1;
	}

	/* show result */
	printf("\n\tTarget AP List\n");
	printf("\t[No.]\t[channel]\t[address]\t\t[SSID]\n");
	for (int i = 0; i < vec_wi.size(); i++)
	printf("\t%02d\t%02d\t%s\t%s\n", i, vec_wi[i].channel, vec_wi[i].szaddr.c_str(), vec_wi[i].ESSID.c_str());
	printf("\tTotal: %ld\n\n", vec_wi.size());

	/* select AP */
	int ap_num = 0;
	/*printf("\nselect AP: ");
	while(!scanf("%d", &ap_num));
	while (ap_num < 0 || ap_num >= vec_wi.size())
	{
	printf("wrong AP number\n");
	printf("select AP: ");
	scanf("%d", &ap_num);
	}*/

	// Register signal (stop signal : Ctrl + C)
	signal(SIGINT, sigint_handler); 

	printf("\n[+] Wait for the Fuzzing to be started...\n");
	printf("[+] Please hit \"Ctrl + C\" to finish...\n");
	
	wifi::interface iface(ifname);
	wifi::sender_ap psta(iface, vec_wi[ap_num]);
	
	// Automatic channel setting
	set_channel(ifname, vec_wi[ap_num].channel);

	// dprint("[+] Changing interface mode to monitor mode...\n");
	if (set_mode(ifname, "monitor")){
		printf("[-] Error while changing to interface mode \"monitor\".");
		printf("[-] Please check your dongle.\n");
		return -1;
	}
	printf("[+] Interface's mode has changed to monitor mode\n");

	pkt_cnt = 0;
	//wifi::Fuzzer fuzz(psta);

	//If replay, just lookup the packets and send the replay packet
	if(is_replay == true){
		//fuzz.replay();
		return 0;
	}

	//Fuzzer fuzz(s); // sender inheritance
	int file_count = 0;

	const char* logfile_name = get_logfile_name().c_str();
	fi = fopen(logfile_name, "w");
	fprintf(fi,"{\n\t\"toolVer\" : \"%s\", \n\t\"interface\" : \"Wi-Fi\", \n\t\"waddr\" : \"%s\",", VERSION, vec_wi[ap_num].szaddr.c_str());
	//fprintf(fi,"{\n\t\"toolVer\" : \"%s\", \n\t\"interface\" : \"Wi-Fi\", \n\t\"waddr\" : \"%s\",", VERSION, tmp.szaddr.c_str());
	fprintf(fi, "\n\t\"starting_time\" : ");
	getTime();
	fprintf(fi, " ,\n\t\"protocol\" : \"802.11_AP\"");
	fprintf(fi, " ,\n\t\"packet\" : [\n");

	for (int cnt = 0; cnt < 2; cnt++)
	{
		try
		{
			//fuzz.fuzz();
		}
		catch (wifi::crash_found &e)
		{
			printf("\n");
			printf("[+] ******** Crash found (%s) ***********\n", e.what());
			printf("[+] state: %d, proto: %d, id: %d\n", e.s, e.p.proto, e.p.id);
			printf("[+] Crash packet\n");
			//printf("proto: %d\n", fuzz.s.proto);
			wifi::pp(e.p);
			
			char org_pkt[1024];
			int size;
			wifi::Database db;
			db.open();
			
			// db.get_original(e.p.proto, e.p.id, org_pkt, &size);
			// printf("[+] size: %d\n", size);
			// for (int i = 0; i < size; i++)
			// {
			// 	if (i % 16 == 0)
			// 		printf("[+] ");
			// 	printf("%02X ", (unsigned char)org_pkt[i]);
			// 	if (i % 16 == 15)
			// 		printf("\n");
			// }

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
	return 0;

}


//int main(int argc, char* argv[])
bool gui_main::OnInit()
{

	if (!is_root())
	{
		printf("[-] User mode must be root\n");
		return -1;
	}

	dprint("[+] DEBUG MODE ON\n");

	if(argc > 1 && strcmp(argv[1], "replay") == 0){
		is_replay = true;
		
	}
	else
		is_replay = false;


	version_check((char *) VERSION);
	wifi::Database db_for_update;
	db_for_update.update_db();
	//select_dongle();
	
	wxFrame* frame = new gui_frame("Main");
	//frame->Show(true);
	frame->ShowFullScreen(true);

	//AP_fuzz(waddr);
	//return 0;
	return true;
}
