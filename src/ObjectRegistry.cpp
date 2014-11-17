#define DEBUG_LEVEL 1
#include "ObjectRegistry.h"

BBC_AUDIOTOOLBOX_START

RegisteredObjectFactory::RegisteredObjectFactory(const char *_name) : name(_name)
{
  DEBUG2(("Registering object type '%s'", _name));
  ObjectRegistry::Get().Register(this);
}

RegisteredObjectFactory::~RegisteredObjectFactory()
{
}

/*--------------------------------------------------------------------------------*/
/** Register a factory with this registry
 */
/*--------------------------------------------------------------------------------*/
void ObjectRegistry::Register(RegisteredObjectFactory *factory)
{
  objects[factory->GetName()] = factory;
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
