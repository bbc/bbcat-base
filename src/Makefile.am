pkgincludedir = $(includedir)/aplibs-dsp-@APLIBS_DSP_MAJORMINOR@/aplibs-dsp

lib_LTLIBRARIES = libap-dsp-@APLIBS_DSP_MAJORMINOR@.la

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_LDFLAGS =	\
	-no-undefined								\
	-version-info $(APLIBS_DSP_LIBVERSION)		\
    $(APLIBS_DSP_LDFLAGS)

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_LIBADD =	\
	$(PTHREAD_LIBS)								\
	$(SOFA_LIBS)								\
	$(RT_LIBS)

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_CPPFLAGS =	\
	$(APLIBS_DSP_CFLAGS)						\
	$(SOFA_CFLAGS)								\
	$(PTHREAD_CFLAGS)

libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_SOURCES =	\
	3DPosition.cpp								\
	ByteSwap.cpp								\
	Crossfader.cpp								\
	DistanceModel.cpp							\
	EnhancedFile.cpp							\
	FractionalSample.cpp						\
	misc.cpp									\
	ParameterSet.cpp							\
	PerformanceMonitor.cpp						\
	SoundDelayBuffer.cpp						\
	SoundFormatConversions.cpp					\
	SoundFormatRawConversions.cpp				\
	SoundMixing.cpp								\
	ThreadLock.cpp								\
	VBAPanner.cpp								\
	VBAPanner2D.cpp

pkginclude_HEADERS =							\
	3DPosition.h								\
	ByteSwap.h									\
	Crossfader.hpp								\
	DistanceModel.h								\
	EnhancedFile.h								\
	FractionalSample.h							\
	Interpolator.h								\
	Logging.hpp									\
	ParameterSet.h								\
	PerformanceMonitor.h						\
	PositionCursor.h							\
	SoundDelayBuffer.h							\
	SoundFormatConversions.h					\
	SoundFormatRawConversions.h					\
	SoundMixing.h								\
	ThreadLock.h								\
	UniversalTime.h								\
	VBAPanner.h									\
	VBAPanner2D.h								\
	misc.h

if ENABLE_GPL
libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_CPPFLAGS += $(APF_CFLAGS) $(SNDFILE_CFLAGS) $(FFTW_CFLAGS)
libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_LIBADD += $(APF_LIBS) $(SNDFILE_LIBS) $(FFTW_LIBS)
libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_SOURCES += Convolver.cpp
pkginclude_HEADERS += Convolver.h
endif

if ENABLE_SOFA
#-DENABLE_SOFA is passed through SOFA_CFLAGS
libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_CPPFLAGS += $(NETCDF_CFLAGS) $(NETCDFCXX_CFLAGS)
libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_LIBADD += $(NETCDF_LIBS) $(NETCDFCXX_LIBS)
libap_dsp_@APLIBS_DSP_MAJORMINOR@_la_SOURCES += SOFA.cpp
pkginclude_HEADERS += SOFA.h
endif

noinst_HEADERS =

nodist_pkginclude_HEADERS = aplibs-dsp-stdint.h

DISTCLEANFILES = aplibs-dsp-stdint.h
