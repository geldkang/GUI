#include "scan.h"

BEGIN_EVENT_TABLE(Scan, wxFrame)
EVT_BUTTON(ID_QUIT, Scan::OnQuit)
END_EVENT_TABLE()

Scan::Scan(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title)
{
	wxPanel* panel = new wxPanel(this);
	wxButton* btn = new wxButton(panel, ID_QUIT, "&Quit", wxPoint(290, 160));
}

Scan::~Scan()
{
}

void Scan::OnQuit(wxCommandEvent& event)
{
	this->Close();
}