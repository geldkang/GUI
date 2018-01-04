#include "scan.h"
#include "fuzz.h"

BEGIN_EVENT_TABLE(Scan, wxFrame)
EVT_BUTTON(ID_FUZZ, Scan::OnFuzz)
EVT_BUTTON(ID_QUIT, Scan::OnQuit)
END_EVENT_TABLE()

Scan::Scan(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title)
{
	wxPanel* panel = new wxPanel(this);
	wxButton* btn1 = new wxButton(panel, ID_FUZZ, "&Fuzz", wxPoint(290, 40), wxSize(100, 100));
	wxButton* btn = new wxButton(panel, ID_QUIT, "&Quit", wxPoint(290, 160));
}

Scan::~Scan()
{
}

void Scan::OnQuit(wxCommandEvent& event)
{
	this->Close();
}

void Scan::OnFuzz(wxCommandEvent& event)
{
	wxFrame *fuzz_window = new Fuzz("Fuzz");
	fuzz_window->Show(TRUE);
}
