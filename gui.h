#pragma once
#include <wx/wx.h>
#include <wx/thread.h>

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
#include "Database.h"
#include "dprint.h"
#include "exceptions.h"

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s);
int version_check(char* version);
int select_dongle();
void sigint_handler(int signo);
int is_root();
int set_mode(const char *ifname, const char *mode);
int set_channel(const char *ifname, const int ch);
void getTime();
string get_logfile_name();
int STA_fuzz(char* ifname);
int AP_fuzz(char* ifname);

class gui_main: public wxApp
{
protected:

    // from wxApp
    	virtual bool OnInit();
};

class gui_frame : public wxFrame
{
	enum
	{
		ID_QUIT = 1,
	};	
	enum
	{
		ID_SCAN = 2,
	};
	enum
	{
		ID_UPLOAD = 3,
	};

public:
	wxString CurrentDocPath;
	wxTextCtrl *MainEditBox;
	gui_frame(const wxString& title);
	~gui_frame();

private:
	void OnQuit(wxCommandEvent& event);
	void OnScan(wxCommandEvent& event);
	void OnUpload(wxCommandEvent& event);
	DECLARE_EVENT_TABLE();
};

class gui_scan : public wxFrame
{
	enum
	{
		ID_QUIT = 1,
	};
	enum
	{
		ID_FUZZ = 2,
	};

public:
	wxListBox* dev_list;
	gui_scan(const wxString& title);
	~gui_scan();
private:
	void OnQuit(wxCommandEvent& event);
	void OnFuzz(wxCommandEvent& event);
	DECLARE_EVENT_TABLE();
};

class gui_fuzz : public wxFrame, public wxThreadHelper
{
	enum
	{
		ID_QUIT = 1,
	};

public:
	char buf[100];
	int packet_count;
	int crash_count;
	int state_curr;
	wxStaticText* statusbar;
	wxListBox* crashpacket;
	wxStaticText* packetnumber;
	char* _ifname;
	vector<wifi::wifi_info> _vec_wi;
	int _AP;
	gui_fuzz(const wxString& title, char* waddr, vector<wifi::wifi_info> vec_wi, int AP);
	~gui_fuzz();
	void OnThreadUpdate(wxThreadEvent& evt);

private:
	void OnQuit(wxCommandEvent& event);
	DECLARE_EVENT_TABLE();
protected:
	virtual wxThread::ExitCode Entry();
};
