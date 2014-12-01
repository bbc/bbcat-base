
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
/** Invalidate object (usually during construction)
 */
/*--------------------------------------------------------------------------------*/
void SelfRegisteringParametricObject::InvalidateObject()
{
  objectvalid = false;
}

/*--------------------------------------------------------------------------------*/
/** Set parameters within object (*only* parameters that can be set more than once)
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
 *
 * @param name object type name
 * @param parameters a set of parameters to create the object with
 * @param factory a pointer to a variable to receive the factory used to create the object
 *
 * @return pointer to object or NULL
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
 *
 * @param name object type name
 * @param parameters a set of parameters to create the object with
 *
 * @return index that object was registered using (may be global or local to a category)
 */
/*--------------------------------------------------------------------------------*/
int SelfRegisteringParametricObjectContainer::Create(const char *name, const ParameterSet& parameters)
{
  SelfRegisteringParametricObject *obj;
  int index = -1;

  if ((obj = CreateObject(name, parameters)) != NULL)
  {
    if (!obj->IsObjectValid())
    {
      // object construction failed for some reason
      ERROR("Failed to create object '%s' correctly", name);

      // ONLY delete object if it is not a singleton
      if (!obj->IsSingleton()) delete obj;
    }
    // don't attempt to register singletons!
    else if (!obj->IsSingleton())
    {
      // object was created, find out what type it is
      if ((index = Register(obj, parameters)) < 0)
      {
        ERROR("Unknown type '%s' (unable to register with container)", name);
        delete obj;
      }
    }
  }
  else ERROR("Unknown type '%s' (cannot create)", name);

  return index;
}

BBC_AUDIOTOOLBOX_END
