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
	Thread.cpp
	ThreadLock.cpp
	UDPSocket.cpp
	${CMAKE_CURRENT_BINARY_DIR}/version.cpp
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
	${CMAKE_CURRENT_BINARY_DIR}/version.h
)

# os specific
if(WIN32)
	set(_sources ${_sources} 
		Windows_uSleep.cpp)
	set(_headers ${_headers}
		WindowsNet.h
		Windows_uSleep.h)
		
	set(_extra_libs
		${_extra_libs}
		"ws2_32.lib")
endif(WIN32)

#include all the parts that are consistent across all libraries
include(CMakeLists-src.txt)
