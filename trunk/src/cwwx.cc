#include <wx/wx.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include "config.h"
#include <string>

extern "C" {
#include "cwstudio.h"
}

using namespace std;

class CWSound
{
private:

	cw_sample   asound, csound;
	cw_param    param;
	string		cwtext;
	int			changed;
	
public:

	CWSound() 
	{
		cw_initparam(&param);
		cw_initsample(&asound, NULL);
		cw_initsample(&csound, &asound);
		//cwtext = new string;
		changed = 1;
	}

	~CWSound()
	{
		cw_freesample(&asound);
		cw_freesample(&csound);
		//delete cwtext; //cw_free(cwtext);
	}

	void SetText(const char* text)
	{
		char *tmp;
		//if (cwtext != NULL) cw_free(cwtext);
		tmp = cw_encode(text);
		cwtext = string(tmp);
		cw_free(tmp);
	}
	
	void Update()
	{
		cw_signals(&asound, param, cwtext.c_str());
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
	void Play(wxCommandEvent& event);
	void Stop(wxCommandEvent& event);
	void Pause(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()

private:
	CWSound *sound;
	wxTextCtrl *textctrl;
};

enum
{
	ID_Quit = 1,
	ID_About,
	ID_Play,
	ID_Pause,
	ID_Stop,
	ID_Regenerate,
	ID_Update,
	
};

BEGIN_EVENT_TABLE(CWWindow, wxFrame)
EVT_MENU(ID_Quit, CWWindow::OnQuit)
EVT_MENU(ID_About, CWWindow::OnAbout)
EVT_BUTTON(ID_Play, CWWindow::Play)
EVT_BUTTON(ID_Stop, CWWindow::Stop)
EVT_BUTTON(ID_Pause, CWWindow::Pause)
END_EVENT_TABLE()

IMPLEMENT_APP(CWStudio)

bool CWStudio::OnInit()
{
	
	CWWindow *frame = new CWWindow( wxString(wxT("CWStudio")), wxPoint(50,50), wxSize(900,650) );
	
	
	
	
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
	
	wxBoxSizer *mainsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *buttonsizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxTextCtrl *textctrl = new wxTextCtrl(this,wxID_ANY,wxT("VVV = CQ DE CWSTUDIO"),wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE);
	
	mainsizer->Add(buttonsizer, 0, 0, 0);
	mainsizer->Add( new wxSlider(this,wxID_ANY,60,40,300),0,wxEXPAND,0);
	mainsizer->Add(textctrl, 1, wxEXPAND, 0);
	
	wxButton *playbutton = new wxButton(this, ID_Play, "Play");
	wxButton *stopbutton = new wxButton(this, ID_Stop, "Stop");
	wxButton *pausebutton = new wxButton(this, ID_Pause, "Pause");
	
	buttonsizer->Add(playbutton,1,wxEXPAND,0);
	buttonsizer->Add(stopbutton,1,wxEXPAND,0);
	buttonsizer->Add(pausebutton,1,wxEXPAND,0);
	//buttonsizer->SetSizeHints(this);
	
	SetSizer(mainsizer);
	SetMenuBar(menuBar);
	//SetIcon(wxIcon(1));
	CreateStatusBar();
	SetStatusText( wxT("Ready.") );
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

void CWWindow::Play(wxCommandEvent& WXUNUSED(event))
{
	SetStatusText( wxT("Setting text...") );
	sound->SetText("AAA");
	SetStatusText( wxT("Updating...") );
	sound->Update();
	SetStatusText( wxT("Playing...") );
	sound->Play();
}

void CWWindow::Stop(wxCommandEvent& WXUNUSED(event))
{
	SetStatusText( wxT("Stopped.") );
	sound->Stop();
}

void CWWindow::Pause(wxCommandEvent& WXUNUSED(event))
{
	sound->Pause();
}
