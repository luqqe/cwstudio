bin_PROGRAMS = cwstudio cwstudiowx

cwstudio_SOURCES = cwstudio.c play.c cwstudio.h
cwstudio_LDADD = libcwgen.a $(PTHREAD_LIBS) $(CURSES_LIB)

cwstudiowx_SOURCES = cwstudiowx.cc
cwstudiowx_CPPFLAGS = $(WX_CPPFLAGS)
cwstudiowx_LDADD = play.o libcwgen.a $(WX_LIBS)

AM_CFLAGS = -Wall -pedantic

noinst_LIBRARIES        = libcwgen.a
libcwgen_a_SOURCES        = fft.c morse.c noise.c random.c sound.c wav.c words.c calls.c init.c cwgen.h
#libcwgen_a_LIBADD        = -lm -lwinmm

if HAVE_WINDRES
cwstudio_SOURCES += cwstudio-res.rc
cwstudiowx_SOURCES += cwstudiowx-res.rc
endif

.rc.o:
	windres $^ -o $@
