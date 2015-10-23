# Arbitrarily set minimum version.
# This specific version is used to avoid complaints about the 
# cygwin platform not defining WIN32 anymore.
CMAKE_MINIMUM_REQUIRED (VERSION 2.8.12.2)

# The name of our top-level project.
PROJECT( bbcat-base C CXX )

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_RELEASE 1)
set(PROJECT_VERSION_BUILD 1)

#include all the parts that are consistent across all libraries
include(CMakeLists-bbcat.txt)

################################################################################
# dependant libraries
if(MSVC)
	set(THREADS_USE_PTHREADS_WIN32 true)

	# Default expected pthreads installation dir is c:/local
	if(NOT DEFINED PTHREADS_INSTALL_DIR)
		set(PTHREADS_INSTALL_DIR "c:/local/pthreads/Pre-built.2")
	endif()

	set(CMAKE_INCLUDE_PATH "${CMAKE_INCLUDE_PATH};${PTHREADS_INSTALL_DIR}/include")
endif()
find_package(Threads REQUIRED)
include_directories(${THREADS_PTHREADS_INCLUDE_DIR})

set(GLOBAL_INCLUDE_DIRS
	${THREADS_PTHREADS_INCLUDE_DIR}
	${INSTALL_INCLUDE_DIR})

set(GLOBAL_FLAGS
	"-DENABLE_JSON=0")
	
ADD_DEFINITIONS(${GLOBAL_FLAGS})

set(_extra_libs
	${_extra_libs}
	${CMAKE_THREAD_LIBS_INIT})

################################################################################
# sources are contained in the src/ directory
ADD_SUBDIRECTORY( src )

################################################################################
# install files for 'share'
install(DIRECTORY "share/"
		DESTINATION "${INSTALL_SHARE_DIR}"
		PATTERN "share/*")
