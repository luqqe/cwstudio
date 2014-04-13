/*$T indentinput.cpp GC 1.140 04/08/14 19:27:37 */
/*$I0

    This file is part of CWStudio.

    Copyright 2008-2014 Lukasz Komsta, SP8QED

    CWStudio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CWStudio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CWStudio. If not, see <http://www.gnu.org/licenses/>.

 */

#include <wx/wx.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include "config.h"
#include <string>


#ifdef HAVE_LIBWINMM
#define SOUND_INTERFACE "/waveout"
#elif defined HAVE_PULSEAUDIO
#define SOUND_INTERFACE "/pulseaudio"
#elif defined HAVE_OSS
#define SOUND_INTERFACE "/oss"
#else
#define SOUND_INTERFACE ""
#endif



extern "C"
{
#include "cwstudio.h"
}
using namespace		std;

static char			*text = NULL, *morsetext = NULL;
static cw_sample	asound, csound;
static cw_param		param;
static int			mode = 0, wordset = 100, chars;
static unsigned int bits = 16;
static unsigned int samplerate = 44100;
static int			playmode = CWSTOPPED;

class CWStudio :
	public wxApp
{
	virtual bool	OnInit();
};

class CWWindow :
	public wxFrame
{
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:
	CWWindow(const wxString &title, const wxPoint &pos, const wxSize &size);

	void	OnQuit(wxCloseEvent &event);
	void	OnAbout(wxCommandEvent &event);
	void	GenerateGroups(wxCommandEvent &event);
	void GenerateCalls(wxCommandEvent & event);
	void GenerateWords(wxCommandEvent & event);
	void Play(wxCommandEvent & event);
	void	Stop(wxCommandEvent &event);
	void	Pause(wxCommandEvent &event);
	void	Update(wxSpinEvent &event);
	void	WAV(wxCommandEvent &event);
	DECLARE_EVENT_TABLE()
	wxTextCtrl		*textctrl;
	wxSlider		*sliderwords;

	wxSpinCtrl		*spins[30];
	wxStaticText	*txts[30];
	wxBoxSizer		*spinsizers[10];
	wxComboBox * boxcharset;

/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
private:
};

enum { ID_Quit = 1, ID_About, ID_Play, ID_Pause, ID_Stop, ID_Groups, ID_Words, ID_Calls, ID_Update, ID_WAV };

BEGIN_EVENT_TABLE(CWWindow, wxFrame)
EVT_CLOSE(CWWindow::OnQuit)
EVT_BUTTON(ID_Play, CWWindow::Play)
EVT_BUTTON(ID_Stop, CWWindow::Stop)
EVT_BUTTON(ID_WAV, CWWindow::WAV)
EVT_BUTTON(ID_Pause, CWWindow::Pause)
EVT_BUTTON(ID_Groups, CWWindow::GenerateGroups)
EVT_BUTTON(ID_Calls, CWWindow::GenerateCalls)
EVT_BUTTON(ID_Words, CWWindow::GenerateWords)
EVT_SPINCTRL(ID_Update, CWWindow::Update)
END_EVENT_TABLE()
IMPLEMENT_APP(CWStudio)
/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool CWStudio::OnInit()
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	CWWindow	*frame = new CWWindow(wxString(wxT("CWStudio")), wxPoint(50, 50), wxSize(1000, 650));
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	frame->Show(TRUE);
	SetTopWindow(frame);
	cw_initparam(&param);
	return TRUE;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
CWWindow::CWWindow(const wxString &title, const wxPoint &pos, const wxSize &size) :
	wxFrame((wxFrame *) NULL, -1, title, pos, size)
{
	FILE * f;
	char filename[255];
	char charset[256];
	wxString captions[30];

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	const int	defs[26] =
	{
		100,
		1,
		0,
		300,
		0,
		0,
		600,
		20,
		0,
		2400,
		0,
		300,
		100,
		0,
		0,
		0,
		0,
		3,
		100,
		0,
		0,
		80,
		100,
		100,
		0
	};
	const int	mins[26] = { 0, 0, 0, 100, 0, 0, 50, 1, 0, 300, 0, 50, 0, 0, 0, 0, -50, 1, 20, -4000, 0, 5, 0, 0, 0 };
	const int	maxs[26] =
	{
		100,
		100,
		100,
		1000,
		100,
		100,
		4000,
		100,
		100,
		10000,
		100,
		1000,
		100,
		10,
		100,
		32767,
		50,
		5,
		300,
		4000,
		10000,
		500,
		10000,
		1000,
		100
	};
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	captions[0] = wxT(" AGC");
	captions[1] = wxT(" Click");
	captions[2] = wxT(" Cspaces");
	captions[3] = wxT(" Dashlen");
	captions[4] = wxT(" Detune");
	captions[5] = wxT(" Even");
	captions[6] = wxT(" Freq");
	captions[7] = wxT(" Number");
	captions[8] = wxT(" Hand");
	captions[9] = wxT(" Highcut");
	captions[10] = wxT(" Hum");
	captions[11] = wxT(" Lowcut");
	captions[12] = wxT(" Noise");
	captions[13] = wxT(" Odd");
	captions[14] = wxT(" QSB");
	captions[15] = wxT(" Seed");
	captions[16] = wxT(" Shape");
	captions[17] = wxT(" Signals");
	captions[18] = wxT(" Spacelen");
	captions[19] = wxT(" Sweep");
	captions[20] = wxT(" Sweepness");
	captions[21] = wxT(" Tempo");
	captions[22] = wxT(" Window");
	captions[23] = wxT(" Wordset");
	captions[24] = wxT(" Wspaces");
	wxString charsets[21];
	charsets[0] = wxT("abstgjnokqfmzixdrhewlypvcu8219376450?!/=");
	charsets[1] = wxT("abstgjnokqfmzixdrhewlypvcu8219376450?!");
	charsets[2] = wxT("abstgjnokqfmzixdrhewlypvcu8219376450");
	charsets[3] = wxT("abstgjnokqfmzixdrhewlypvcu82193764");
	charsets[4] = wxT("abstgjnokqfmzixdrhewlypvcu821937");
	charsets[5] = wxT("abstgjnokqfmzixdrhewlypvcu8219");
	charsets[6] = wxT("abstgjnokqfmzixdrhewlypvcu82");
	charsets[7] = wxT("abstgjnokqfmzixdrhewlypvcu");
	charsets[8] = wxT("abstgjnokqfmzixdrhewlypvc");
	charsets[9] = wxT("abstgjnokqfmzixdrhewlyp");
	charsets[10] = wxT("abstgjnokqfmzixdrhewl");
	charsets[11] = wxT("abstgjnokqfmzixdrhe");
	charsets[12] = wxT("abstgjnokqfmzixdr");
	charsets[13] = wxT("abstgjnokqfmzix");
	charsets[14] = wxT("abstgjnokqfmz");
	charsets[15] = wxT("abstgjnokqf");
	charsets[16] = wxT("abstgjnok");
	charsets[17] = wxT("abstgjn");
	charsets[18] = wxT("abstg");
	charsets[19] = wxT("abst");
	charsets[20] = wxT("abs");
	
#if defined(__WXMSW__)
	SetIcon(wxICON(id));
#endif
	wxPanel * panel = new wxPanel(this, wxID_ANY);
	

	/*
	 * wxMenu *menuFile = new wxMenu;
	 * *menuFile->Append( ID_About, wxT("&About...") );
	 * *menuFile->AppendSeparator();
	 * *menuFile->Append( ID_Quit, wxT("E&xit") );
	 * ;
	 * wxMenuBar *menuBar = new wxMenuBar;
	 * *menuBar->Append( menuFile, wxT("&File") );
	 */
	wxBoxSizer * mainsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer	*buttonsizer = new wxBoxSizer(wxHORIZONTAL);

	for(int i = 0; i < 5; i++) spinsizers[i] = new wxBoxSizer(wxHORIZONTAL);
	textctrl = new wxTextCtrl(panel, wxID_ANY, wxT("VVV ="), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	textctrl->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("Courier New")));
	

	mainsizer->Add(buttonsizer, 0, wxEXPAND, 0);
	for(int i = 0; i < 5; i++) mainsizer->Add(spinsizers[i], 0, 0, 0);

	mainsizer->Add(textctrl, 1, wxEXPAND, 0);
	panel->SetSizer(mainsizer);

	wxButton * playbutton = new wxButton(panel, ID_Play, wxT("Play"));
	wxButton	*stopbutton = new wxButton(panel, ID_Stop, wxT("Stop"));
	wxButton	*pausebutton = new wxButton(panel, ID_Pause, wxT("Pause"));
	wxButton	*wavbutton = new wxButton(panel, ID_WAV, wxT("Save WAV"));
	wxButton	*groupsbutton = new wxButton(panel, ID_Groups, wxT("Groups"));
	wxButton *callsbutton = new wxButton(panel, ID_Calls, wxT("Calls"));
	wxButton *wordsbutton = new wxButton(panel, ID_Words, wxT("Words"));
	
	for(int j = 0; j < 5; j++)
	{
		for(int i = 0; i < 5; i++)
		{
			spins[5 * j + i] = new wxSpinCtrl
				(
					panel,
					ID_Update,
					wxEmptyString,
					wxDefaultPosition,
					wxDefaultSize,
					wxSP_ARROW_KEYS,
					mins[5 * j + i],
					maxs[5 * j + i],
					defs[5 * j + i],
					wxT("wxSpinCtrl")
				);
			//spins[5 * j + i]->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &CWWindow::Update, this);
			txts[5 * j + i] = new wxStaticText
				(
					panel,
					wxID_ANY,
					captions[5 * j + i],
					wxDefaultPosition,
					wxDefaultSize,
					0,
					wxT("staticText")
				);
			spinsizers[j]->Add(txts[5 * j + i], 1, wxEXPAND, 0);
			spinsizers[j]->Add(spins[5 * j + i], 1, wxEXPAND, 0);
		}
	}

	boxcharset = new wxComboBox
		(
			panel,
			wxID_ANY,
			wxT("abstgjnokqfmzixdrhewlypvcu8219376450?!/="),
			wxDefaultPosition,
			wxDefaultSize,
			20,
			charsets,
			0,
			wxDefaultValidator,
			wxT("x")
		);
		
	buttonsizer->Add(playbutton, 1, wxEXPAND, 0);
	buttonsizer->Add(stopbutton, 1, wxEXPAND, 0);
	buttonsizer->Add(pausebutton, 1, wxEXPAND, 0);
	buttonsizer->Add(wavbutton, 1, wxEXPAND, 0);
	buttonsizer->Add(groupsbutton, 1, wxEXPAND, 0);
	buttonsizer->Add(callsbutton, 1, wxEXPAND, 0);
	buttonsizer->Add(wordsbutton, 1, wxEXPAND, 0);
	buttonsizer->Add(boxcharset, 1, wxEXPAND, 0);
	

	/*
	 * SetMenuBar(menuBar);
	 */
	CreateStatusBar();
	SetStatusText(wxT("(C) 2008-2014 Lukasz Komsta SP8QED. http://cwstudio.sf.net/"));
	SetTitle(wxString(wxT("CWStudio ")) + wxString(wxT(VERSION)) + wxString(wxT(" (")) + wxString(wxT(CANONICAL_HOST)) + wxString(wxT(SOUND_INTERFACE)) + wxString(wxT(")")));
	param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
	spins[15]->SetValue(param.seed);
	Centre();
	sprintf(filename, "wx-%s.cfg", CANONICAL_HOST);
	if((f = fopen(filename, "r")) != NULL)
	{
		fread(&mode, sizeof(int), 1, f);
		fread(&wordset, sizeof(int), 1, f);
		fread(&chars, sizeof(int), 1, f);
		fread(&bits, sizeof(int), 1, f);
		fread(&samplerate, sizeof(int), 1, f);
		fread(&param, sizeof(cw_param), 1, f);
		fgets(charset, 256, f);
		boxcharset->SetValue(wxString::FromUTF8(charset));
		fclose(f);
		spins[0]->SetValue(param.agc);
		spins[1]->SetValue(param.click);
		spins[2]->SetValue(param.cspaces);
		spins[3]->SetValue(param.dashlen);
		spins[4]->SetValue(param.detune);
		spins[5]->SetValue(param.even);
		spins[6]->SetValue(param.freq);
		spins[7]->SetValue(param.number);
		spins[8]->SetValue(param.hand);
		spins[9]->SetValue(param.highcut);
		spins[10]->SetValue(param.hum);
		spins[11]->SetValue(param.lowcut);
		spins[12]->SetValue(param.noise);
		spins[13]->SetValue(param.odd);
		spins[14]->SetValue(param.qsb);
		spins[15]->SetValue(param.seed);
		spins[16]->SetValue(param.shape);
		spins[17]->SetValue(param.signals);
		spins[18]->SetValue(param.spacelen);
		spins[19]->SetValue(param.sweep);
		spins[20]->SetValue(param.sweepness);
		spins[21]->SetValue(param.tempo);
		spins[22]->SetValue(param.window);
		spins[23]->SetValue(wordset);
		spins[24]->SetValue(param.wspaces);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CWWindow::OnQuit(wxCloseEvent &event)
{
	/*~~~~~~~*/
	FILE	*f;
	/*~~~~~~~*/

	char filename[255];
	sprintf(filename, "wx-%s.cfg", CANONICAL_HOST);
	if((f = fopen(filename, "w")) != NULL)
	{
		fwrite(&mode, sizeof(int), 1, f);
		fwrite(&wordset, sizeof(int), 1, f);
		fwrite(&chars, sizeof(int), 1, f);
		fwrite(&bits, sizeof(int), 1, f);
		fwrite(&samplerate, sizeof(int), 1, f);
		fwrite(&param, sizeof(cw_param), 1, f);
		fprintf(f, "%s", (const char *) boxcharset->GetValue().mb_str(wxConvUTF8));
		fclose(f);
		
	}

	Destroy();
	

	/*
	 * Close(TRUE);
	 */
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CWWindow::OnAbout(wxCommandEvent &WXUNUSED(event))
{
	wxMessageBox(wxT("CWStudio"), wxT("CWStudio"), wxOK | wxICON_INFORMATION, this);
}

void CWWindow:: GenerateGroups(wxCommandEvent &WXUNUSED(event))
{
	char *t;
	t = cw_rand_groups(param.number, param.shape, (const char *) boxcharset->GetValue().mb_str(wxConvUTF8), param.seed);
	textctrl->SetValue(wxString::FromAscii(t));
	cw_free(t);
	param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
	spins[15]->SetValue(param.seed);
	
}
void CWWindow::GenerateCalls(wxCommandEvent & WXUNUSED(event))
{
	char *t;
	t = cw_rand_calls(param.number, param.shape, param.seed);
	textctrl->SetValue(wxString::FromAscii(t));
	cw_free(t);
	param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
	spins[15]->SetValue(param.seed);
	
}
void CWWindow::GenerateWords(wxCommandEvent & WXUNUSED(event))
{
	char *t;
	t = cw_rand_words(param.number, param.shape, wordset, param.seed);
	textctrl->SetValue(wxString::FromAscii(t));
	cw_free(t);
	param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
	spins[15]->SetValue(param.seed);
	
}
void CWWindow::Play(wxCommandEvent & WXUNUSED(event))
{
	if(playmode == CWSTOPPED)
	{
		SetStatusText(wxT("Playing."));
		cw_free(text);
		cw_freesample(&asound);
		cw_freesample(&csound);
		cw_initsample(&asound, NULL);
		asound.samplerate = samplerate;
		cw_initsample(&csound, &asound);

		std::cout << textctrl->GetValue();

		text = (char *) malloc((textctrl->GetValue()).Len() * sizeof(char) + 1);
		strcpy(text, (const char *) (textctrl->GetValue()).mb_str(wxConvUTF8));

		morsetext = cw_encode(text);

		cw_signals(&asound, param, morsetext);
		cw_convert(&asound, &csound, bits);
		playmode = cwstudio_play(&csound);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CWWindow::Stop(wxCommandEvent &WXUNUSED(event))
{
	SetStatusText(wxT("Stopped."));
	playmode = cwstudio_stop();

}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CWWindow::Pause(wxCommandEvent &WXUNUSED(event))
{
	playmode = cwstudio_pause();
	if(playmode == CWPLAYING)
		SetStatusText(wxT("Resumed."));
	else if(playmode == CWPAUSED)
		SetStatusText(wxT("Paused."));
	else if(playmode == CWSTOPPED)
		SetStatusText(wxT("Stopped."));
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CWWindow::Update(wxSpinEvent &WXUNUSED(event))
{
	param.agc = spins[0]->GetValue();
	param.click = spins[1]->GetValue();
	param.cspaces = spins[2]->GetValue();
	param.dashlen = spins[3]->GetValue();
	param.detune = spins[4]->GetValue();
	param.even = spins[5]->GetValue();
	param.freq = spins[6]->GetValue();
	param.number = spins[7]->GetValue();
	param.hand = spins[8]->GetValue();
	param.highcut = spins[9]->GetValue();
	param.hum = spins[10]->GetValue();
	param.lowcut = spins[11]->GetValue();
	param.noise = spins[12]->GetValue();
	param.odd = spins[13]->GetValue();
	param.qsb = spins[14]->GetValue();
	param.seed = spins[15]->GetValue();
	param.shape = spins[16]->GetValue();
	param.signals = spins[17]->GetValue();
	param.spacelen = spins[18]->GetValue();
	param.sweep = spins[19]->GetValue();
	param.sweepness = spins[20]->GetValue();
	param.tempo = spins[21]->GetValue();
	param.window = spins[22]->GetValue();
	wordset = spins[23]->GetValue();
	param.wspaces = spins[24]->GetValue();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CWWindow::WAV(wxCommandEvent &WXUNUSED(event))
{
	  wxFileDialog *savedialog = new wxFileDialog(this, wxT("Save WAV file"), wxT(""), wxT(""), wxT("WAV files (*.wav)|*.wav"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

  if (savedialog->ShowModal() == wxID_OK){
      wxString filename = savedialog->GetPath();
      cw_wavout(filename.mb_str(wxConvUTF8), &csound);

  }
}
