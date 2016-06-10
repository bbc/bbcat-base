
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BBCDEBUG_LEVEL 1
#include "SystemParameters.h"
#include "EnhancedFile.h"

BBC_AUDIOTOOLBOX_START

const std::string SystemParameters::installdirkey = "installdir";
const std::string SystemParameters::sharedirkey   = "sharedir";
const std::string SystemParameters::homedirkey    = "homedir";

SystemParameters::SystemParameters()
{
  ThreadLock lock(tlock);
  static bool init = false;

  if (!init)
  {
    const char *p;
    std::string homedir;
    
    init = true;
    
    // attempt to find user's home directory
    if ((p = getenv("HOME")) != NULL) homedir = p;
#ifdef TARGET_OS_WINDOWS
    // Windows uses HOMEDRIVE and HOMEPATH
    else if ((p = getenv("HOMEDRIVE")) != NULL)
    {
      // replace all backspaces (yes, Windows filesystem commands are perfectly happy with forward slashes!)
      homedir = SearchAndReplace(p, "\\", "/");
      
      // concatenate HOMEPATH, replacing all backspaces
      if ((p = getenv("HOMEPATH")) != NULL) homedir = EnhancedFile::catpath(homedir, SearchAndReplace(p, "\\", "/"));
    }
#endif

    // set home path
    Set(homedirkey, homedir);

    // now that home directory has been set, attempt to read other values from various files
#ifdef TARGET_OS_UNIXBSD
    ReadFromFile("/etc/bbcat.conf");    // system wide version for Unix
#else
    if ((p = getenv("ALLUSERSPROFILE")) != NULL) ReadFromFile(EnhancedFile::catpath(SearchAndReplace(p, "\\", "/"), "bbcat.conf")); // all users' version for Windows
    else                                         BBCERROR("No location to read system-wide bbcat.conf");
#endif

    // read user's home directory version
    ReadFromFile(SubstitutePathList("{" + homedirkey + "}/bbcat.conf"));

    // finally, read version from current directory
    ReadFromFile("bbcat.conf");

    // allow BBCATINSTALL env var to override defaults or that read from the conf file
    if ((p = getenv("BBCATINSTALLDIR")) != NULL) Set(installdirkey, p);
    else if (!Exists(installdirkey))
    {
#ifdef INSTALL_PREFIX
      Set(installdirkey, INSTALL_PREFIX);
#elif TARGET_OS_WINDOWS
      Set(installdirkey, "c:/local");
#else  
      Set(installdirkey, "/usr/local");
#endif
    }
  
    // set share path (relative to installdir)
    if ((p = getenv("BBCATSHAREDIR")) != NULL) Set(sharedirkey, p);
    else if (!Exists(sharedirkey))
    {
#ifdef TARGET_OS_WINDOWS
      // add guess of where share folder could be
      Set(sharedirkey, "../share;{" + installdirkey + "}/share");
#else
      Set(sharedirkey, "{" + installdirkey + "}/share");
#endif
    }
    
    BBCDEBUG2(("SystemParameters: {%s}", parameters.ToString().c_str()));
  }
}

/*--------------------------------------------------------------------------------*/
/** Get single instance of system parameters object
 */
/*--------------------------------------------------------------------------------*/
SystemParameters& SystemParameters::Get()
{
  static SystemParameters parameters;
  return parameters;
}

/*--------------------------------------------------------------------------------*/
/** Read values from .conf style file
 */
/*--------------------------------------------------------------------------------*/
bool SystemParameters::ReadFromFile(const std::string& filename)
{
  EnhancedFile file;
  bool success = false;

  if (EnhancedFile::exists(filename.c_str()) && file.fopen(filename.c_str()))
  {
    static char buffer[1024];
    int l;
      
    while ((l = file.readline(buffer, sizeof(buffer) - 1)) >= 0)
    {
      std::string line = buffer;
      size_t p;

      // clear any comments
      if ((p = line.find(";")) < std::string::npos) line = line.substr(0, p);

      // use <var>=<val> notation (with whitespace stripped)
      if ((p = line.find("=")) < std::string::npos)
      {
        size_t p1 = 0, p2 = p, p3 = p + 1, p4 = line.length();

        // strip whitespace from front and back of var
        while ((p2 > p1) && ((line[p2 - 1] == ' ') || (line[p2 - 1] == '\t'))) p2--;
        while ((p1 < p2) && ((line[p1]     == ' ') || (line[p1]     == '\t'))) p1++;

        // strip whitespace from front and back of val
        while ((p4 > p3) && ((line[p4 - 1] == ' ') || (line[p4 - 1] == '\t'))) p4--;
        while ((p3 < p4) && ((line[p3]     == ' ') || (line[p3]     == '\t'))) p3++;

        // allow quotes to enclose val
        int quote;
        if (((p4 - p3) >= 2) && (((quote = line[p3]) == '\"') || (quote == '\'')) && (line[p4 - 1] == quote))
        {
          // shink limits over quotes
          p3++;
          p4--;
        }
          
        Set(line.substr(p1, p2 - p1), line.substr(p3, p4 - p3));
      }
    }

    file.fclose();

    success = true;
  }

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Get string value and substitute other referenced variables
 *
 * @note the return is passed through Substitute()
 */
/*--------------------------------------------------------------------------------*/
bool SystemParameters::GetSubstituted(const std::string& name, std::string& val) const
{
  ThreadLock lock(tlock);
  bool success = false;
  if (parameters.Get(name, val))
  {
    val = Substitute(val);
    success = true;
  }
  return success;
}

/*--------------------------------------------------------------------------------*/
/** Return whether a parameter exists
 */
/*--------------------------------------------------------------------------------*/
bool SystemParameters::Exists(const std::string& name) const
{
  ThreadLock lock(tlock);
  return parameters.Exists(name);
}
 
/*--------------------------------------------------------------------------------*/
/** Replace {} entries from other values
 *
 * This will look for other values surrounded by '{' and '}' and replace them with
 * the values' value.  '{env:<envvar>}' means environment var '<envvar>' UNLESS
 * a parameter exists with the name 'env:<envvar>'.
 *
 * @param str string to replace entries in
 * @param replaceunknown true to replace unknown entries by blank, false to leave as is
 *
 * @return string with substitutions
 */
/*--------------------------------------------------------------------------------*/
std::string SystemParameters::Substitute(const std::string& str, bool replaceunknown) const
{
  ThreadLock lock(tlock);
  std::string res = str;
  size_t p1 = 0, p2;

  // find sets of '{}' and replace them by their dereferenced values
  while (((p1 = res.find("{", p1))     < std::string::npos) &&
         ((p2 = res.find("}", p1 + 1)) < std::string::npos))
  {
    std::string var = res.substr(p1 + 1, p2 - p1 - 1), val;
    const char *env = NULL;

    BBCDEBUG3(("Found var '%s' at %s", var.c_str(), StringFrom(p1).c_str()));
    
    if (parameters.Exists(var))
    {
      parameters.Get(var, val);
      res = res.substr(0, p1) + val + res.substr(p2 + 1);
      // leave p1 where it is so substitution is attempted again at this position
    }
    else if ((var.substr(0, 4) == "env:") && ((env = getenv(var.substr(4).c_str())) != NULL))
    {
      // use env variable
      res = res.substr(0, p1) + std::string(env) + res.substr(p2 + 1);
      // leave p1 where it is so substitution is attempted again at this position
    }
    else if (replaceunknown)
    {
      // unknown var -> replace by blank
      res = res.substr(0, p1) + res.substr(p2 + 1);
    }
    else
    {
      // unknown var -> skip over
      p1 = p2 + 1;
    }

    BBCDEBUG3(("New string '%s', searching from %s", res.c_str(), StringFrom(p1).c_str()));
  }

  BBCDEBUG2(("Substitute: converted '%s' to '%s'", str.c_str(), res.c_str()));
  
  return res;
}

/*--------------------------------------------------------------------------------*/
/** Iterate through a list of paths, substituting and expanding paths where necessary
 */
/*--------------------------------------------------------------------------------*/
void SystemParameters::SubstitutePathList(std::vector<std::string>& paths) const
{
  uint_t i;

  // process each path separately
  for (i = 0; i < (uint_t)paths.size();)
  {
    std::string path = paths[i];
    size_t p1, p2;

    if (((p1 = path.find("{"))         < std::string::npos) &&
        ((p2 = path.find("}", p1 + 1)) < std::string::npos))
    {
      std::string var = path.substr(p1 + 1, p2 - p1 - 1);
      std::string val, left = path.substr(0, p1), right = path.substr(p2 + 1);

      // if var doesn't exist, it will be replaced by blank anyway
      parameters.Get(var, val);

      // split replacement
      std::vector<std::string> subpaths;
      SplitString(val, subpaths, ';');
        
      // remove path from list before expanding
      paths.erase(paths.begin() + i);

      // add combination of each subpath and remainder of string
      uint_t j;
      for (j = 0; j < (uint_t)subpaths.size(); j++)
      {
        paths.push_back(left + subpaths[j] + right);
      }
    }
    // ignore normal paths
    else i++;
  }
}

/*--------------------------------------------------------------------------------*/
/** Replace {} entries from other values, assuming strings are lists of paths
 *
 * This is similar to the above *except* that substitutions as assumed to be a list
 * of paths, so:
 *   '{sharedir}/subdir'
 * when {sharedir} is 'share;../share', is expanded to:
 *   'share/subdir;../share/subdir'
 *
 * @param str string to replace entries in
 *
 * @return string with substitutions
 *
 * @note there is no 'replaceunknown' option here - any unknown entries are *always* replaced by blanks 
 */
/*--------------------------------------------------------------------------------*/
std::string SystemParameters::SubstitutePathList(const std::string& str) const
{
  ThreadLock lock(tlock);
  std::vector<std::string> paths;
  std::string res;

  // split input string into a list of paths
  SplitString(str, paths, ';');

  // perform repeated substitution and expansion
  SubstitutePathList(paths);

  // reconstitute pathlist
  uint_t i;
  for (i = 0; i < (uint_t)paths.size(); i++)
  {
    if (i) res += ";";
    res += paths[i];
  }

  BBCDEBUG2(("SubstitutePath: converted '%s' to '%s'", str.c_str(), res.c_str()));
  
  return res;
}

BBC_AUDIOTOOLBOX_END
