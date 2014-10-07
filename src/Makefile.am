pkgincludedir = $(includedir)/bbcat-base-@BBCAT_BASE_MAJORMINOR@/bbcat-base

lib_LTLIBRARIES = libap-base-@BBCAT_BASE_MAJORMINOR@.la

libap_base_@BBCAT_BASE_MAJORMINOR@_la_LDFLAGS =	\
	-no-undefined								\
	-version-info $(BBCAT_BASE_LIBVERSION)		\
    $(BBCAT_BASE_LDFLAGS)

libap_base_@BBCAT_BASE_MAJORMINOR@_la_LIBADD =	\
	$(BBCAT_GLOBAL_LIBS)						\
	$(PTHREAD_LIBS)								\
	$(RT_LIBS)

libap_base_@BBCAT_BASE_MAJORMINOR@_la_CPPFLAGS =	\
	$(BBCAT_GLOBAL_CFLAGS)							\
	$(BBCAT_BASE_CFLAGS)							\
	$(PTHREAD_CFLAGS)

libap_base_@BBCAT_BASE_MAJORMINOR@_la_SOURCES =	\
	3DPosition.cpp								\
	ByteSwap.cpp								\
	DistanceModel.cpp							\
	EnhancedFile.cpp							\
	misc.cpp									\
	ParameterSet.cpp							\
	PerformanceMonitor.cpp						\
	ThreadLock.cpp								\
	UDPSocket.cpp

pkginclude_HEADERS =							\
	3DPosition.h								\
	ByteSwap.h									\
	DistanceModel.h								\
	EnhancedFile.h								\
	ParameterSet.h								\
	PerformanceMonitor.h						\
	PositionCursor.h							\
	ThreadLock.h								\
	UniversalTime.h								\
	UDPSocket.h									\
	misc.h

noinst_HEADERS =
