
#include <string.h>

#define DEBUG_LEVEL 1
#include "SelfRegisteringParametricObject.h"

BBC_AUDIOTOOLBOX_START

/*----------------------------------------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Create an object of the specified type
 */
/*--------------------------------------------------------------------------------*/
SelfRegisteringParametricObject *SelfRegisteringParametricObjectContainer::CreateObject(const char *name, const ParameterSet& parameters, SelfRegisteringParametricObjectFactoryBase **factory)
{
  SelfRegisteringParametricObjectFactoryBase *_factory;
  SelfRegisteringParametricObject *obj = NULL;

  if ((_factory = dynamic_cast<SelfRegisteringParametricObjectFactoryBase *>(ObjectRegistry::Get().GetFactory(name))) != NULL)
  {
    if (factory) *factory = _factory;

    obj = _factory->Create(parameters);
  }

  return obj;
}

/*--------------------------------------------------------------------------------*/
/** Create (self-registered-parametric) object of given name and add it to this object
 */
/*--------------------------------------------------------------------------------*/
int SelfRegisteringParametricObjectContainer::Create(const char *name, const ParameterSet& parameters)
{
  SelfRegisteringParametricObjectFactoryBase *factory;
  SelfRegisteringParametricObject *obj;
  int index = -1;

  if ((obj = CreateObject(name, parameters, &factory)) != NULL)
  {
    // don't attempt to register singletons!
    if (!factory->IsSingleton())
    {
      // object was created, find out what type it is
      if ((index = Register(obj, parameters)) < 0)
      {
        ERROR("Unknown type '%s' (don't known what to do with it)", name);
        delete obj;
      }
    }
  }
  else ERROR("Unknown type '%s' (cannot create)", name);

  return index;
}

BBC_AUDIOTOOLBOX_END
