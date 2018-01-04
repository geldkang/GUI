#pragma once
#include <wx/wx.h>

class Scan : public wxFrame
{
	enum
	{
		ID_QUIT = 1,
	};

public:
	Scan(const wxString& title);
	~Scan();

private:
	void OnQuit(wxCommandEvent& event);
	DECLARE_EVENT_TABLE();
};