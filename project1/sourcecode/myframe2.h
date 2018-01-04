#pragma once
#include <wx/wx.h>

class MyFrame2 : public wxFrame
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
	MyFrame2(const wxString& title);
	~MyFrame2();

private:
	void OnQuit(wxCommandEvent& event);
	void OnScan(wxCommandEvent& event);
	void OnUpload(wxCommandEvent& event);
	DECLARE_EVENT_TABLE();
};