[![Download CWStudio](https://img.shields.io/sourceforge/dm/cwstudio.svg)](https://sourceforge.net/projects/cwstudio/files/latest/download)

**CWStudio** is fast, computationally efficient and portable generator of various CW signals for telegraphy training purposes. Besides efficiency and portability, it is designed to simulate as much as possible a real air listening.

It can generate signals of various complexity, from one sine signal without any noise, to AGC-like complex air signal mix with possible hum, chirp, detuning, hand irregularities and so on. It can also generate stereo and multichannel (surround) sound with surround panning.

The project was started in 2008 and after reaching any usability it was registered in April 2010 on SourceForge. From a simple command line tool, it has become a fully functional application with many features. 

* Written in ANSI, pedantic C and C++. CLI is library independent, the other binaries depend only on ncurses or wxWidgets. Designed to be small and portable. Compiles and runs under DOS, Windows, Linux, FreeBSD, OpenBSD, NetBSD, Solaris and OpenSolaris.
* One executable and portable file (with optional lame_enc.dll). No installer, simply download and run.
* Optimized sound routines provide quick sound generation on slower machines. Well-considered memory management allows to use on vintage hardware.
* Many parameters and switches. Simulation of a real AGC-like noise, QSB, chirp, hum, detuning, clicking etc.
* Stereo sound support with phase setting, panning and panning drift. Dots and dahs can be panned separately!
* Built-in database of real callsigns and most often used English words (inside executable!).
* Both text and graphical user interfaces. Curses-compatible library is enough to compile and run. For GUI you need wxWidgets 2.8 or above.
* Direct MP3 export through any external encoder compatible with lame_enc.dll.
* The command line interface allows you to generate training texts in batch mode and pipe them to any external software (lame, ffmpeg etc.).
* Used by increasing number of CW fans all over the world (currently more than 500 downloads per month).
* Free as in freedom and free as in beer. Open source, GPL3.
