#define DEBUG_LEVEL 1
#include "ObjectRegistry.h"

BBC_AUDIOTOOLBOX_START

RegisteredObjectFactory::RegisteredObjectFactory(const char *_name) : name(_name)
{
}

RegisteredObjectFactory::~RegisteredObjectFactory()
{
}

/*--------------------------------------------------------------------------------*/
/** Register this object with object registry (MUST be called AFTER construction is complete!)
 */
/*--------------------------------------------------------------------------------*/
void RegisteredObjectFactory::Register()
{
  ObjectRegistry::Get().Register(this);
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Register a factory with this registry
 */
/*--------------------------------------------------------------------------------*/
void ObjectRegistry::Register(RegisteredObjectFactory *factory)
{
  std::map<std::string,RegisteredObjectFactory *>::iterator it;
  const std::string& name = factory->GetName();

  // if a factory of this name doesn't exist OR
  // the new one is a higher priority than the existing one,
  // set the factory for this name
  if (((it = objects.find(name)) == objects.end()) ||
      (factory->GetPriority() > it->second->GetPriority()))
  {
    objects[name] = factory;
  }
}

/*--------------------------------------------------------------------------------*/
/** Return the factory for the specified type name or NULL
 */
/*--------------------------------------------------------------------------------*/
RegisteredObjectFactory *ObjectRegistry::GetFactory(const char *name)
{
  RegisteredObjectFactory *factory = NULL;
  std::map<std::string,RegisteredObjectFactory *>::iterator it;

  if ((it = objects.find(name)) != objects.end()) factory = it->second;

  return factory;
}

/*--------------------------------------------------------------------------------*/
/** Return ObjectRegistry singleton
 */
/*--------------------------------------------------------------------------------*/
ObjectRegistry& ObjectRegistry::Get()
{
  static ObjectRegistry registry;
  return registry;
}

BBC_AUDIOTOOLBOX_END
