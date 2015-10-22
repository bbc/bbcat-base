pkgincludedir = $(includedir)/bbcat-base-@BBCAT_BASE_MAJORMINOR@/bbcat-base

lib_LTLIBRARIES = libbbcat-base-@BBCAT_BASE_MAJORMINOR@.la

libbbcat_base_@BBCAT_BASE_MAJORMINOR@_la_LDFLAGS =	\
	-no-undefined									\
	-version-info $(BBCAT_BASE_LIBVERSION)			\
    $(BBCAT_BASE_LDFLAGS)

libbbcat_base_@BBCAT_BASE_MAJORMINOR@_la_LIBADD =	\
	$(BBCAT_GLOBAL_LIBS)							\
	$(PTHREAD_LIBS)									\
	$(RT_LIBS)

libbbcat_base_@BBCAT_BASE_MAJORMINOR@_la_CPPFLAGS =	\
	$(BBCAT_GLOBAL_CFLAGS)							\
	$(BBCAT_BASE_CFLAGS)							\
	$(PTHREAD_CFLAGS)

libbbcat_base_@BBCAT_BASE_MAJORMINOR@_la_SOURCES =	\
	3DPosition.cpp									\
	BackgroundFile.cpp								\
	ByteSwap.cpp									\
	DistanceModel.cpp								\
	EnhancedFile.cpp								\
	LoadedVersions.cpp								\
	misc.cpp										\
	NamedParameter.cpp								\
	ObjectRegistry.cpp								\
	ParameterSet.cpp								\
	PerformanceMonitor.cpp							\
	SelfRegisteringParametricObject.cpp				\
	Thread.cpp										\
	ThreadLock.cpp									\
	UDPSocket.cpp									\
	version.cpp

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
	Thread.h									\
	ThreadLock.h								\
	UniversalTime.h								\
	UDPSocket.h									\
	misc.h										\
	version.h

noinst_HEADERS =

CLEANFILES = version.h version.cpp

version.h: Makefile ../config.h ../gitbranch.txt
	echo "{\"@PACKAGE@\", \"@VERSION@-`cat ../gitbranch.txt`\"}," >version.h

version.cpp: Makefile ../config.h ../gitbranch.txt
	echo "/* Auto-generated: DO NOT EDIT! */" >version.cpp
	echo "#include \"LoadedVersions.h\"" >>version.cpp
	echo "static volatile const uint_t library_id = bbcat::LoadedVersions::Get().Register(\"@PACKAGE@\", \"@VERSION@-`cat ../gitbranch.txt`\");" >>version.cpp
