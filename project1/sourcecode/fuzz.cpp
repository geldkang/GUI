#include "fuzz.h"

BEGIN_EVENT_TABLE(Fuzz, wxFrame)
EVT_BUTTON(ID_QUIT, Fuzz::OnQuit)
END_EVENT_TABLE()

Fuzz::Fuzz(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title)
{
	wxPanel* panel = new wxPanel(this);

	wxButton* btn = new wxButton(panel, ID_QUIT, "&Stop", wxPoint(290, 50), wxSize(100, 100));

	wxStaticText* text = new wxStaticText(this, -1, _T("Hello World!"));
}

Fuzz::~Fuzz()
{
}

void Fuzz::OnQuit(wxCommandEvent& event)
{
	this->Close();
}
