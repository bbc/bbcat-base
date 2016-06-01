# Arbitrarily set minimum version.
# This specific version is used to avoid complaints about the
# cygwin platform not defining WIN32 anymore.
CMAKE_MINIMUM_REQUIRED (VERSION 2.8.12.2)

# The name of our top-level project.
PROJECT( bbcat-base C CXX )

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_RELEASE 2)
set(PROJECT_VERSION_BUILD 2)

set(PROJECT_DESCRIPTION "Base library for BBC Audio Toolbox")

set(BBCAT_COMMON_DIR ${PROJECT_SOURCE_DIR}/bbcat-common)

#include all the parts that are consistent across all projects
include(${BBCAT_COMMON_DIR}/CMakeProject.txt)

set(GLOBAL_FLAGS
	${GLOBAL_FLAGS}
	"-D_FILE_OFFSET_BITS=64"
	-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

################################################################################
# dependant libraries
OPTION(USE_PTHREADS "Use pthreads for thread support" OFF)

if(USE_PTHREADS)
	if(MSVC)
		set(THREADS_USE_PTHREADS_WIN32 true)

		# Default expected pthreads installation dir is c:/local
		if(NOT DEFINED PTHREADS_INSTALL_DIR)
			set(PTHREADS_INSTALL_DIR "${WIN32_LIBRARIES_LOCATION}/pthreads/Pre-built.2")
		endif()

		set(CMAKE_INCLUDE_PATH "${CMAKE_INCLUDE_PATH};${PTHREADS_INSTALL_DIR}/include")
	endif()

	find_package(Threads REQUIRED)

	set(GLOBAL_FLAGS
		"-DUSE_PTHREADS"
		${GLOBAL_FLAGS})

	# set include directories for storing in .cmake file
	set(GLOBAL_INCLUDE_DIRS
		${THREADS_PTHREADS_INCLUDE_DIR}
		${GLOBAL_INCLUDE_DIRS})

	set(GLOBAL_EXTRA_LIBS
		${GLOBAL_EXTRA_LIBS}
		${CMAKE_THREAD_LIBS_INIT})
endif()

bbcat_find_library(LIBRARY jsoncpp HEADER json/json.h DEFINE ENABLE_JSON)
if(DEFINED jsoncpp_VERSION)
	if(${jsoncpp_VERSION} VERSION_LESS "1.6.5")
		message("Warning: old jsoncpp version ${jsoncpp_VERSION}")
		set(GLOBAL_FLAGS
			${GLOBAL_FLAGS}
			"-DOLD_JSON_CPP")
	endif()
endif()
	
option(ENABLE_GPL "Enable GPL support" ON)
if(ENABLE_GPL)
	message("GPL support enabled")
	set(GLOBAL_FLAGS
		${GLOBAL_FLAGS}
		"-DENABLE_GPL=1")
else()
	message("GPL support *disabled*")
	set(GLOBAL_FLAGS
		${GLOBAL_FLAGS}
		"-DENABLE_GPL=0")
endif()

option(ENABLE_3RDPARTY "Enable 3rd party library support" ON)
if(ENABLE_3RDPARTY)
	message("3rd party library support enabled")
	set(GLOBAL_FLAGS
		${GLOBAL_FLAGS}
		"-DENABLE_3RDPARTY=1")
else()
	message("3rd party library support *disabled*")
	set(GLOBAL_FLAGS
		${GLOBAL_FLAGS}
		"-DENABLE_3RDPARTY=0")
endif()

# set flags for compiling
add_definitions(${GLOBAL_FLAGS})

# set include directories for compiling
include_directories(${GLOBAL_INCLUDE_DIRS})

# set library directories for linking
link_directories(${GLOBAL_LIB_DIRS})

################################################################################
# sources are contained in the src/ directory
ADD_SUBDIRECTORY( src )

ADD_SUBDIRECTORY( test )

################################################################################
# install files for 'share'
install(DIRECTORY "share/"
		DESTINATION "${INSTALL_SHARE_DIR}"
		PATTERN "share/*")

################################################################################
# install common files to cmake directory
install(DIRECTORY "bbcat-common/"
		DESTINATION "${CMAKE_INSTALL_PREFIX}/share/bbcat-common"
		PATTERN "bbcat-common/*")

# generate package config file
bbcat_generate_pkg_config()
