/* Auto-generated: DO NOT EDIT! */
#include "LoadedVersions.h"
BBC_AUDIOTOOLBOX_START
// list of libraries this library is dependant on

// list of this library's component registration functions

// registration function
bool bbcat_register_bbcat_base()
{
  static bool registered = false;
  // prevent registration functions being called more than once
  if (!registered)
  {
    registered = true;
    // register other libraries

    // register this library's version number
    LoadedVersions::Get().Register("bbcat-base", "0.1.2.2-master");
    // register this library's components

  }
  return registered;
}
// automatically call registration functions
volatile const bool bbcat_base_registered = bbcat_register_bbcat_base();
BBC_AUDIOTOOLBOX_END
