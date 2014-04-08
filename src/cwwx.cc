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
	void	Update(wxCommandEvent &event);
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


/*
 * EVT_MENU(ID_Quit, CWWindow::OnQuit) ;
 * EVT_MENU(ID_About, CWWindow::OnAbout)
 */
EVT_BUTTON(ID_Play, CWWindow::Play)
EVT_BUTTON(ID_Stop, CWWindow::Stop)
EVT_BUTTON(ID_WAV, CWWindow::WAV)
EVT_BUTTON(ID_Pause, CWWindow::Pause)
EVT_BUTTON(ID_Groups, CWWindow::GenerateGroups)

EVT_BUTTON(ID_Calls, CWWindow::GenerateCalls)

EVT_BUTTON(ID_Words, CWWindow::GenerateWords)

EVT_SLIDER(ID_Update, CWWindow::Update)
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

	captions[0] = wxString(" AGC");
	captions[1] = wxString(" Click");
	captions[2] = wxString(" Cspaces");
	captions[3] = wxString(" Dashlen");
	captions[4] = wxString(" Detune");
	captions[5] = wxString(" Even");
	captions[6] = wxString(" Freq");
	captions[7] = wxString(" Number");
	captions[8] = wxString(" Hand");
	captions[9] = wxString(" Highcut");
	captions[10] = wxString(" Hum");
	captions[11] = wxString(" Lowcut");
	captions[12] = wxString(" Noise");
	captions[13] = wxString(" Odd");
	captions[14] = wxString(" QSB");
	captions[15] = wxString(" Seed");
	captions[16] = wxString(" Shape");
	captions[17] = wxString(" Signals");
	captions[18] = wxString(" Spacelen");
	captions[19] = wxString(" Sweep");
	captions[20] = wxString(" Sweepness");
	captions[21] = wxString(" Tempo");
	captions[22] = wxString(" Window");
	captions[23] = wxString(" Wordset");
	captions[24] = wxString(" Wspaces");
	wxString charsets[21];
	charsets[0] = wxString("abstgjnokqfmzixdrhewlypvcu8219376450?!/=");
	charsets[1] = wxString("abstgjnokqfmzixdrhewlypvcu8219376450?!");
	charsets[2] = wxString("abstgjnokqfmzixdrhewlypvcu8219376450");
	charsets[3] = wxString("abstgjnokqfmzixdrhewlypvcu82193764");
	charsets[4] = wxString("abstgjnokqfmzixdrhewlypvcu821937");
	charsets[5] = wxString("abstgjnokqfmzixdrhewlypvcu8219");
	charsets[6] = wxString("abstgjnokqfmzixdrhewlypvcu82");
	charsets[7] = wxString("abstgjnokqfmzixdrhewlypvcu");
	charsets[8] = wxString("abstgjnokqfmzixdrhewlypvc");
	charsets[9] = wxString("abstgjnokqfmzixdrhewlyp");
	charsets[10] = wxString("abstgjnokqfmzixdrhewl");
	charsets[11] = wxString("abstgjnokqfmzixdrhe");
	charsets[12] = wxString("abstgjnokqfmzixdr");
	charsets[13] = wxString("abstgjnokqfmzix");
	charsets[14] = wxString("abstgjnokqfmz");
	charsets[15] = wxString("abstgjnokqf");
	charsets[16] = wxString("abstgjnok");
	charsets[17] = wxString("abstgjn");
	charsets[18] = wxString("abstg");
	charsets[19] = wxString("abst");
	charsets[20] = wxString("abs");
	
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

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	wxBoxSizer	*buttonsizer = new wxBoxSizer(wxHORIZONTAL);
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	for(int i = 0; i < 5; i++) spinsizers[i] = new wxBoxSizer(wxHORIZONTAL);
	textctrl = new wxTextCtrl(panel, wxID_ANY, wxT("VVV ="), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	textctrl->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("Courier New")));
	

	mainsizer->Add(buttonsizer, 0, 0, 0);
	for(int i = 0; i < 5; i++) mainsizer->Add(spinsizers[i], 0, 0, 0);

	
	mainsizer->Add(textctrl, 1, wxEXPAND, 0);
	panel->SetSizer(mainsizer);
	wxButton * playbutton = new wxButton(panel, ID_Play, "Play");

	wxButton	*stopbutton = new wxButton(panel, ID_Stop, "Stop");
	wxButton	*pausebutton = new wxButton(panel, ID_Pause, "Pause");
	wxButton	*wavbutton = new wxButton(panel, ID_WAV, "Save WAV");
	wxButton	*groupsbutton = new wxButton(panel, ID_Groups, "Groups");
	wxButton *callsbutton = new wxButton(panel, ID_Calls, "Calls");
	wxButton *wordsbutton = new wxButton(panel, ID_Words, "Words");
	
	for(int j = 0; j < 5; j++)
	{
		for(int i = 0; i < 5; i++)
		{
			spins[5 * j + i] = new wxSpinCtrl
				(
					panel,
					wxID_ANY,
					wxEmptyString,
					wxDefaultPosition,
					wxDefaultSize,
					wxSP_ARROW_KEYS,
					mins[5 * j + i],
					maxs[5 * j + i],
					defs[5 * j + i],
					"wxSpinCtrl"
				);
			spins[5 * j + i]->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &CWWindow::Update, this);
			txts[5 * j + i] = new wxStaticText
				(
					panel,
					wxID_ANY,
					captions[5 * j + i],
					wxDefaultPosition,
					wxDefaultSize,
					0,
					"staticText"
				);
			spinsizers[j]->Add(txts[5 * j + i], 1, wxEXPAND, 0);
			spinsizers[j]->Add(spins[5 * j + i], 1, wxEXPAND, 0);
		}
	}

	boxcharset = new wxComboBox
		(
			panel,
			wxID_ANY,
			"abstgjnokqfmzixdrhewlypvcu8219376450?!/=",
			wxDefaultPosition,
			wxDefaultSize,
			20,
			charsets,
			0,
			wxDefaultValidator,
			"x"
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
	SetLabel(wxString("CWStudio ") + wxString(VERSION) + wxString(" (") + wxString(CANONICAL_HOST) + wxString(")"));
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
		boxcharset->SetValue(charset);
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
	textctrl->SetValue(t);
	cw_free(t);
	param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
	spins[15]->SetValue(param.seed);
	
}
void CWWindow::GenerateCalls(wxCommandEvent & WXUNUSED(event))
{
	char *t;
	t = cw_rand_calls(param.number, param.shape, param.seed);
	textctrl->SetValue(t);
	cw_free(t);
	param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
	spins[15]->SetValue(param.seed);
	
}
void CWWindow::GenerateWords(wxCommandEvent & WXUNUSED(event))
{
	char *t;
	t = cw_rand_words(param.number, param.shape, wordset, param.seed);
	textctrl->SetValue(t);
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
void CWWindow::Update(wxCommandEvent &WXUNUSED(event))
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
	  wxFileDialog *savedialog = new wxFileDialog(this, _("Save WAV file"), "", "", "WAV files (*.wav)|*.wav", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

  if (savedialog->ShowModal() == wxID_OK){
      wxString filename = savedialog->GetPath();
      cw_wavout(filename.mb_str(wxConvUTF8), &csound);

  }
}
