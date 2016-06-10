AC_PREREQ([2.68])
AC_INIT([BBCAT_BASE],[0.1.2.2])

AS_NANO(BBCAT_BASE_CVS=no,BBCAT_BASE_CVS=yes)

AM_INIT_AUTOMAKE([1.11 silent-rules])
AM_SILENT_RULES([yes])

# Version number that will be appended to the library filename
BBCAT_BASE_MAJORMINOR=0.1
AC_SUBST(BBCAT_BASE_MAJORMINOR)

AC_CONFIG_HEADERS([config.h])

# CURRENT, REVISION, AGE
# - library source changed but interface the same -> increment REVISION
# - interfaces added/removed/changed -> increment CURRENT, set REVISION = 0
# - interfaces added -> increment AGE also
# - interfaces removed -> set AGE = 0
#    (AGE is the number of previous values of CURRENT that are compatible)
BBCAT_BASE_LIBVERSION="1:0:0"
AC_SUBST(BBCAT_BASE_LIBVERSION)

LT_PREREQ([2.2.6])
LT_INIT(disable-static win32-dll)

AC_PROG_CXX
AM_PROG_CC_C_O

AC_CONFIG_SRCDIR([src])

AC_CONFIG_MACRO_DIR([m4])
ACLOCAL_AMFLAGS="-I m4 $ACLOCAL_AMFLAGS"
AC_SUBST(ACLOCAL_AMFLAGS)

DX_DOXYGEN_FEATURE(ON)
DX_INIT_DOXYGEN(bbcat-base, doxygen.cfg)

BBCAT_GLOBAL_BASE_CFLAGS="-D_FILE_OFFSET_BITS=64 -DINSTALL_PREFIX=\\\"${prefix}\\\""
BBCAT_GLOBAL_BASE_LIBS=""

case $host_os in
  linux*)
    BBCAT_GLOBAL_BASE_CFLAGS="$BBCAT_GLOBAL_BASE_CFLAGS -D__LINUX__"
    ;;
  *)
    ;;
esac

# Check if we should disable GPL code  (./configure --disable-gpl)
AC_MSG_CHECKING(whether to include code under GPL)
AC_ARG_ENABLE(gpl, AS_HELP_STRING([--disable-gpl], [disable code under GPL]), ENABLE_GPL="no", ENABLE_GPL="yes")
if test "x${ENABLE_GPL}" = "xyes"; then
  AC_MSG_RESULT(yes)
  BBCAT_GLOBAL_BASE_CFLAGS="$BBCAT_GLOBAL_BASE_CFLAGS -DENABLE_GPL=1"
else
  AC_MSG_RESULT(no)
  BBCAT_GLOBAL_BASE_CFLAGS="$BBCAT_GLOBAL_BASE_CFLAGS -DENABLE_GPL=0"
fi

AC_MSG_CHECKING(whether to include JSON handling)
AC_ARG_ENABLE(json, AS_HELP_STRING([--disable-json], [disable JSON handling]), DISABLE_JSON_HANDLING="yes", DISABLE_JSON_HANDLING="no")
if test "x${DISABLE_JSON_HANDLING}" != "xyes"; then
  AC_MSG_RESULT(yes)

  # jsoncpp is required
  JSONCPP_VER="1.6.0"
  PKG_CHECK_MODULES(JSONCPP, jsoncpp >= $JSONCPP_VER, HAVE_JSONCPP=yes, HAVE_JSONCPP=no)
  if test "x${HAVE_JSONCPP}" != xyes ; then
    JSONCPP_OLD_VER="0.6.0"
    PKG_CHECK_MODULES(JSONCPP, jsoncpp >= $JSONCPP_OLD_VER, HAVE_JSONCPP=yes, HAVE_JSONCPP=no)
    if test "x${HAVE_JSONCPP}" = "xyes" ; then
      JSONCPP_CFLAGS="$JSONCPP_CFLAGS -DOLD_JSON_CPP"
    else
      AC_MSG_ERROR([jsoncpp >= $JSONCPP_OLD_VER is required])
    fi
  fi
  BBCAT_GLOBAL_BASE_CFLAGS="$BBCAT_GLOBAL_BASE_CFLAGS -DENABLE_JSON=1 $JSONCPP_CFLAGS"
  BBCAT_GLOBAL_BASE_LIBS="$BBCAT_GLOBAL_BASE_LIBS $JSONCPP_LIBS"
  ENABLE_JSON="yes"
else
  AC_MSG_RESULT(no)

  BBCAT_GLOBAL_BASE_CFLAGS="$BBCAT_GLOBAL_BASE_CFLAGS -DENABLE_JSON=0"
  ENABLE_JSON="no"
fi

AM_CONDITIONAL(ENABLE_JSON, test "x${ENABLE_JSON}" = "xyes")

# check for 3rd party library support
AC_MSG_CHECKING(whether to disable 3rd party library support)
AC_ARG_ENABLE(t3rdparty, AS_HELP_STRING([--disable-3rdparty], [disable 3rd party library support]), DISABLE_3RDPARTY="yes", DISABLE_3RDPARTY="no")
if test "x${DISABLE_3RDPARTY}" = "xyes"; then
  AC_MSG_RESULT(yes)
  ENABLE_3RDPARTY="no"
else
  AC_MSG_RESULT(no)
  ENABLE_3RDPARTY="yes"
fi

AC_MSG_CHECKING(3rd party library support)
if test "x${ENABLE_3RDPARTY}" = "xyes"; then
  BBCAT_GLOBAL_BASE_CFLAGS="$BBCAT_GLOBAL_BASE_CFLAGS -DENABLE_3RDPARTY=1"
  AC_MSG_RESULT(yes)
else
  BBCAT_GLOBAL_BASE_CFLAGS="$BBCAT_GLOBAL_BASE_CFLAGS -DENABLE_3RDPARTY=0"
  AC_MSG_RESULT(no)
fi

AM_CONDITIONAL(ENABLE_3RDPARTY, test "x${ENABLE_3RDPARTY}" = "xyes")

# Check if we should disable optimization  (./configure --disable-opt)
AC_MSG_CHECKING(whether to disable optimization)
AC_ARG_ENABLE(opt, AS_HELP_STRING([--disable-opt], [disable optimzation]), DISABLE_OPTIMIZATION="yes", DISABLE_OPTIMIZATION="no")
if test "x${DISABLE_OPTIMIZATION}" = "xyes"; then
  CFLAGS="$CFLAGS -O0"
  CXXFLAGS="$CXXFLAGS -O0"
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)

AC_ARG_WITH([thread],
    AS_HELP_STRING([--with-thread=IMPL],[use threading library {auto,stl,pthreads} [[default=auto]]]),
    [],
    [with_thread=auto])

AC_MSG_CHECKING([threading implementation to use])
if test "x${with_thread}" = "xauto" ; then
  case $host_os in
    *)
      with_thread=stl
      ;;
  esac
fi
AC_MSG_RESULT([$with_thread])

THREAD_IMPL=${with_thread}
AC_SUBST(THREAD_IMPL)

if test "x${with_thread}" = "xpthreads" ; then
  case "$host_os" in
    *)
      PTHREAD_CFLAGS=-DUSE_PTHREADS
      PTHREAD_LIBS=-lpthread
      ;;
  esac
else
  # stl::threads can require pthread lib anyway
  PTHREAD_LIBS=-lpthread
fi
AC_SUBST(PTHREAD_CFLAGS)
AC_SUBST(PTHREAD_LIBS)

if test ! x$osx = xtrue ; then
  dnl librt is required
  RT_LIBS=""
  AC_CHECK_LIB(rt, clock_gettime, [RT_LIBS="-lrt"])
  AC_SUBST(RT_LIBS)
fi

AM_CONDITIONAL(ENABLE_JSON, test "x${ENABLE_JSON}" = "xyes")

BBCAT_GLOBAL_BASE_CFLAGS="$BBCAT_GLOBAL_BASE_CFLAGS $PTHREAD_CFLAGS"
BBCAT_GLOBAL_BASE_LIBS="$BBCAT_GLOBAL_BASE_LIBS $RT_LIBS $PTHREAD_LIBS"

AC_SUBST(BBCAT_GLOBAL_BASE_CFLAGS)
AC_SUBST(BBCAT_GLOBAL_BASE_LIBS)

# Check if we should disable optimization  (./configure --disable-opt)
AC_MSG_CHECKING(whether to disable optimization)
AC_ARG_ENABLE(opt, AS_HELP_STRING([--disable-opt], [disable optimzation]), DISABLE_OPTIMIZATION="yes", DISABLE_OPTIMIZATION="no")
if test "x${DISABLE_OPTIMIZATION}" = "xyes"; then
  CFLAGS="$CFLAGS -O0"
  CXXFLAGS="$CXXFLAGS -O0"
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)

BBCAT_BASE_CFLAGS=""
AC_APPEND_SUPPORTED_CFLAGS(BBCAT_BASE_CFLAGS, [-Wall])
AC_APPEND_SUPPORTED_CFLAGS(BBCAT_BASE_CFLAGS, [-Werror])
AC_APPEND_SUPPORTED_CFLAGS(BBCAT_BASE_CFLAGS, [-Wextra])
AC_APPEND_SUPPORTED_CFLAGS(BBCAT_BASE_CFLAGS, [-msse2])

AC_LANG_PUSH([C++])
AC_APPEND_SUPPORTED_CFLAGS(BBCAT_BASE_CXXFLAGS, [-std=c++11])
AC_LANG_POP([C++])

AC_SUBST(BBCAT_BASE_CFLAGS)

AC_SUBST([AM_CXXFLAGS],["$AM_CXXFLAGS $BBCAT_BASE_CXXFLAGS"])

BBCAT_BASE_LDFLAGS=""
AC_APPEND_SUPPORTED_LDFLAGS(BBCAT_BASE_LDFLAGS, [-Wl,--no-undefined])
AC_SUBST(BBCAT_BASE_LDFLAGS)

BBCAT_BASE_LIBS="\$(top_builddir)/src/libbbcat-base-$BBCAT_BASE_MAJORMINOR.la"
AC_SUBST(BBCAT_BASE_LIBS)

AC_SUBST(BBCAT_BASE_PKG_DEPS)

pkgconfigdir="\$(libdir)/pkgconfig"
AC_SUBST(pkgconfigdir)

bbcatbasebindir="\$(libdir)/bbcat-base/"
AC_SUBST(bbcatbasebindir)

AC_CONFIG_FILES([
Makefile
bbcat-base-uninstalled.pc
bbcat-base.pc
src/Makefile
test/Makefile
])
AC_OUTPUT
