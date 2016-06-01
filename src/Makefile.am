pkgincludedir = $(includedir)/bbcat-base-@BBCAT_BASE_MAJORMINOR@/bbcat-base

lib_LTLIBRARIES = libbbcat-base-@BBCAT_BASE_MAJORMINOR@.la

libbbcat_base_@BBCAT_BASE_MAJORMINOR@_la_LDFLAGS =	\
	-no-undefined									\
	-version-info $(BBCAT_BASE_LIBVERSION)			\
    $(BBCAT_BASE_LDFLAGS)

libbbcat_base_@BBCAT_BASE_MAJORMINOR@_la_LIBADD =	\
	$(BBCAT_GLOBAL_BASE_LIBS)						\
	$(RT_LIBS)

libbbcat_base_@BBCAT_BASE_MAJORMINOR@_la_CPPFLAGS =	\
	$(BBCAT_BASE_CFLAGS)							\
	$(BBCAT_GLOBAL_BASE_CFLAGS)

libbbcat_base_sources =							\
	3DPosition.cpp								\
	BackgroundFile.cpp							\
	ByteSwap.cpp								\
	DistanceModel.cpp							\
	EnhancedFile.cpp							\
	LoadedVersions.cpp							\
	misc.cpp									\
	NamedParameter.cpp							\
	ObjectRegistry.cpp							\
	ParameterSet.cpp							\
	PerformanceMonitor.cpp						\
	SelfRegisteringParametricObject.cpp			\
	SystemParameters.cpp						\
	Thread.cpp									\
	ThreadLock.cpp								\
	UDPSocket.cpp

pkginclude_HEADERS =							\
	3DPosition.h								\
	BackgroundFile.h							\
	ByteSwap.h									\
	CallbackHook.h								\
	DistanceModel.h								\
	EnhancedFile.h								\
	LoadedVersions.h							\
	LockFreeBuffer.h							\
	NamedParameter.h							\
	ObjectRegistry.h							\
	OSCompiler.h								\
	ParameterSet.h								\
	PerformanceMonitor.h						\
	RefCount.h									\
	SelfRegisteringParametricObject.h			\
	SystemParameters.h							\
	Thread.h									\
	ThreadLock.h								\
	UniversalTime.h								\
	UDPSocket.h									\
	misc.h										\
	json.h

if ENABLE_JSON
libbbcat_base_sources += json.cpp
endif

#CLEANFILES = register.cpp

# register.cpp is included in repo but will be updated by the script below 
register.cpp: Makefile ../config.h ../gitbranch.txt $(libbbcat_base_sources)
	sh ../bbcat-common/findregisterfunctions.sh "bbcat-base" "@VERSION@-`cat ../gitbranch.txt`" "$(libbbcat_base_sources)" "" >$@

libbbcat_base_@BBCAT_BASE_MAJORMINOR@_la_SOURCES = $(libbbcat_base_sources) register.cpp
