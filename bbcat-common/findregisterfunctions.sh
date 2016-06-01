#!/bin/sh
LIBRARY="$1"
PACKAGE="`echo "$LIBRARY" | sed -E "s/-/_/g"`"
VERSION="$2"
SOURCES="$3"
DEPENDENCIES="$4"

REGISTERFUNCTIONPATTERN="^void bbcat_register_[A-Za-z0-9_]+\(\)"

REGISTRATIONFUNCTIONS="`grep -h -o -E "$REGISTERFUNCTIONPATTERN" $SOURCES`"

echo "/* Auto-generated: DO NOT EDIT! */"
if [ "$LIBRARY" = "bbcat-base" ] ; then
	echo "#include \"LoadedVersions.h\""
else
	echo "#include <bbcat-base/LoadedVersions.h>"
fi
echo "BBC_AUDIOTOOLBOX_START"
if [ -n "$DEPENDENCIES" ] ; then
	echo "// list of libraries this library is dependant on"
	for DEP in $DEPENDENCIES ; do
		echo "extern bool bbcat_register_$DEP();"
	done
fi
if [ -n "$REGISTRATIONFUNCTIONS" ] ; then
	echo "// list of this library's component registration functions"
	grep -h -o -E "$REGISTERFUNCTIONPATTERN" $SOURCES | sed -E "s/^(.+)$/extern \1;/" | sort
fi
echo "// registration function"
echo "bool bbcat_register_$PACKAGE()"
echo "{"
echo "  static bool registered = false;"
echo "  // prevent registration functions being called more than once"
echo "  if (!registered)"
echo "  {"
echo "    registered = true;"
if [ -n "$DEPENDENCIES" ] ; then
	echo "    // register other libraries"
	for DEP in $DEPENDENCIES ; do
		echo "    bbcat_register_$DEP();"
	done
	fi
echo "    // register this library's version number"
echo "    LoadedVersions::Get().Register(\"$LIBRARY\", \"$VERSION\");"
if [ -n "$REGISTRATIONFUNCTIONS" ] ; then
	echo "    // register this library's components"
	grep -h -o -E "$REGISTERFUNCTIONPATTERN" $SOURCES | sed -E "s/^void (.+)$/    \1;/" | sort
fi
echo "  }"
echo "  return registered;"
echo "}"
echo "// automatically call registration functions"
echo "volatile const bool ${PACKAGE}_registered = bbcat_register_$PACKAGE();"
echo "BBC_AUDIOTOOLBOX_END"
