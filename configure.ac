AC_PREREQ([2.68])
AC_INIT([APLIBS_DSP],[0.1.1.0])

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
APLIBS_DSP_LIBVERSION="1:1:0"
AC_SUBST(APLIBS_DSP_LIBVERSION)

LT_PREREQ([2.2.6])
LT_INIT(disable-static win32-dll)

AC_PROG_CXX
AM_PROG_CC_C_O

AC_CONFIG_SRCDIR([src])

AC_CONFIG_MACRO_DIR([m4])
ACLOCAL_AMFLAGS="-I m4 $ACLOCAL_AMFLAGS"
AC_SUBST(ACLOCAL_AMFLAGS)

DX_DOXYGEN_FEATURE(ON)
DX_INIT_DOXYGEN(aplibs-dsp, doxygen.cfg)

# Check if we should enable GPL code  (./configure --enable-gpl)
AC_MSG_CHECKING(whether to include code under gpl)
AC_ARG_ENABLE(gpl, AS_HELP_STRING([--disable-gpl], [disable code under GPL]), ENABLE_GPL="no", ENABLE_GPL="yes")
AM_CONDITIONAL(ENABLE_GPL, test "x${ENABLE_GPL}" = "xyes")
if test "x${ENABLE_GPL}" = "xyes"; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

SOFA_LIBS=""
SOFA_CFLAGS=""
AC_MSG_CHECKING(whether to include support for SOFA files)
AC_ARG_ENABLE(sofa, AS_HELP_STRING([--disable-sofa], [disable SOFA support]), ENABLE_SOFA="no", ENABLE_SOFA="yes")
AM_CONDITIONAL(ENABLE_SOFA, test "x${ENABLE_SOFA}" = "xyes")
if test "x${ENABLE_SOFA}" = "xyes"; then
  SOFA_CFLAGS="-DENABLE_SOFA=1"
  AC_MSG_RESULT(yes)
else
  SOFA_CFLAGS="-DENABLE_SOFA=0"
  AC_MSG_RESULT(no)
fi
AC_SUBST(SOFA_CFLAGS)
AC_SUBST(SOFA_LIBS)

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

if test ! x$osx = xtrue ; then
  dnl librt is required
  RT_LIBS=""
  AC_CHECK_LIB(rt, clock_gettime, [RT_LIBS="-lrt"])
  AC_SUBST(RT_LIBS)
fi

if test "x${ENABLE_SOFA}" = "xyes"; then
  NETCDF_VER="4.1.3"
  PKG_CHECK_MODULES([NETCDF], [netcdf >= $NETCDF_VER], HAVE_NETCDF=yes, HAVE_NETCDF=no)

  if test "x${HAVE_NETCDF}" != "xyes" ; then
    AC_MSG_ERROR([netcdf >= $NETCDF_VER is required])
  fi

  NETCDFCXX_VER="4.2.1"
  PKG_CHECK_MODULES([NETCDFCXX], [netcdf-cxx4 >= $NETCDFCXX_VER], HAVE_NETCDFCXX=yes, HAVE_NETCDFCXX=no)

  if test "x${HAVE_NETCDFCXX}" != "xyes" ; then
    AC_MSG_ERROR([netcdf-cxx4 >= $NETCDFCXX_VER is required])
  fi
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AX_CREATE_STDINT_H([src/aplibs-dsp-stdint.h])

AC_APPEND_SUPPORTED_CFLAGS(APLIBS_DSP_CFLAGS, [-Wall])
#AC_APPEND_SUPPORTED_CFLAGS(APLIBS_DSP_CFLAGS, [-Werror])
AC_APPEND_SUPPORTED_CFLAGS(APLIBS_DSP_CFLAGS, [-Wextra])
AC_APPEND_SUPPORTED_CFLAGS(APLIBS_DSP_CFLAGS, [-msse2])

AC_LANG_PUSH([C++])
AC_APPEND_SUPPORTED_CFLAGS(APLIBS_DSP_CFLAGS, [-std=c++11])
AC_LANG_POP([C++])

APLIBS_DSP_CFLAGS="$APLIBS_DSP_CFLAGS -I\$(top_srcdir)"
AC_SUBST(APLIBS_DSP_CFLAGS)

APLIBS_DSP_LDFLAGS=""
AC_APPEND_SUPPORTED_LDFLAGS(APLIBS_DSP_LDFLAGS, [-Wl,--no-undefined])
AC_SUBST(APLIBS_DSP_LDFLAGS)

APLIBS_DSP_LIBS="\$(top_builddir)/src/libap-dsp-$APLIBS_DSP_MAJORMINOR.la"
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
src/Makefile
])
AC_OUTPUT
