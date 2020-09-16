#include "gui.h"

BEGIN_EVENT_TABLE(gui_frame, wxFrame)
EVT_BUTTON(ID_QUIT, gui_frame::OnQuit)
EVT_BUTTON(ID_SCAN, gui_frame::OnScan)
EVT_BUTTON(ID_UPLOAD, gui_frame::OnUpload)
END_EVENT_TABLE()


gui_frame::gui_frame(const wxString& title)
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


gui_frame::~gui_frame()
{
}


void gui_frame::OnQuit(wxCommandEvent& event)
{
	this->Close();
}

void gui_frame::OnScan(wxCommandEvent& event)
{
	wxFrame *scan_window = new gui_scan("Scan");
	//scan_window->Show(TRUE);
	scan_window->ShowFullScreen(true);
	this->Close();
}

void gui_frame::OnUpload(wxCommandEvent& event)
{
	popen("firefox https://iotcube.net/process/type/bf1","r");

	//run in fullscreen
	//popen("firefox https://iotcube.net/process/type/bf1 & xdotool search --sync --onlyvisible --class \"Firefox\" windowactivate key F11","r");

	//dialog
	/*wxFileDialog* OpenDialog = new wxFileDialog(this, _("Choose a file to open"), wxEmptyString, wxEmptyString,_("Result files (*.wfl)|*.wfl"),wxFD_OPEN, wxDefaultPosition);

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
	OpenDialog->Destroy();*/

}
