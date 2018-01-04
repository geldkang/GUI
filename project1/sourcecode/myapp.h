#pragma once
#include <wx/wx.h>
class MyApp : public wxApp
{
public:
	MyApp();
	~MyApp();
	virtual bool OnInit();
};
DECLARE_APP(MyApp);
