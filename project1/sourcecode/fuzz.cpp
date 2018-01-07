#include "fuzz.h"

BEGIN_EVENT_TABLE(Fuzz, wxFrame)
EVT_BUTTON(ID_QUIT, Fuzz::OnQuit)
END_EVENT_TABLE()

Fuzz::Fuzz(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title)
{
	wxPanel *panel = new wxPanel(this, -1);

    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *hbox1 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *status =  new wxStaticText(panel, wxID_ANY,wxT("Status"));
    hbox1->Add(status, 0, wxRIGHT, 10);
    wxTextCtrl *statusbar = new wxTextCtrl(panel, wxID_ANY);
    hbox1->Add(statusbar, 1);
    vbox->Add(hbox1, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);

    wxBoxSizer *hbox2 = new wxBoxSizer(wxHORIZONTAL);
    wxTextCtrl *crashpacket = new wxTextCtrl(panel, wxID_ANY, wxT(""), wxPoint(-1, -1), wxSize(-1, -1), wxTE_MULTILINE);
    hbox2->Add(crashpacket, 1, wxEXPAND | wxALL, 10);

    wxBoxSizer *vbox2 = new wxBoxSizer(wxVERTICAL);
    wxTextCtrl *packetnumber = new wxTextCtrl(panel, wxID_ANY, wxT(""), wxPoint(-1, -1), wxSize(-1, -1), wxTE_MULTILINE);
    vbox2->Add(packetnumber, 1, wxEXPAND | wxALL, 10);
    wxButton* stop = new wxButton(panel, ID_QUIT, "&Stop");
    vbox2->Add(stop, 1, wxEXPAND | wxALL, 10);

    hbox2->Add(vbox2, 1, wxEXPAND, 10);

    vbox->Add(hbox2, 1, wxALL | wxEXPAND, 10);

    panel->SetSizer(vbox);

    Centre();

}

Fuzz::~Fuzz()
{
}

void Fuzz::OnQuit(wxCommandEvent& event)
{
	this->Close();
}
