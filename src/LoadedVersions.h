#ifndef __LOADED_VERSIONS__
#define __LOADED_VERSIONS__

#include <vector>
#include <string>

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Singleton class to hold a list of the loaded versions of libraries and applications
 *
 */
/*--------------------------------------------------------------------------------*/
class LoadedVersions
{
public:
  /*--------------------------------------------------------------------------------*/
  /** Access through singleton mechanism only
   */
  /*--------------------------------------------------------------------------------*/
  static LoadedVersions& Get();

  /*--------------------------------------------------------------------------------*/
  /** Register a library/application and its version string
   *
   * @note the return is the index of the entry which is a dummy return to allow the function to be called at load
   */
  /*--------------------------------------------------------------------------------*/
  uint_t Register(const char *name, const char *version);

  /*--------------------------------------------------------------------------------*/
  /** Return version string of loaded library/application of given name or empty string if not found
   */
  /*--------------------------------------------------------------------------------*/
  std::string GetVersion(const char *name);

  /*--------------------------------------------------------------------------------*/
  /** Return a textual list of libraries and applications and their versions
   */
  /*--------------------------------------------------------------------------------*/
  std::string GetVersionsList() const;

  /*--------------------------------------------------------------------------------*/
  /** Return number of entries in the versions list
   */
  /*--------------------------------------------------------------------------------*/
  uint_t GetVersionListCount() const;

  /*--------------------------------------------------------------------------------*/
  /** Return number of entries in the versions list
   */
  /*--------------------------------------------------------------------------------*/
  std::string GetVersionIndex(uint_t n) const;
  
protected:
  LoadedVersions();
  ~LoadedVersions();

  typedef struct
  {
    std::string name;
    std::string version;
  } VERSION;

protected:
  std::vector<VERSION> versions;
};

BBC_AUDIOTOOLBOX_END

#endif
