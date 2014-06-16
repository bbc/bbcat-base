pkgincludedir = $(includedir)/aplibs-dsp-@APLIBS_DSP_MAJORMINOR@/aplibs-dsp

lib_LTLIBRARIES = libap-dsp-@APLIBS_DSP_MAJORMINOR@.la

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_LDFLAGS =	\
	-no-undefined								\
	-version-info $(APLIBS_DSP_LIBVERSION)		\
    $(APLIBS_DSP_LDFLAGS)

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_LIBADD =	\
	$(PTHREAD_LIBS)

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_CPPFLAGS =	\
	$(APLIBS_DSP_CFLAGS)						\
	$(PTHREAD_CFLAGS)

if ENABLE_GPL
libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_CPPFLAGS += $(APF_CFLAGS) $(SNDFILE_CFLAGS) $(FFTW_CFLAGS)
libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_LIBADD += $(APF_LIBS) $(SNDFILE_LIBS) $(FFTW_LIBS)
GPL_SRC = Convolver.cpp
GPL_HDR = Convolver.h
else
GPL_SRC = 
GPL_HDR = 
endif

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_SOURCES =	\
	3DPosition.cpp								\
	ByteSwap.cpp								\
	Crossfader.cpp								\
	FractionalSample.cpp						\
	misc.cpp									\
	ParameterSet.cpp							\
	SoundDelayBuffer.cpp						\
	SoundFormatConversions.cpp					\
	SoundFormatRawConversions.cpp				\
	ThreadLock.cpp								\
	VBAPanner.cpp								\
	$(GPL_SRC)

pkginclude_HEADERS =							\
	3DPosition.h								\
	ByteSwap.h									\
	Crossfader.hpp								\
	FractionalSample.h							\
	Logging.hpp									\
	misc.h										\
	ParameterSet.h								\
	PositionCursor.h							\
	SoundDelayBuffer.h							\
	SoundFormatConversions.h					\
	SoundFormatRawConversions.h					\
	ThreadLock.h								\
	VBAPanner.h									\
	$(GPL_HDR)

noinst_HEADERS =

nodist_pkginclude_HEADERS = aplibs-dsp-stdint.h

DISTCLEANFILES = aplibs-dsp-stdint.h
