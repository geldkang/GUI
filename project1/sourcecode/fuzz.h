#pragma once
#include <wx/wx.h>

class Fuzz : public wxFrame
{
	enum
	{
		ID_QUIT = 1,
	};

public:
	Fuzz(const wxString& title);
	~Fuzz();

private:
	void OnQuit(wxCommandEvent& event);
	DECLARE_EVENT_TABLE();
};
