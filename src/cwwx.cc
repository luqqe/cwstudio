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

static char			*text = NULL, *morsetext = NULL;
static cw_sample	asound, csound;
static cw_param		param;
static int			mode = 0, wordset = 100, chars;
static unsigned int bits = 16;
static unsigned int samplerate = 44100;
static int			playmode = CWSTOPPED;


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
	void Tempo(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()

	wxTextCtrl *textctrl;
	wxSlider *slidertempo;

private:
};

enum
{
	ID_Quit = 1,
	ID_About,
	ID_Play,
	ID_Pause,
	ID_Stop,
	ID_Generate,
	ID_Update,
	ID_Tempo,
	
};

BEGIN_EVENT_TABLE(CWWindow, wxFrame)
EVT_MENU(ID_Quit, CWWindow::OnQuit)
EVT_MENU(ID_About, CWWindow::OnAbout)
EVT_BUTTON(ID_Play, CWWindow::Play)
EVT_BUTTON(ID_Stop, CWWindow::Stop)
EVT_BUTTON(ID_Pause, CWWindow::Pause)
EVT_BUTTON(ID_Generate, CWWindow::Generate)
EVT_SLIDER(ID_Tempo, CWWindow::Tempo)
END_EVENT_TABLE()

IMPLEMENT_APP(CWStudio)

bool CWStudio::OnInit()
{
	
	CWWindow *frame = new CWWindow( wxString(wxT("CWStudio")), wxPoint(50,50), wxSize(900,650) );
	frame->Show(TRUE);
	SetTopWindow(frame);
	cw_initparam(&param);
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
	
	textctrl = new wxTextCtrl(this,wxID_ANY,wxT("VVV = CQ DE CWSTUDIO"),wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE);
	
	slidertempo = new wxSlider(this,ID_Tempo,60,40,300);
	
	mainsizer->Add(buttonsizer, 0, 0, 0);
	mainsizer->Add(slidertempo,0,wxEXPAND,0);
	mainsizer->Add(textctrl, 1, wxEXPAND, 0);
	
	wxButton *playbutton = new wxButton(this, ID_Play, "Play");
	wxButton *stopbutton = new wxButton(this, ID_Stop, "Stop");
	wxButton *pausebutton = new wxButton(this, ID_Pause, "Pause");
	wxButton *generatebutton = new wxButton(this, ID_Generate, "Generate");
	
	buttonsizer->Add(playbutton,1,wxEXPAND,0);
	buttonsizer->Add(stopbutton,1,wxEXPAND,0);
	buttonsizer->Add(pausebutton,1,wxEXPAND,0);
	buttonsizer->Add(generatebutton,1,wxEXPAND,0);
	//buttonsizer->SetSizeHints(this);
	
	SetSizer(mainsizer);
	SetMenuBar(menuBar);
	//SetIcon(wxIcon(1));
	CreateStatusBar();
	SetStatusText( wxT("Ready.") );
	//CWSound *sound = new CWSound();
	
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
	char *t;
	t = cw_rand_groups(25, 5, "abstgjn", 0);
	textctrl->SetValue(t);
	cw_free(t);
	
 
	
}

void CWWindow::Play(wxCommandEvent& WXUNUSED(event))
{
	if(playmode == CWSTOPPED) {
					SetStatusText( wxT("Playing.") );
					cw_free(text);
					cw_freesample(&asound);
					cw_freesample(&csound);
					cw_initsample(&asound, NULL);
					asound.samplerate = samplerate;
					cw_initsample(&csound, &asound);
					
					std::cout << textctrl->GetValue();
					
					text = (char*)malloc((textctrl->GetValue()).Len()*sizeof(char)+1);
					strcpy(text, (const char*)(textctrl->GetValue()).mb_str(wxConvUTF8));
										
					morsetext = cw_encode(text);
					
					cw_signals(&asound, param, morsetext);
					cw_convert(&asound, &csound, bits);
					playmode = cwstudio_play(&csound);
				}
				
}

void CWWindow::Stop(wxCommandEvent& WXUNUSED(event))
{
	SetStatusText( wxT("Stopped.") );
	playmode = cwstudio_stop();

	
	//sound->Stop();
}

void CWWindow::Pause(wxCommandEvent& WXUNUSED(event))
{

				playmode = cwstudio_pause();
				if(playmode == CWPLAYING)
					SetStatusText( wxT("Resumed.") );
				else if(playmode == CWPAUSED)
					SetStatusText( wxT("Paused.") );
				else if(playmode == CWSTOPPED)
					SetStatusText( wxT("Stopped.") );
}

void CWWindow::Tempo(wxCommandEvent& WXUNUSED(event))
{
	param.tempo = slidertempo->GetValue();

}