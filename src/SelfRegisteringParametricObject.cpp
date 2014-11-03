
#define DEBUG_LEVEL 0
#include "SelfRegisteringParametricObject.h"

BBC_AUDIOTOOLBOX_START

// a pointer to the map of creators, see ::Register() for an explanation of why this is a pointer
const std::map<const char *,SelfRegisteringParametricObject::CREATOR> *SelfRegisteringParametricObject::creators = NULL;

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
  static std::map<const char *,CREATOR> _creators;

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
SelfRegisteringParametricObject *SelfRegisteringParametricObject::Create(const char *type, const ParameterSet& parameters)
{
  std::map<const char *,CREATOR>::const_iterator it;
  SelfRegisteringParametricObject *obj = NULL;

  if (creators && ((it = creators->find(type)) != creators->end()))
  {
    // call creator with parameters
    obj = (*it->second)(parameters);
  }
  else ERROR("Failed to find creator for object '%s'", type);

  return obj;
}

BBC_AUDIOTOOLBOX_END
