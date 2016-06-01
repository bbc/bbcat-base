LDADD = $(BBCAT_BASE_LIBS)								\
        $(BBCAT_BASE_GLOBAL_LIBS)						\
        ../src/libbbcat-base-@BBCAT_BASE_MAJORMINOR@.la

AM_CPPFLAGS = $(BBCAT_BASE_CFLAGS)				\
			  $(BBCAT_GLOBAL_BASE_CFLAGS)		\
			  -I../bbcat-common/include			\
			  -I../src

AM_CXXFLAGS = $(AM_CPPFLAGS)					\
			  -std=c++11

check_PROGRAMS =
TESTS =

tests_SOURCES = testbase.cpp stringfromtests.cpp jsontests.cpp
check_PROGRAMS += tests
TESTS += tests
