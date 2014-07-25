
AUTOMAKE_OPTIONS = foreign

include doxygen.am

SUBDIRS = src

EXTRA_DIST = README COPYING autogen.sh

pkgconfig_DATA = aplibs-dsp-$(APLIBS_DSP_MAJORMINOR).pc

aplibs-dsp-$(APLIBS_DSP_MAJORMINOR).pc: aplibs-dsp.pc
	cp aplibs-dsp.pc aplibs-dsp-$(APLIBS_DSP_MAJORMINOR).pc

aplibs-dsp-$(APLIBS_DSP_MAJORMINOR)-uninstalled.pc: aplibs-dsp-uninstalled.pc
	cp aplibs-dsp-uninstalled.pc aplibs-dsp-$(APLIBS_DSP_MAJORMINOR)-uninstalled.pc

BUILT_SOURCES = aplibs-dsp-$(APLIBS_DSP_MAJORMINOR)-uninstalled.pc

CLEANFILES = aplibs-dsp-$(APLIBS_DSP_MAJORMINOR).pc aplibs-dsp-$(APLIBS_DSP_MAJORMINOR)-uninstalled.pc

ACLOCAL_FLAGS = -I m4
