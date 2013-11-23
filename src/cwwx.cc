#include <wx/wx.h>
#include <wx/icon.h>
#include "config.h"

extern "C" {
#include "cwstudio.h"
}

class CWSound
{
private:

	cw_sample   asound, csound;
	cw_param    param;
	char		*cwtext = NULL;
	int			changed;
	
public:

	CWSound() 
	{
		cw_initparam(&param);
		cw_initsample(&asound, NULL);
		cw_initsample(&csound, &asound);
		changed = 1;
	}

	~CWSound()
	{
		cw_freesample(&asound);
		cw_freesample(&csound);
		cw_free(cwtext);
	}

	void SetText(const char* text)
	{
		if (cwtext) cw_free(cwtext);
		cwtext = cw_encode(text);
	}
	
	void Update()
	{
		cw_signals(&asound, param, cwtext);
		cw_convert(&asound, &csound, 16);
		changed = 0;
	}
	
	void WaveOut(const char* filename)
	{
		cw_wavout(filename, &csound);
	}
	
	void Play()
	{
		cwstudio_play(&csound);
	}
	
	void Pause()
	{
		cwstudio_pause();
	}
	void Stop()
	{
		cwstudio_stop();
	}

};





class CWStudio: public wxApp
{
	virtual bool OnInit();
};


class CWWindow: public wxFrame
{
public:

	CWWindow(const wxString& title, 
	const wxPoint& pos, const wxSize& size);

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void Generate(wxCommandEvent& event);
	
	DECLARE_EVENT_TABLE()
};




enum
{
	ID_Quit = 1,
	ID_About,
	ID_Play,
};

BEGIN_EVENT_TABLE(CWWindow, wxFrame)
EVT_MENU(ID_Quit, CWWindow::OnQuit)
EVT_MENU(ID_About, CWWindow::OnAbout)
EVT_BUTTON(ID_Play, CWWindow::Generate)
END_EVENT_TABLE()

IMPLEMENT_APP(CWStudio)

bool CWStudio::OnInit()
{
	
	CWWindow *frame = new CWWindow( wxString(wxT("CWStudio")), wxPoint(50,50), wxSize(450,340) );
	
	
		
		
	frame->Show(TRUE);
	SetTopWindow(frame);
	return TRUE;
} 

CWWindow::CWWindow(const wxString& title, const wxPoint& pos, const wxSize& size) 
: wxFrame((wxFrame *)NULL, -1, title, pos, size)
{
	wxMenu *menuFile = new wxMenu;
	menuFile->Append( ID_About, wxT("&About...") );
	menuFile->AppendSeparator();
	menuFile->Append( ID_Quit, wxT("E&xit") );
	
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append( menuFile, wxT("&File") );

	wxPanel *panel = new wxPanel(this, wxID_ANY);


	wxButton *button  = new wxButton(panel,ID_Play,wxT("My wxButton"),wxPoint(10,10),wxSize(100,20),0);
	wxTextCtrl *textcontrol = new wxTextCtrl(panel,wxID_ANY,wxT(""),wxPoint(10,40),wxSize(200,200),wxTE_MULTILINE);
	
	SetMenuBar( menuBar );
	//SetIcon(wxIcon(1));

	CreateStatusBar();
	SetStatusText( wxT("Welcome to wxWindows!") );
	CWSound *sound = new CWSound();
	
}


void CWWindow::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(TRUE);
}

void CWWindow::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	
	wxMessageBox(wxT("CWStudio"),
	wxT("CWStudio"), wxOK | wxICON_INFORMATION, this);
	
}

void CWWindow::Generate(wxCommandEvent& WXUNUSED(event))
{

	
	//sound->SetText("VVV = CQ DE SP8QED");
	//sound->Update();
	//sound->Play();
	//delete sound;

	
}

