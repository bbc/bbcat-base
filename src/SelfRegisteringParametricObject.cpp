
#include <string.h>

#define DEBUG_LEVEL 1
#include "SelfRegisteringParametricObject.h"

BBC_AUDIOTOOLBOX_START

// a pointer to the map of creators, see ::Register() for an explanation of why this is a pointer
const std::map<std::string,SelfRegisteringParametricObject::OBJECTDATA> *SelfRegisteringParametricObject::creators = NULL;

static const struct {
  PARAMETERDESC id;
} _parameters = 
{
  {"id", "User specified ID for this object"},
};

/*--------------------------------------------------------------------------------*/
/** Set parameters within object
 */
/*--------------------------------------------------------------------------------*/
void SelfRegisteringParametricObject::SetParameters(const ParameterSet& parameters)
{
  parameters.Get(_parameters.id.name, registeredobjectid);
}

/*--------------------------------------------------------------------------------*/
/** Get a list of parameters for this object
 */
/*--------------------------------------------------------------------------------*/
void SelfRegisteringParametricObject::GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list)
{
  const PARAMETERDESC *pparameters = (const PARAMETERDESC *)&_parameters;
  uint_t i, n = sizeof(_parameters) / sizeof(pparameters[0]);

  for (i = 0; i < n; i++) list.push_back(pparameters + i);
}

/*--------------------------------------------------------------------------------*/
/** registration function (called by SELF_REGISTER macro below)
 */
/*--------------------------------------------------------------------------------*/
uint_t SelfRegisteringParametricObject::RegisterSelfRegisteringParameterObjectCreator(const char *type, CREATOR creator, GETPARAMETERS getparameters)
{
  // because we can't control the order of static creation (dictated by link order), we
  // cannot control whether individual registration functions will be called BEFORE any
  // static objects in this class are created
  // therefore we create the map here and then set an external pointer to it
  // ::Create() is safe to call BEFORE this 
  static std::map<std::string,OBJECTDATA> _creators;
  OBJECTDATA data = {type, creator, getparameters};

  DEBUG2(("Registering object type '%s'", type));
#if DEBUG_LEVEL >= 3
  {
    std::vector<const PARAMETERDESC *> list;
    uint_t i;

    (*getparameters)(list);
  
    for (i = 0; i < list.size(); i++)
    {
      DEBUG("   Parameter '%s': %s", list[i]->name, list[i]->desc);
    }
  }
#endif

  // set creator in map
  _creators[type] = data;

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
  SelfRegisteringParametricObject *obj = NULL;
  Iterator it;

  if (creators && ((it = creators->find(type)) != creators->end()))
  {
    // call creator with parameters
    if (it->second.creator) obj = (*it->second.creator)(parameters);
    else ERROR("Type '%s' cannot be created", type);
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
void SelfRegisteringParametricObject::GetList(std::vector<std::string>& list, const char *match)
{
  if (creators)
  {
    Iterator it;
    uint_t l = match ? strlen(match) : 0;

    for (it = creators->begin(); it != creators->end(); ++it)
    {
      if (!match || !l || (l && (strncasecmp(it->first.c_str(), match, l) == 0))) list.push_back(it->first);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** create an instance of the specified type
 */
/*--------------------------------------------------------------------------------*/
void SelfRegisteringParametricObject::GetParameters(const char *type, std::vector<const PARAMETERDESC *>& list)
{
  Iterator it;

  if (creators && ((it = creators->find(type)) != creators->end()))
  {
    // call GetParameters()
    if (it->second.getparameters) (*it->second.getparameters)(list);
    else ERROR("Type '%s' has no function to get its parameters", type);
  }
  else DEBUG2(("Failed to find entry for object '%s'", type));
}

BBC_AUDIOTOOLBOX_END
