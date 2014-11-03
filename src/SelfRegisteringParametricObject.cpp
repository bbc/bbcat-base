
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_LEVEL 1
#include "SelfRegisteringParametricObject.h"

BBC_AUDIOTOOLBOX_START

// a pointer to the map of creators, see ::Register() for an explanation of why this is a pointer
const std::map<const std::string,SelfRegisteringParametricObject::CREATOR> *SelfRegisteringParametricObject::creators = NULL;

/*--------------------------------------------------------------------------------*/
/** registration function (called by SELF_REGISTER macro below)
 */
/*--------------------------------------------------------------------------------*/
uint_t SelfRegisteringParametricObject::Register(const char *type, CREATOR creator)
{
  // because we can't control the order of static creation (dictated by link order), we
  // cannot control whether individual registration functions will be called BEFORE any
  // static objects in this class are created
  // therefore we create the map here and then set an external pointer to it
  // ::Create() is safe to call BEFORE this 
  static std::map<const std::string,CREATOR> _creators;

  DEBUG1(("Registering object type '%s'", type));

  // set creator in map
  _creators[type] = creator;

  // set external pointer to static object
  creators = &_creators;

  // return arbitrary value, doesn't matter what it is just as long as a value is returned
  return _creators.size();
}

/*--------------------------------------------------------------------------------*/
/** create an instance of the specified type
 */
/*--------------------------------------------------------------------------------*/
SelfRegisteringParametricObject *SelfRegisteringParametricObject::CreateObject(const char *type, const ParameterSet& parameters)
{
  std::map<const std::string,CREATOR>::const_iterator it;
  SelfRegisteringParametricObject *obj = NULL;

  if (creators && ((it = creators->find(type)) != creators->end()))
  {
    // call creator with parameters
    obj = (*it->second)(parameters);
  }
  else ERROR("Failed to find creator for object '%s'", type);

  return obj;
}

/*--------------------------------------------------------------------------------*/
/** Get a list of objects that can be created (optionally restricted)
 *
 * @param match a string that *must* appear at the start of the name for it to be entered into the list
 */
/*--------------------------------------------------------------------------------*/
void SelfRegisteringParametricObject::GetList(std::vector<const std::string>& list, const char *match)
{
  if (creators)
  {
    std::map<const std::string,CREATOR>::const_iterator it;
    uint_t l = match ? strlen(match) : 0;

    for (it = creators->begin(); it != creators->end(); ++it)
    {
      if (!match || (l && (strncasecmp(it->first.c_str(), match, l) == 0))) list.push_back(it->first);
    }
  }
}

BBC_AUDIOTOOLBOX_END
