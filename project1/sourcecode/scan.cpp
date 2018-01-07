#include "scan.h"
#include "fuzz.h"

BEGIN_EVENT_TABLE(Scan, wxFrame)
EVT_BUTTON(ID_FUZZ, Scan::OnFuzz)
EVT_BUTTON(ID_QUIT, Scan::OnQuit)
END_EVENT_TABLE()

Scan::Scan(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title)
{
	wxPanel* panel = new wxPanel(this, -1);

    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    wxTextCtrl *dev_list = new wxTextCtrl(panel, wxID_ANY, wxT(""),wxPoint(-1, -1), wxSize(-1, -1), wxTE_MULTILINE);
    hbox->Add(dev_list, 1, wxEXPAND | wxALL, 10);

    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxButton* fuzz = new wxButton(panel, ID_FUZZ, "&Fuzz");
	vbox->Add(fuzz, 1, wxEXPAND | wxALL, 10);
	wxButton* quit = new wxButton(panel, ID_QUIT, "&Quit");
	vbox->Add(quit, 1, wxEXPAND | wxALL, 10);

	hbox->Add(vbox, 1, wxEXPAND, 10);

	panel->SetSizer(hbox);
    Centre();
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
	this->Close();
}
