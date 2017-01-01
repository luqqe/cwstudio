EXTRA_DIST = cwcli.ico cwcurses.ico cwwx.ico BladeMP3EncDLL.h

bin_PROGRAMS = cwcli

if COMPILE_CURSES
bin_PROGRAMS += cwcurses
cwcurses_SOURCES = cwcurses.c play.c cwstudio.h
cwcurses_LDADD = libcwgen.a $(PTHREAD_LIBS) $(CURSES_LIB)
if HAVE_WINDRES
cwcurses_SOURCES += cwcurses-res.rc
endif
endif

if COMPILE_WX
bin_PROGRAMS += cwwx
cwwx_SOURCES = cwwx.cc
cwwx_CPPFLAGS = $(WX_CPPFLAGS)
cwwx_LDADD = play.o libcwgen.a $(WX_LIBS)
if HAVE_WINDRES
cwwx_SOURCES += cwwx-res.rc
endif
endif


cwcli_SOURCES = cwcli.c play.c cwstudio.h
cwcli_LDADD = libcwgen.a $(PTHREAD_LIBS) $(CURSES_LIB)


#AM_CFLAGS = -static-libgcc
#AM_CXXFLAGS =  -static-libgcc -static-libstdc++

noinst_LIBRARIES        = libcwgen.a
libcwgen_a_SOURCES        = fft.c morse.c noise.c random.c sound.c wav.c words.c calls.c init.c cwgen.h
#libcwgen_a_LDFLAGS		  = -shared
#libcwgen_a_LIBADD        = -lm -lwinmm

if HAVE_WINDRES
cwcli_SOURCES += cwcli-res.rc
endif

.rc.o:
	$(WINDRES) $^ -o $@
