#pragma once
#include <wx/wx.h>

class Scan : public wxFrame
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
	Scan(const wxString& title);
	~Scan();

private:
	void OnQuit(wxCommandEvent& event);
	void OnFuzz(wxCommandEvent& event);
	DECLARE_EVENT_TABLE();
};
