pkgincludedir = $(includedir)/aplibs-dsp-@APLIBS_DSP_MAJORMINOR@/aplibs-dsp

lib_LTLIBRARIES = libap-dsp-@APLIBS_DSP_MAJORMINOR@.la

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_LDFLAGS = \
	-no-undefined \
	-version-info $(APLIBS_DSP_LIBVERSION) \
        $(APLIBS_DSP_LDFLAGS)

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_LIBADD = \
	$(FFTW_LIBS)

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_CPPFLAGS = $(APLIBS_DSP_CFLAGS) \
	$(FFTW_CFLAGS)

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_SOURCES = \
	Crossfader.cpp \
	$(GPL_SRC)

pkginclude_HEADERS = \
	Crossfader.hpp \
	Logging.hpp \
	$(GPL_HDR)

noinst_HEADERS =

nodist_pkginclude_HEADERS = aplibs-dsp-stdint.h

DISTCLEANFILES = aplibs-dsp-stdint.h
