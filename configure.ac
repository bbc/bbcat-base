AC_PREREQ([2.68])
AC_INIT([APLIBS_DSP],[0.1.0.0])

AS_NANO(APLIBS_DSP_CVS=no,APLIBS_DSP_CVS=yes)

AM_INIT_AUTOMAKE([1.11 silent-rules])
AM_SILENT_RULES([yes])

# Version number that will be appended to the library filename
APLIBS_DSP_MAJORMINOR=0.1
AC_SUBST(APLIBS_DSP_MAJORMINOR)

AC_CONFIG_HEADERS([config.h])

# CURRENT, REVISION, AGE
# - library source changed but interface the same -> increment REVISION
# - interfaces added/removed/changed -> increment CURRENT, set REVISION = 0
# - interfaces added -> increment AGE also
# - interfaces removed -> set AGE = 0
#    (AGE is the number of previous values of CURRENT that are compatible)
APLIBS_DSP_LIBVERSION="1:0:0"
AC_SUBST(APLIBS_DSP_LIBVERSION)

LT_PREREQ([2.2.6])
LT_INIT(disable-static win32-dll)

AC_PROG_CXX
AM_PROG_CC_C_O

AC_CONFIG_SRCDIR([aplibs-dsp])

AC_CONFIG_MACRO_DIR([m4])
ACLOCAL_AMFLAGS="-I m4 $ACLOCAL_AMFLAGS"
AC_SUBST(ACLOCAL_AMFLAGS)

DX_DOXYGEN_FEATURE(ON)
DX_INIT_DOXYGEN(aplibs-dsp, doxygen.cfg)

# Check if we should enable debug support  (./configure --enable-debug)
AC_MSG_CHECKING(whether to include code under gpl)
gpl_default="yes"
AC_ARG_ENABLE(gpl, [  --enable-gpl=[no/yes] include code under gpl [default=$gpl_default]],, ENABLE_GPL=$gpl_default)
AM_CONDITIONAL(ENABLE_GPL, test "x$ENABLE_GPL" = "xyes")

# Check if we should enable debug support  (./configure --enable-debug)
AC_MSG_CHECKING(whether to enable debugging)
debug_default="no"
AC_ARG_ENABLE(debug, [  --enable-debug=[no/yes] turn on debugging
					   [default=$debug_default]],, enable_debug=$debug_default)
if test "x$enable_debug" = "xyes"; then
		 APLIBS_DSP_CFLAGS="$APLIBS_DSP_CFLAGS -g -O0"
#		 APLIBS_DSP_CFLAGS="$APLIBS_DSP_CFLAGS -g -O0 -DDEBUG"
		 CXXFLAGS="$CXXFLAGS -O0"
AC_MSG_RESULT(yes)
else
AC_MSG_RESULT(no)
fi

LIBBOOST_VER="1.49.0"
AX_BOOST_BASE($LIBBOOST_VER, HAVE_BOOST=yes, HAVE_BOOST=no)
if test "x${HAVE_BOOST}" != xyes ; then
  AC_MSG_ERROR([Boost library is required])
fi

LIBSNDFILE_VER="1.0.25"
PKG_CHECK_MODULES([SNDFILE], [sndfile >= LIBSNDFILE_VER], HAVE_SNDFILE=yes, HAVE_SNDFILE=no)
if test "x${HAVE_SNDFILE}" != xyes ; then
  AC_MSG_ERROR([sndfile library is required])
fi

AC_ARG_WITH([thread],
    AS_HELP_STRING([--with-thread=IMPL],[use threading library {auto,pthread,gthread,win32,none} [[default=auto]]]),
    [],
    [with_thread=auto])

AC_MSG_CHECKING([threading implementation to use])
if test "x${with_thread}" = "xauto" ; then
  case $host_os in
    mingw* | pw32*)
      with_thread=win32
      ;;
    *)
      with_thread=pthread
      ;;
  esac
fi
AC_MSG_RESULT([$with_thread])

THREAD_IMPL=${with_thread}
AC_SUBST(THREAD_IMPL)

if test "x${with_thread}" = "xpthread" ; then
  case "$host_os" in
    *)
      PTHREAD_CFLAGS=
      PTHREAD_LIBS=-lpthread
      ;;
  esac
  AC_SUBST(PTHREAD_CFLAGS)
  AC_SUBST(PTHREAD_LIBS)
fi

if test "x${with_thread}" = "xgthread" ; then
  PKG_CHECK_MODULES(GTHREAD, gthread-2.0, HAVE_GTHREAD=yes, HAVE_GTHREAD=no)
  AC_SUBST(GTHREAD_LIBS)
  AC_SUBST(GTHREAD_CFLAGS)
  COG_PKG_DEPS="$COG_PKG_DEPS gthread-2.0"
fi

if test "x$ENABLE_GPL" = "xyes"; then
  # Checking for FFTW3 (single-precision)
  FFTW_VER="3.0.0"
  PKG_CHECK_MODULES([FFTW], [fftw3f >= FFTW_VER], HAVE_FFTW=yes, HAVE_FFTW=no)
  if test "x${HAVE_FFTW}" != xyes ; then
    AC_MSG_ERROR([fftw3f >= $FFTW_VER is required])
  fi
  
  dnl Check for apf
  APF_VER="0.2.0"
  PKG_CHECK_MODULES([APF], [apf-0.2 >= 0.2.0], HAVE_APF=yes, HAVE_APF=no)
  if test "x${HAVE_APF}" != xyes ; then
    AC_MSG_ERROR([apf >= $APF_VER is required])
  fi
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AX_CREATE_STDINT_H([aplibs-dsp/aplibs-dsp-stdint.h])

AC_APPEND_SUPPORTED_CFLAGS(APLIBS_DSP_CFLAGS, [-Wall])
AC_APPEND_SUPPORTED_CFLAGS(APLIBS_DSP_CFLAGS, [-Werror])
AC_APPEND_SUPPORTED_CFLAGS(APLIBS_DSP_CFLAGS, [-Wextra])

AC_LANG_PUSH([C++])
AC_APPEND_SUPPORTED_CFLAGS(APLIBS_DSP_CFLAGS, [-std=c++11])
AC_LANG_POP([C++])

APLIBS_DSP_CFLAGS="$APLIBS_DSP_CFLAGS -I\$(top_srcdir)"
AC_SUBST(APLIBS_DSP_CFLAGS)

APLIBS_DSP_LDFLAGS=""
AC_APPEND_SUPPORTED_LDFLAGS(APLIBS_DSP_LDFLAGS, [-Wl,--no-undefined])
AC_SUBST(APLIBS_DSP_LDFLAGS)

APLIBS_DSP_LIBS="\$(top_builddir)/aplibs-dsp/libap-dsp-$APLIBS_DSP_MAJORMINOR.la"
AC_SUBST(APLIBS_DSP_LIBS)

AC_SUBST(APLIBS_DSP_PKG_DEPS)

pkgconfigdir="\$(libdir)/pkgconfig"
AC_SUBST(pkgconfigdir)

aplibsdspbindir="\$(libdir)/aplibs-dsp/"
AC_SUBST(aplibsdspbindir)

AC_CONFIG_FILES([
Makefile
aplibs-dsp-uninstalled.pc
aplibs-dsp.pc
aplibs-dsp/Makefile
])
AC_OUTPUT
