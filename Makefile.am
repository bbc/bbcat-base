
AUTOMAKE_OPTIONS = foreign

include doxygen.am

SUBDIRS = src test

base_DATA = share/licences.txt

basedir = $(prefix)/share/bbcat-base

catch_DATA = bbcat-common/include/catch/*
catchdir = $(prefix)/share/bbcat-common/include/catch

common_DATA = bbcat-common/*.sh
commondir = $(prefix)/share/bbcat-common

EXTRA_DIST = README COPYING autogen.sh $(base_DATA) $(catch_DATA) $(common_DATA)

pkgconfig_DATA = bbcat-base-$(BBCAT_BASE_MAJORMINOR).pc

bbcat-base-$(BBCAT_BASE_MAJORMINOR).pc: bbcat-base.pc
	cp bbcat-base.pc bbcat-base-$(BBCAT_BASE_MAJORMINOR).pc

bbcat-base-$(BBCAT_BASE_MAJORMINOR)-uninstalled.pc: bbcat-base-uninstalled.pc
	cp bbcat-base-uninstalled.pc bbcat-base-$(BBCAT_BASE_MAJORMINOR)-uninstalled.pc

BUILT_SOURCES = bbcat-base-$(BBCAT_BASE_MAJORMINOR)-uninstalled.pc gitbranch.txt

CLEANFILES = bbcat-base-$(BBCAT_BASE_MAJORMINOR).pc bbcat-base-$(BBCAT_BASE_MAJORMINOR)-uninstalled.pc src/register.cpp gitbranch.txt .branch

ACLOCAL_FLAGS = -I m4

.PHONY: gitbranch.txt

gitbranch.txt:
	@-git rev-parse --abbrev-ref HEAD >.branch && (diff gitbranch.txt .branch >/dev/null || cp .branch gitbranch.txt)
	@if [ ! -e "gitbranch.txt" ] ; then touch "gitbranch.txt" ; fi
	@echo "Branch: `cat gitbranch.txt`"
