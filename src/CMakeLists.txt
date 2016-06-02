#sources
set(_sources
	3DPosition.cpp
	BackgroundFile.cpp
	ByteSwap.cpp
	DistanceModel.cpp
	EnhancedFile.cpp
	LoadedVersions.cpp
	misc.cpp
	NamedParameter.cpp
	ObjectRegistry.cpp
	ParameterSet.cpp
	PerformanceMonitor.cpp
	SelfRegisteringParametricObject.cpp
	SystemParameters.cpp
	Thread.cpp
	ThreadLock.cpp
	UDPSocket.cpp
)

# public headers
set(_headers
	3DPosition.h
	BackgroundFile.h
	ByteSwap.h
	CallbackHook.h
	DistanceModel.h
	EnhancedFile.h
	LoadedVersions.h
	LockFreeBuffer.h
	NamedParameter.h
	ObjectRegistry.h
	OSCompiler.h
	ParameterSet.h
	PerformanceMonitor.h
	RefCount.h
	SelfRegisteringParametricObject.h
	SystemParameters.h
	Thread.h
	ThreadLock.h
	UniversalTime.h
	UDPSocket.h
	misc.h
	json.h
	register.h)

if(ENABLE_JSON)
	set(_sources
		${_sources}
		json.cpp)
endif()

# os specific
if(WIN32)
	set(_headers ${_headers}
		WindowsNet.h
		Windows_uSleep.h)

	set(_sources ${_sources}
		Windows_uSleep.cpp)

	set(GLOBAL_EXTRA_LIBS
		${GLOBAL_EXTRA_LIBS}
		"ws2_32.lib")
endif(WIN32)

# auto-generate register.cpp
bbcat_generate_register_cpp()

# include all the parts that are consistent across all projects
include(${BBCAT_COMMON_DIR}/CMakeLibrary.txt)
