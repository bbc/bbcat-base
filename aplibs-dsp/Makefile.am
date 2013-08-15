pkgincludedir = $(includedir)/aplibs-dsp-@APLIBS_DSP_MAJORMINOR@/aplibs-dsp

lib_LTLIBRARIES = libap-dsp-@APLIBS_DSP_MAJORMINOR@.la

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_LDFLAGS = \
	-no-undefined \
	-version-info $(APLIBS_DSP_LIBVERSION)

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_LIBADD = \
	$(SAMPLERATE_LIBS) \
	$(FFTW_LIBS)

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_CPPFLAGS = $(APLIBS_DSP_CFLAGS) \
	$(SAMPLERATE_CFLAGS) \
	$(FFTW_CFLAGS)

if ENABLE_GPL
  GPL_SRC = \
	ConvolverFFT.cpp \
	Convolver.cpp
  GPL_HDR = \
  	ConvolverFFT.hpp \
	Convolver.hpp
else
  GPL_SRC = 
  GPL_HDR = 
endif


libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_SOURCES = \
	Crossfader.cpp \
	FractionalDelay.cpp \
	$(GPL_SRC)



pkginclude_HEADERS = \
	Crossfader.hpp \
	FractionalDelay.hpp \
	Logging.hpp \
	$(GPL_HDR)

noinst_HEADERS =

nodist_pkginclude_HEADERS = aplibs-dsp-stdint.h

DISTCLEANFILES = aplibs-dsp-stdint.h
