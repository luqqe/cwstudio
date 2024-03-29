/*$T cwwx.c GC 1.150 2015-02-06 14:12:15 */

/*$I0

    This file is part of CWStudio.

    Copyright 2008-2016 Lukasz Komsta, SP8QED

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

#ifdef HAVE_WINDOWS_H
extern "C"
{
#include "BladeMP3EncDLL.h"
}
#endif
#ifdef HAVE_LIBWINMM
#ifdef WIN9X
#define SOUND_INTERFACE "/waveout"
#else
#define SOUND_INTERFACE "/waveoutex"
#endif
#elif defined HAVE_PULSEAUDIO
#define SOUND_INTERFACE "/pulseaudio"
#elif defined HAVE_SNDIO
#define SOUND_INTERFACE "/sndio"
#elif defined HAVE_AUDIOIO
#define SOUND_INTERFACE "/audioio"
#elif defined HAVE_OSS
#define SOUND_INTERFACE "/oss"
#elif defined HAVE_COREAUDIO
#define SOUND_INTERFACE "/coreaudio"
#else
#define SOUND_INTERFACE ""
#endif
#ifdef __WXMSW__
#define WXGUI	"/msw"
#elif defined __WXGTK__
#define WXGUI	"/gtk"
#elif defined __WXMOTIF__
#define WXGUI	"/motif"
#elif defined __WXOSX__
#define WXGUI	"/osx"
#elif defined __WXX11__
#define WXGUI	"/x11"
#else
#define WXGUI	""
#endif
extern "C"
{
#include "cwstudio.h"
} using namespace		std;
static char			*text = NULL, *morsetext = NULL;
static cw_sample	asound, csound;
static cw_param		param;
static int			mode = 0, wordset = 100, chars;
static unsigned int bits = 16;
static unsigned int samplerate = 44100;
static int			playmode = CWSTOPPED;

#ifdef HAVE_WINDOWS_H
HINSTANCE			hDLL = NULL;
#endif
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
 Public - currently all :)
 -----------------------------------------------------------------------------------------------------------------------
 */
public:
	CWWindow(const wxString &title, const wxPoint &pos, const wxSize &size);

	void	OnQuit(wxCloseEvent &event);
	void	OnAbout(wxCommandEvent &event);
	void	GenerateGroups(wxCommandEvent &event);
	void	GenerateCalls(wxCommandEvent &event);
	void	GenerateWords(wxCommandEvent &event);
	void	Play(wxCommandEvent &event);
	void	Stop(wxCommandEvent &event);
	void	Pause(wxCommandEvent &event);
	void	UpdateSpins(wxSpinEvent &event);
	void  UpdateComboboxes(wxCommandEvent &event);
	void	WAV(wxCommandEvent &event);
	void	MP3(wxCommandEvent &event);
	void	Load(wxCommandEvent &event);
	void	Save(wxCommandEvent &event);
	void	Reset(wxCommandEvent &event);
	void	OnChanged(wxCommandEvent &event);
	void	GenerateSound();
	void	ReverseUpdate();
	void 	SaveConfig(const char*);
	void 	LoadConfig(const char*);

	DECLARE_EVENT_TABLE()
	wxTextCtrl		*textctrl;
	wxSlider		*sliderwords;

	wxSpinCtrl		*spins[30];
	wxStaticText	*txts[30];
	wxBoxSizer		*spinsizers[10];
	wxComboBox		*boxcharset;
	wxComboBox		*boxsamplerate;
	wxComboBox		*boxbits;

	char			charset[256];

	int sampleratetable[9] = { 8000, 11025, 22050, 44100, 16000, 24000, 48000, 96000, 192000 } ;
	int bitstable[5] = { 0, 8, 16, 24, 32 } ;

	int ShouldGenerate;

/*
 -----------------------------------------------------------------------------------------------------------------------
 Private - currently empty
 -----------------------------------------------------------------------------------------------------------------------
 */
private:
};

class Generator :
	public wxThread
{
/* */
public:

	/* */
	Generator() :
	wxThread(wxTHREAD_JOINABLE)
	{ ;
	}

//void Generator();
protected:
	virtual ExitCode	Entry();
};

enum
{
	ID_Quit		= 1,
	ID_About,
	ID_Play,
	ID_Pause,
	ID_Stop,
	ID_Groups,
	ID_Words,
	ID_Calls,
	ID_Update,
	ID_Combo,
	ID_WAV,
	ID_MP3,
	ID_Load,
	ID_Save,
	ID_Reset,
	ID_Text
};

BEGIN_EVENT_TABLE(CWWindow, wxFrame)
EVT_CLOSE(CWWindow::OnQuit)
EVT_BUTTON(ID_Play, CWWindow::Play)
EVT_BUTTON(ID_Stop, CWWindow::Stop)
EVT_BUTTON(ID_WAV, CWWindow::WAV)
EVT_BUTTON(ID_MP3, CWWindow::MP3)
EVT_BUTTON(ID_Pause, CWWindow::Pause)
EVT_BUTTON(ID_Groups, CWWindow::GenerateGroups)
EVT_BUTTON(ID_Calls, CWWindow::GenerateCalls)
EVT_BUTTON(ID_Words, CWWindow::GenerateWords)
EVT_BUTTON(ID_Load, CWWindow::Load)
EVT_BUTTON(ID_Save, CWWindow::Save)
EVT_BUTTON(ID_Reset, CWWindow::Reset)
EVT_SPINCTRL(ID_Update, CWWindow::UpdateSpins)
EVT_COMBOBOX(ID_Combo, CWWindow::UpdateComboboxes)
EVT_TEXT(ID_Text, CWWindow::OnChanged)
END_EVENT_TABLE()
IMPLEMENT_APP(CWStudio)

/*
 =======================================================================================================================
 OnInit event
 =======================================================================================================================
 */
bool CWStudio::OnInit()
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	CWWindow	*frame = new CWWindow(wxString(wxT("CWStudio")), wxPoint(50, 50), wxSize(1200, 650));

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	frame->Show(TRUE);
	SetTopWindow(frame);
#ifdef HAVE_WINDOWS_H
	hDLL = LoadLibrary(_T("lame_enc.dll"));
#endif
	return TRUE;
}

/*
 =======================================================================================================================
 Main Constructor
 =======================================================================================================================
 */
CWWindow::CWWindow(const wxString &title, const wxPoint &pos, const wxSize &size) :
	wxFrame((wxFrame *) NULL, -1, title, pos, size)
{
	FILE		*f;
	char		filename[255];

	wxString	captions[30];

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	const int	defs[30] =
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
		0,1,0,0,0,0
	};
	const int	mins[30] = { 0, 0, 0, 100, 0, 0, 50, 1, 0, 300, 0, 50, 0, 0, 0, 0, -50, 1, 20, -4000, 0, 5, 0, 0, 0, 1, -720, -180, 0, 0 };
	const int	maxs[30] =
	{
		100,
		100,
		100,
		1000,
		100,
		100,
		4000,
		1000,
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
		100,7,720,180,0,0
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
	captions[25] = wxT(" Channels");
	captions[26] = wxT(" Pan");
	captions[27] = wxT(" Pandrift");
	captions[28] = wxT(" Bits");
	captions[29] = wxT(" Samplerate");

	wxString	charsets[21];
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

	wxString	samplerates[9];
	samplerates[0] = wxT("8000");
	samplerates[1] = wxT("11025");
	samplerates[2] = wxT("22050");
	samplerates[3] = wxT("44100");
	samplerates[4] = wxT("16000");
	samplerates[5] = wxT("24000");
	samplerates[6] = wxT("48000");
	samplerates[7] = wxT("96000");
	samplerates[8] = wxT("192000");

	wxString	bitvalues[5];
	bitvalues[0] = wxT("0");
	bitvalues[1] = wxT("8");
	bitvalues[2] = wxT("16");
	bitvalues[3] = wxT("24");
	bitvalues[4] = wxT("32");

	wxString	tooltips[30];
	tooltips[0] = wxT("Simulate AGC response of receiver by varying noise volume along RMS of the signal. Default is 100.");
	tooltips[1] = wxT("Simulate click by lowering sustain part of tone at given level (in dB) below attack phase. Default is 1 dB.");
	tooltips[2] = wxT("Set additional spaces (one space has a length of a dot) between chars. Default is 0.");
	tooltips[3] = wxT("Length of dash, in percentage of dot. Default is 300 (3:1).");
	tooltips[4] = wxT("Simulate frequency drift of the signal. Default is 0.");
	tooltips[5] = wxT("Enhance the signal by given percent of even harmonics. Default is 0.");
	tooltips[6] = wxT("Use given frequency in Hz.");
	tooltips[7] = wxT("Generate given number of groups/words/calls. Default is 20.");
	tooltips[8] = wxT("Simulate hand transmitting by introduce random errors in dash/dot lengths. (0-100)");
	tooltips[9] = wxT("Set high cutoff frequency (Hz) of generated noise. Default is 2400.");
	tooltips[10] = wxT("Add given percentage of 50 Hz mains hum. Default is 0.");
	tooltips[11] = wxT("Set low cutoff frequency (Hz) of generated noise. Default is 300.");
	tooltips[12] = wxT("Add given percentage of noise. Default is 100.");
	tooltips[13] = wxT("Enhance the signal by given percent of odd harmonics. Default is 0.");
	tooltips[14] = wxT("Add given amount of signal QSB. Default is 0. (0-100)");
	tooltips[15] = wxT("Set explicitly the seed of random generator.");
	tooltips[16] = wxT("Set the shape of random generated numbers. Positive value increases the fraction of higher numbers (last letters, words, calls). Negative values prefer first ones. Zero means flat uniform distribution. Can be used to increase the number of newly learned letters/words. In practice, values -10 to 10 are well working.");
	tooltips[17] = wxT("Generate given number of mixed signals, each of hardcoded frequency ratio, tempo ratio and amplitude ratio. Every signal plays the same text in a loop. Default is 3.");
	tooltips[18] = wxT("Space length (between dots and dashes) in percentage of dot. Default is 100 (1:1).");
	tooltips[19] = wxT("Simulate sweep (chirp or filter ringing according to parameters) by starting each tone from given frequency (can be negative, which results in zero crossing). Frequency is then changed exponentially to this given in 'freq' with a rate given by 'sweepness'. Default is 0.");
	tooltips[20] = wxT("The rate of exponential frequency change from 'sweep' to 'freq'. Default is 0 (no sweep).");
	tooltips[21] = wxT("Tempo of generated main signal in cpm (chars per minute) according to PARIS group.");
	tooltips[22] = wxT("Raised cosine window width (used to avoid clicks in each tone). Default is 100 samples.");
	tooltips[23] = wxT("Take only first given number of most common English words.");
	tooltips[24] = wxT("Set additional spaces between words/groups/calls. Default is zero.");
	tooltips[25] = wxT("Set channel numbers.");
	tooltips[26] = wxT("Panning multichannel angle.");
	tooltips[27] = wxT("Drift of panning after each dot or dash.");
	tooltips[28] = wxT(" ");
	tooltips[29] = wxT(" ");

#if defined(__WXMSW__)
	SetIcon(wxICON(id));
#endif

	wxPanel		*panel = new wxPanel(this, wxID_ANY);

	CreateStatusBar();

	/*
	 * wxMenu *menuFile = new wxMenu;
	 * *menuFile->Append( ID_About, wxT("&About...") );
	 * *menuFile->AppendSeparator();
	 * *menuFile->Append( ID_Quit, wxT("E&xit") );
	 * ;
	 * wxMenuBar *menuBar = new wxMenuBar;
	 * *menuBar->Append( menuFile, wxT("&File") );
	 */
	wxBoxSizer	*mainsizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer	*buttonsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer	*lowbuttonsizer = new wxBoxSizer(wxHORIZONTAL);

	for(int i = 0; i < 6; i++) spinsizers[i] = new wxBoxSizer(wxHORIZONTAL);
	textctrl = new wxTextCtrl(panel, ID_Text, wxT("VVV ="), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	//textctrl->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("Courier New")));
	wxFont* tempfont = new wxFont(14,wxMODERN,wxNORMAL,wxNORMAL);
    	textctrl->SetFont(*tempfont);
	mainsizer->Add(lowbuttonsizer, 0, wxEXPAND, 0);
	for(int i = 0; i < 6; i++) mainsizer->Add(spinsizers[i], 0, wxEXPAND, 0);
	mainsizer->Add(buttonsizer, 0, wxEXPAND, 0);

	mainsizer->Add(textctrl, 1, wxEXPAND, 0);
	panel->SetSizer(mainsizer);

	wxButton	*playbutton = new wxButton(panel, ID_Play, wxT("Play"));
	wxButton	*stopbutton = new wxButton(panel, ID_Stop, wxT("Stop"));
	wxButton	*pausebutton = new wxButton(panel, ID_Pause, wxT("Pause"));
	wxButton	*wavbutton = new wxButton(panel, ID_WAV, wxT("Save WAV"));
#ifdef HAVE_WINDOWS_H
	wxButton	*mp3button = new wxButton(panel, ID_MP3, wxT("Save MP3"));
#endif
	wxButton	*groupsbutton = new wxButton(panel, ID_Groups, wxT("Groups"));
	wxButton	*callsbutton = new wxButton(panel, ID_Calls, wxT("Calls"));
	wxButton	*wordsbutton = new wxButton(panel, ID_Words, wxT("Words"));
	wxButton	*loadbutton = new wxButton(panel, ID_Load, wxT("Load Settings"));
	wxButton	*savebutton = new wxButton(panel, ID_Save, wxT("Save Settings"));
	wxButton	*resetbutton = new wxButton(panel, ID_Reset, wxT("Reset Settings"));

	boxsamplerate = new wxComboBox(panel,ID_Combo,wxT("44100"),wxDefaultPosition,wxDefaultSize,9,samplerates,0,wxDefaultValidator,wxT("x"));
	boxbits = new wxComboBox(panel,ID_Combo,wxT("16"),wxDefaultPosition,wxDefaultSize,5,bitvalues,0,wxDefaultValidator,wxT("x"));

	for(int j = 0; j < 6; j++)
	{
		for(int i = 0; i < 5; i++)
		{
			if ((j<5) || ((j==5) && (i<3)))
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
			spins[5 * j + i]->SetToolTip(tooltips[5 * j + i]);
			}

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
			if ((j==5) && (i==3)) spinsizers[j]->Add(boxbits, 1, wxEXPAND, 0);
			else
			if ((j==5) && (i==4)) spinsizers[j]->Add(boxsamplerate, 1, wxEXPAND, 0);
			else
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
	buttonsizer->Add(groupsbutton, 1, wxEXPAND, 0);
	buttonsizer->Add(callsbutton, 1, wxEXPAND, 0);
	buttonsizer->Add(wordsbutton, 1, wxEXPAND, 0);
	buttonsizer->Add(boxcharset, 1, wxEXPAND, 0);

	lowbuttonsizer->Add(wavbutton, 1, wxEXPAND, 0);
#ifdef HAVE_WINDOWS_H
	lowbuttonsizer->Add(mp3button, 1, wxEXPAND, 0);
#endif
	lowbuttonsizer->Add(loadbutton, 1, wxEXPAND, 0);
	lowbuttonsizer->Add(savebutton, 1, wxEXPAND, 0);
	lowbuttonsizer->Add(resetbutton, 1, wxEXPAND, 0);

	/*
	 * SetMenuBar(menuBar);
	 */
	SetStatusText(wxT("(C) 2008 - 2022 Lukasz Komsta SP8QED. http://cwstudio.sf.net/"));
	SetTitle(wxString(wxT("CWStudio ")) + wxString(wxT(VERSION)) + wxString(wxT(" (")) + wxString(wxT(CANONICAL_HOST)) + wxString(wxT(WXGUI)) + wxString(wxT(SOUND_INTERFACE)) + wxString(wxT(")")));
	param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
	spins[15]->SetValue(param.seed);
	Centre();

	cwstudio_getconfigfile(filename);
	cw_initparam(&param);
	LoadConfig(filename);
	ReverseUpdate();
	ShouldGenerate = 1;
	panel->Layout();
}

void CWWindow::SaveConfig(const char *filename)
{
	FILE *f;
	if((f = fopen(filename, "w")) != NULL) {
		fprintf(f,"mode = %i\n",mode);
		fprintf(f,"wordset = %i\n",wordset);
		fprintf(f,"chars = %i\n",chars);
		fprintf(f,"bits = %i\n",bits);
		fprintf(f,"samplerate = %i\n",samplerate);
		fprintf(f,"agc = %i\n",param.agc);
		fprintf(f,"channels = %i\n",param.channels);
		fprintf(f,"click = %i\n",param.click);
		fprintf(f,"cspaces = %i\n",param.cspaces);
		fprintf(f,"dashlen = %i\n",param.dashlen);
		fprintf(f,"detune = %i\n",param.detune);
		fprintf(f,"even = %i\n",param.even);
		fprintf(f,"freq = %i\n",param.freq);
		fprintf(f,"hand = %i\n",param.hand);
		fprintf(f,"highcut = %i\n",param.highcut);
		fprintf(f,"hum = %i\n",param.hum);
		fprintf(f,"lowcut = %i\n",param.lowcut);
		fprintf(f,"noise = %i\n",param.noise);
		fprintf(f,"number = %i\n",param.number);
		fprintf(f,"odd = %i\n",param.odd);
		fprintf(f,"pan = %i\n",param.pan);
		fprintf(f,"pandrift = %i\n",param.pandrift);
		fprintf(f,"qsb = %i\n",param.qsb);
		fprintf(f,"seed = %i\n",param.seed);
		fprintf(f,"shape = %i\n",param.shape);
		fprintf(f,"signals = %i\n",param.signals);
		fprintf(f,"spacelen = %i\n",param.spacelen);
		fprintf(f,"sweep = %i\n",param.sweep);
		fprintf(f,"sweepness = %i\n",param.sweepness);
		fprintf(f,"tempo = %i\n",param.tempo);
		fprintf(f,"window = %i\n",param.window);
		fprintf(f,"wspaces = %i\n",param.wspaces);
		fprintf(f, "charset = %s\n", (const char *) boxcharset->GetValue().mb_str(wxConvUTF8));
		fclose(f);
	}
}

void CWWindow::LoadConfig(const char *filename)
{
	FILE *f;
	char buffer[256];
	if((f = fopen(filename, "r")) != NULL) {
		while(fgets(buffer, 256, f) != NULL) {
		sscanf(buffer," mode = %i",&mode);
		sscanf(buffer," wordset = %i",&wordset);
		sscanf(buffer," chars = %i",&chars);
		sscanf(buffer," bits = %i",&bits);
		sscanf(buffer," samplerate = %i",&samplerate);
		sscanf(buffer," agc = %i",&param.agc);
		sscanf(buffer," channels = %i",&param.channels);
		sscanf(buffer," click = %i",&param.click);
		sscanf(buffer," cspaces = %i ",&param.cspaces);
		sscanf(buffer," dashlen = %i",&param.dashlen);
		sscanf(buffer," detune = %i",&param.detune);
		sscanf(buffer," even = %i",&param.even);
		sscanf(buffer," freq = %i",&param.freq);
		sscanf(buffer," hand = %i",&param.hand);
		sscanf(buffer," highcut = %i",&param.highcut);
		sscanf(buffer," hum = %i",&param.hum);
		sscanf(buffer," lowcut = %i",&param.lowcut);
		sscanf(buffer," noise = %i",&param.noise);
		sscanf(buffer," number = %i",&param.number);
		sscanf(buffer," odd = %i",&param.odd);
		sscanf(buffer," pan = %i",&param.pan);
		sscanf(buffer," pandrift = %i",&param.pandrift);
		sscanf(buffer," qsb = %i",&param.qsb);
		sscanf(buffer," seed = %i",&param.seed);
		sscanf(buffer," shape = %i",&param.shape);
		sscanf(buffer," signals = %i",&param.signals);
		sscanf(buffer," spacelen = %i",&param.spacelen);
		sscanf(buffer," sweep = %i",&param.sweep);
		sscanf(buffer," sweepness = %i",&param.sweepness);
		sscanf(buffer," tempo = %i",&param.tempo);
		sscanf(buffer," window = %i",&param.window);
		sscanf(buffer," wspaces = %i",&param.wspaces);
		sscanf(buffer," charset = %s",charset);
		}
		fclose(f);
		boxcharset->SetValue(wxString::FromUTF8(charset));
}
}



/*
 =======================================================================================================================
 Save config on quit
 =======================================================================================================================
 */
void CWWindow::OnQuit(wxCloseEvent &event)
{
	/*~~~~~~~*/
	FILE	*f;

	/*~~~~~~~*/
	char	filename[255];
#ifdef WIN32
	sprintf(filename, "%s%s%s",getenv("HOMEDRIVE"),getenv("HOMEPATH"),"\\cwstudio.ini");
#else
	char homedir[255];
	//if ((homedir = getenv("HOME")) == NULL) homedir = getpwuid(getuid())->pw_dir;
	sprintf(filename, "%s%s",getenv("HOME"),"/.cwstudio");
#endif
	SaveConfig(filename);

#ifdef HAVE_WINDOWS_H
	FreeLibrary(hDLL);
#endif
	Destroy();

	/*
	 * Close(TRUE);
	 */
}

/*
 =======================================================================================================================
 Generate random groups
 =======================================================================================================================
 */
void CWWindow::GenerateGroups(wxCommandEvent &WXUNUSED(event))
{
	char	*t;
	t = cw_rand_groups(param.number, param.shape, (const char *) boxcharset->GetValue().mb_str(wxConvUTF8), param.seed);
	textctrl->SetValue(wxString::FromAscii(t));
	cw_free(t);
	param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
	spins[15]->SetValue(param.seed);
}

/*
 =======================================================================================================================
 Generate random callsigns
 =======================================================================================================================
 */
void CWWindow::GenerateCalls(wxCommandEvent &WXUNUSED(event))
{
	char	*t;
	t = cw_rand_calls(param.number, param.shape, param.seed);
	textctrl->SetValue(wxString::FromAscii(t));
	cw_free(t);
	param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
	spins[15]->SetValue(param.seed);
}

/*
 =======================================================================================================================
 Generate random words
 =======================================================================================================================
 */
void CWWindow::GenerateWords(wxCommandEvent &WXUNUSED(event))
{
	char	*t;
	t = cw_rand_words(param.number, param.shape, wordset, param.seed);
	textctrl->SetValue(wxString::FromAscii(t));
	cw_free(t);
	param.seed = (((unsigned int) (time(NULL) << 12)) % 32767) + 1;
	spins[15]->SetValue(param.seed);
}

/*
 =======================================================================================================================
 Text in textcontrol was changed
 =======================================================================================================================
 */
void CWWindow::OnChanged(wxCommandEvent &WXUNUSED(event))
{
	ShouldGenerate = 1;
	SetStatusText(wxT(""));

}

/*
 =======================================================================================================================
 Generate sound
 =======================================================================================================================
 */
void CWWindow::GenerateSound()
{
		SetStatusText(wxT("Generating."));

		cw_free(text);
		text = (char *) malloc((textctrl->GetValue()).Len() * sizeof(char) + 1);
		strcpy(text, (const char *) (textctrl->GetValue()).mb_str(wxConvUTF8));

		/*
		cw_freesample(&asound);
		cw_freesample(&csound);
		cw_initsample(&asound, NULL);
		asound.samplerate = samplerate;

		cw_initsample(&csound, &asound);
		morsetext = cw_encode(text);
		cw_signals(&asound, param, morsetext);

		cw_convert(&asound, &csound, bits);
*/
		Generator	*gen = new Generator();
		gen->Create();
		gen->Run();
		gen->Wait();

		ShouldGenerate = 0;

		delete gen;

}

/*
 =======================================================================================================================
 Play audio
 =======================================================================================================================
 */
void CWWindow::Play(wxCommandEvent &WXUNUSED(event))
{
	if(playmode == CWPLAYING) playmode = cwstudio_stop();
	if(playmode == CWSTOPPED)
	{
		if(ShouldGenerate) GenerateSound();
		playmode = cwstudio_play(&csound);
		SetStatusText(wxT("Playing."));

	}
}

/*
 =======================================================================================================================
 Stop audio
 =======================================================================================================================
 */
void CWWindow::Stop(wxCommandEvent &WXUNUSED(event))
{
	SetStatusText(wxT("Stopped."));
	playmode = cwstudio_stop();
}

/*
 =======================================================================================================================
 Pause audio
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
 Update internal variables (after change of any wxSpinCtrl value)
 =======================================================================================================================
 */
void CWWindow::UpdateSpins(wxSpinEvent &WXUNUSED(event))
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
	param.channels = spins[25]->GetValue();
	param.pan = spins[26]->GetValue();
	param.pandrift = spins[27]->GetValue();
	ShouldGenerate = 1;
	SetStatusText(wxT(""));
}

/*
 =======================================================================================================================
 Update samplerate and bits
 =======================================================================================================================
 */
void CWWindow::UpdateComboboxes(wxCommandEvent &WXUNUSED(event))
{
	int s;

	if ((s = boxbits->GetSelection()) == wxNOT_FOUND) bits = 16; else bits = bitstable[s];
	if ((s = boxsamplerate->GetSelection()) == wxNOT_FOUND) samplerate = 44100; else samplerate = sampleratetable[s];
	ShouldGenerate = 1;
	SetStatusText(wxT(""));
}

void CWWindow::ReverseUpdate()
{
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
			spins[25]->SetValue(param.channels);
			spins[26]->SetValue(param.pan);
			spins[27]->SetValue(param.pandrift);
			boxsamplerate->SetValue(wxString::Format(wxT("%i"),samplerate));
			boxbits->SetValue(wxString::Format(wxT("%i"),bits));
}

/*
 =======================================================================================================================
 Save to WAV file
 =======================================================================================================================
 */
void CWWindow::WAV(wxCommandEvent &WXUNUSED(event))
{
	wxFileDialog	*savedialog = new wxFileDialog
		(
			this,
			wxT("Save WAV file"),
			wxT(""),
			wxT(""),
			wxT("WAV files (*.wav)|*.wav"),
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT
		);

	if(savedialog->ShowModal() == wxID_OK)
	{
		if(ShouldGenerate) GenerateSound();
		wxString	filename = savedialog->GetPath();
		cw_wavout(filename.mb_str(wxConvUTF8), &csound);
		SetStatusText(wxT("WAV file written."));

	}
}

/*

 =======================================================================================================================
 Save to MP3 file. Based on lame_enc.dll example by A. L. Faber
 =======================================================================================================================
 */
void CWWindow::MP3(wxCommandEvent &WXUNUSED(event))
{
#ifdef HAVE_WINDOWS_H
	BEINITSTREAM		beInitStream = NULL;
	BEENCODECHUNK		beEncodeChunk = NULL;
	BEDEINITSTREAM		beDeinitStream = NULL;
	BECLOSESTREAM		beCloseStream = NULL;
	BEVERSION			beVersion = NULL;
	BEWRITEVBRHEADER	beWriteVBRHeader = NULL;
	BE_ERR				err = 0;
	BE_CONFIG			beConfig = { 0, };
	FILE				*pFileOut = NULL;
	DWORD				dwSamples = 0;
	DWORD				dwMP3Buffer = 0;
	HBE_STREAM			hbeStream = 0;
	PBYTE				pMP3Buffer = NULL;
	PSHORT				pWAVBuffer = NULL;


	if((bits != 16) || (samplerate != 44100) || (param.channels > 2))
	{
		wxMessageBox(_T("Unsupported channels/rate/bits!"), _T("MP3 Export"), wxOK | wxICON_ERROR);
		return;
	}
	if(hDLL == NULL)
	{
		wxMessageBox(_T("Please download lame_enc.dll!"), _T("MP3 Export"), wxOK | wxICON_ERROR);
		return;
	}

	beInitStream = (BEINITSTREAM) GetProcAddress(hDLL, TEXT_BEINITSTREAM);
	beEncodeChunk = (BEENCODECHUNK) GetProcAddress(hDLL, TEXT_BEENCODECHUNK);
	beDeinitStream = (BEDEINITSTREAM) GetProcAddress(hDLL, TEXT_BEDEINITSTREAM);
	beCloseStream = (BECLOSESTREAM) GetProcAddress(hDLL, TEXT_BECLOSESTREAM);
	beVersion = (BEVERSION) GetProcAddress(hDLL, TEXT_BEVERSION);
	beWriteVBRHeader = (BEWRITEVBRHEADER) GetProcAddress(hDLL, TEXT_BEWRITEVBRHEADER);
	if(!beInitStream || !beEncodeChunk || !beDeinitStream || !beCloseStream || !beVersion || !beWriteVBRHeader)
	{
		wxMessageBox(_T("Unable to load lame_enc.dll functions!"), _T("MP3 Export"), wxOK | wxICON_ERROR);
		return;
	}

	wxFileDialog	*savedialog = new wxFileDialog
		(
			this,
			wxT("Save MP3 file"),
			wxT(""),
			wxT(""),
			wxT("MP3 files (*.mp3)|*.mp3"),
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT
		);

	if(savedialog->ShowModal() == wxID_OK)
	{
		if(ShouldGenerate) GenerateSound();

		wxString	filename = savedialog->GetPath();
		pFileOut = fopen(filename.mb_str(wxConvUTF8), "wb+");
		memset(&beConfig, 0, sizeof(beConfig));

		beConfig.dwConfig = BE_CONFIG_LAME;
		beConfig.format.LHV1.dwStructVersion = 1;
		beConfig.format.LHV1.dwStructSize = sizeof(beConfig);
		beConfig.format.LHV1.dwSampleRate = csound.samplerate;
		beConfig.format.LHV1.dwReSampleRate = 0;
		beConfig.format.LHV1.nMode = param.channels > 1 ? BE_MP3_MODE_JSTEREO : BE_MP3_MODE_MONO;
		beConfig.format.LHV1.dwBitrate = 128;
		beConfig.format.LHV1.dwMaxBitrate = 320;
		beConfig.format.LHV1.nPreset = LQP_NOPRESET;
		beConfig.format.LHV1.dwMpegVersion = MPEG1;
		beConfig.format.LHV1.dwPsyModel = 0;
		beConfig.format.LHV1.dwEmphasis = 0;
		beConfig.format.LHV1.bOriginal = TRUE;
		beConfig.format.LHV1.bCRC = TRUE;
		beConfig.format.LHV1.bCopyright = TRUE;
		beConfig.format.LHV1.bPrivate = TRUE;
		beConfig.format.LHV1.bWriteVBRHeader = FALSE;
		beConfig.format.LHV1.bEnableVBR = FALSE;
		beConfig.format.LHV1.nVBRQuality = 5;
		beConfig.format.LHV1.bNoRes = TRUE;

		err = beInitStream(&beConfig, &dwSamples, &dwMP3Buffer, &hbeStream);
		pMP3Buffer = new BYTE[dwMP3Buffer];

		DWORD	dwRead = 0;
		DWORD	dwWrite = 0;
		DWORD	dwDone = 0;
		pWAVBuffer = (PSHORT) csound.data;

		long int	length = csound.length;

		while(length >= dwSamples)
		{
			err = beEncodeChunk(hbeStream, dwSamples, pWAVBuffer, pMP3Buffer, &dwWrite);

			if(err != BE_ERR_SUCCESSFUL)
			{
				beCloseStream(hbeStream);
				wxMessageBox(_T("beEncodeChunk() failed!"), _T("MP3 Export"), wxOK | wxICON_ERROR);
				return;
			}

			if(fwrite(pMP3Buffer, 1, dwWrite, pFileOut) != dwWrite)
			{
				wxMessageBox(_T("Output file write error!"), _T("MP3 Export"), wxOK | wxICON_ERROR);
				return;
			}

			dwDone += dwRead * sizeof(SHORT);

			SetStatusText(wxString::Format(wxT("%i samples to encode..."), length));

			length -= dwSamples;
			pWAVBuffer += dwSamples;
		}

		if(length > 0)
		{
			err = beEncodeChunk(hbeStream, length, pWAVBuffer, pMP3Buffer, &dwWrite);
			if(fwrite(pMP3Buffer, 1, dwWrite, pFileOut) != dwWrite)
			{
				wxMessageBox(_T("Output file write error!"), _T("MP3 Export"), wxOK | wxICON_ERROR);
				return;
			}
		}

		err = beDeinitStream(hbeStream, pMP3Buffer, &dwWrite);
		if(err != BE_ERR_SUCCESSFUL)
		{
			beCloseStream(hbeStream);
			wxMessageBox(wxString::Format(wxT("Error %lu!!!"), err), _T("MP3 Export"), wxOK | wxICON_ERROR);
			return;
		}

		if(dwWrite)
		{
			if(fwrite(pMP3Buffer, 1, dwWrite, pFileOut) != dwWrite)
			{
				wxMessageBox(_T("Output file write error!"), _T("MP3 Export"), wxOK | wxICON_ERROR);
				return;
			}

		}


		beCloseStream(hbeStream);
		delete[] pMP3Buffer;
		fclose(pFileOut);

		SetStatusText(wxT("MP3 file written."));

		//beWriteVBRHeader(filename.mb_str(wxConvUTF8));
	}
#endif
}

/*
 =======================================================================================================================
 Load Settings
 =======================================================================================================================
 */
void CWWindow::Load(wxCommandEvent &WXUNUSED(event))
{
	/*~~~~~~~*/
	FILE			*f;

	/*~~~~~~~*/
	wxFileDialog	*savedialog = new wxFileDialog
		(
			this,
			wxT("Load Settings"),
			wxT(""),
			wxT(""),
			wxT("Config files (*.cfg)|*.cfg"),
			wxFD_OPEN
		);

	if(savedialog->ShowModal() == wxID_OK)
	{
		wxString	filename = savedialog->GetPath();
		LoadConfig(((const char*)filename.mb_str(wxConvUTF8)));
		ShouldGenerate = 1;
		SetStatusText(wxT("Loaded."));
	}
}

/*
 =======================================================================================================================
 Save Settings
 =======================================================================================================================
 */
void CWWindow::Save(wxCommandEvent &WXUNUSED(event))
{
	/*~~~~~~~*/
	FILE			*f;

	/*~~~~~~~*/
	wxFileDialog	*savedialog = new wxFileDialog
		(
			this,
			wxT("Save Settings"),
			wxT(""),
			wxT(""),
			wxT("Config files (*.cfg)|*.cfg"),
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT
		);

	if(savedialog->ShowModal() == wxID_OK)
	{
		wxString	filename = savedialog->GetPath();
		SaveConfig((const char*)filename.mb_str(wxConvUTF8));
		SetStatusText(wxT("Saved."));

	}
}

/*
 =======================================================================================================================
 Reset Settings to default values
 =======================================================================================================================
 */
void CWWindow::Reset(wxCommandEvent &WXUNUSED(event))
{
	wxMessageDialog *dialog = new wxMessageDialog
		(
			NULL,
			wxT("Are you sure to reset settings?"),
			wxT("Question"),
			wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION
		);

	if(dialog->ShowModal() == wxID_YES)
	{
		cw_initparam(&param);
		samplerate = 44100;
		bits = 16;
		ReverseUpdate();
		ShouldGenerate = 1;
		SetStatusText(wxT("Reset done."));

	}
}

/* Thread for creating audio data - entry point */
void *Generator::Entry()
{
	cw_freesample(&asound);
	cw_freesample(&csound);
	cw_initsample(&asound, NULL);
	asound.samplerate = samplerate;
	cw_initsample(&csound, &asound);
	morsetext = cw_encode(text);
	cw_signals(&asound, param, morsetext);
	cw_convert(&asound, &csound, bits);
	return(NULL);
}
