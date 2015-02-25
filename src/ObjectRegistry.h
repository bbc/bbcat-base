#ifndef __OBJECT_REGISTRY__
#define __OBJECT_REGISTRY__

#include <map>
#include <string>

#include "misc.h"

BBC_AUDIOTOOLBOX_START

class RegisteredObjectFactory
{
public:
  RegisteredObjectFactory(const char *_name);
  virtual ~RegisteredObjectFactory();

  /*--------------------------------------------------------------------------------*/
  /** Get name for the object this factory creates
   */
  /*--------------------------------------------------------------------------------*/
  const std::string& GetName() const {return name;}

  /*--------------------------------------------------------------------------------*/
  /** Return relative priority of this factory
   */
  /*--------------------------------------------------------------------------------*/
  virtual int GetPriority() const = 0;

protected:
  /*--------------------------------------------------------------------------------*/
  /** Register this object with object registry (MUST be called AFTER construction is complete!)
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Register();

protected:
  std::string name;
};

class ObjectRegistry {
public:
  ~ObjectRegistry() {}

  /*--------------------------------------------------------------------------------*/
  /** Register a factory with this registry
   */
  /*--------------------------------------------------------------------------------*/
  void Register(RegisteredObjectFactory *factory);

  /*--------------------------------------------------------------------------------*/
  /** Return the factory for the specified type name or NULL
   */
  /*--------------------------------------------------------------------------------*/
  RegisteredObjectFactory *GetFactory(const char *name);

  /*--------------------------------------------------------------------------------*/
  /** Typedefs
   */
  /*--------------------------------------------------------------------------------*/
  typedef const std::map<std::string,RegisteredObjectFactory *> map;
  typedef map::const_iterator iterator;

  /*--------------------------------------------------------------------------------*/
  /** Return map of objects
   */
  /*--------------------------------------------------------------------------------*/
  map& GetObjects() const {return objects;}

  /*--------------------------------------------------------------------------------*/
  /** Return ObjectRegistry singleton
   */
  /*--------------------------------------------------------------------------------*/
  static ObjectRegistry& Get();

protected:
  ObjectRegistry() {}     // can only be created by a member function -> enforces singleton

protected:
  std::map<std::string,RegisteredObjectFactory *> objects;
};

BBC_AUDIOTOOLBOX_END

#endif
