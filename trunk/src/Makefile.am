bin_PROGRAMS = cwcli cwcurses cwwx

cwcurses_SOURCES = cwstudio.c cwcurses.c play.c cwstudio.h
cwcurses_LDADD = libcwgen.a $(PTHREAD_LIBS) $(CURSES_LIB)

cwcli_SOURCES = cwstudio.c cwcli.c play.c cwstudio.h
cwcli_LDADD = libcwgen.a $(PTHREAD_LIBS) $(CURSES_LIB)

cwwx_SOURCES = cwwx.cc
cwwx_CPPFLAGS = $(WX_CPPFLAGS)
cwwx_LDADD = play.o libcwgen.a $(WX_LIBS)

AM_CFLAGS = -Wall -pedantic

noinst_LIBRARIES        = libcwgen.a
libcwgen_a_SOURCES        = fft.c morse.c noise.c random.c sound.c wav.c words.c calls.c init.c cwgen.h
#libcwgen_a_LIBADD        = -lm -lwinmm

if HAVE_WINDRES
cwcurses_SOURCES += cwcurses-res.rc
cwcli_SOURCES += cwcli-res.rc
cwwx_SOURCES += cwwx-res.rc
endif

.rc.o:
	windres $^ -o $@
