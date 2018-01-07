#include "myframe2.h"
#include "scan.h"

BEGIN_EVENT_TABLE(MyFrame2, wxFrame)
EVT_BUTTON(ID_QUIT, MyFrame2::OnQuit)
EVT_BUTTON(ID_SCAN, MyFrame2::OnScan)
EVT_BUTTON(ID_UPLOAD, MyFrame2::OnUpload)
END_EVENT_TABLE()


MyFrame2::MyFrame2(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title)
{
	wxPanel* panel = new wxPanel(this);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	wxButton* scan = new wxButton(panel, ID_SCAN, "&Scan");
	hbox->Add(scan, 1, wxEXPAND | wxALL, 10);
	wxButton* upload = new wxButton(panel, ID_UPLOAD, "&Upload");
	hbox->Add(upload, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 10);
	wxButton* quit = new wxButton(panel, ID_QUIT, "&Quit");
	hbox->Add(quit, 1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 10);

	panel->SetSizer(hbox);
    Centre();
}


MyFrame2::~MyFrame2()
{
}


void MyFrame2::OnQuit(wxCommandEvent& event)
{
	this->Close();
}

void MyFrame2::OnScan(wxCommandEvent& event)
{
	wxFrame *scan_window = new Scan("Scan");
	scan_window->Show(TRUE);
	this->Close();
}

void MyFrame2::OnUpload(wxCommandEvent& event)
{
	wxFileDialog* OpenDialog = new wxFileDialog(this, _("Choose a file to open"), wxEmptyString, wxEmptyString,_("Result files (*.wfl)|*.wfl"),wxFD_OPEN, wxDefaultPosition);

	// Creates a "open file" dialog with 4 file types
	if (OpenDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "Cancel"
	{
		CurrentDocPath = OpenDialog->GetPath();
		// Sets our current document to the file the user selected
		MainEditBox->LoadFile(CurrentDocPath); //Opens that file
		SetTitle(wxString("Edit - ") <<
			OpenDialog->GetFilename()); // Set the Title to reflect the file open
	}

	// Clean up after ourselves
	OpenDialog->Destroy();
}
