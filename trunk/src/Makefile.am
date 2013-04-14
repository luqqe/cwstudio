bin_PROGRAMS = cwstudio
cwstudio_SOURCES = cwstudio.c play.c cwstudio.h
cwstudio_LDADD = libcwgen.a $(PTHREAD_LIBS)

AM_CFLAGS = -Wall -pedantic

noinst_LIBRARIES        = libcwgen.a
libcwgen_a_SOURCES        = fft.c morse.c noise.c random.c sound.c wav.c words.c calls.c init.c cwgen.h
#libcwgen_a_LIBADD        = -lm -lwinmm