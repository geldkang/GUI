#include "myapp.h"
#include "myframe2.h"
#include "scan.h"

IMPLEMENT_APP(MyApp);

MyApp::MyApp()
{
}

MyApp::~MyApp()
{
}

bool MyApp::OnInit()
{
	wxFrame* frame = new MyFrame2("Main");
	frame->Show(true);

	return true;    // false ´Â Á¾·á.
}
