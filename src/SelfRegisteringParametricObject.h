#ifndef __SELF_REGISTERING_PARAMETRIC_OBJECT__
#define __SELF_REGISTERING_PARAMETRIC_OBJECT__

#include <string>
#include <map>
#include <vector>

#include "ParameterSet.h"
#include "ObjectRegistry.h"

BBC_AUDIOTOOLBOX_START

class SelfRegisteringParametricObject;
class SelfRegisteringParametricObjectFactoryBase {
public:
  SelfRegisteringParametricObjectFactoryBase() {}
  virtual ~SelfRegisteringParametricObjectFactoryBase() {}

  /*--------------------------------------------------------------------------------*/
  /** Return whether the object this factory makes is a singleton
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool IsSingleton() const {return false;}

  /*--------------------------------------------------------------------------------*/
  /** create an instance of the specified type
   */
  /*--------------------------------------------------------------------------------*/
  virtual SelfRegisteringParametricObject *Create(const ParameterSet& parameters) = 0;

  /*--------------------------------------------------------------------------------*/
  /** Get a list of parameters for this object
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list) const = 0;
};

/*--------------------------------------------------------------------------------*/
/** Template for self-registering parametric object factory
 */
/*--------------------------------------------------------------------------------*/
template<class TYPE>
class SelfRegisteringParametricObjectFactory : public SelfRegisteringParametricObjectFactoryBase, public RegisteredObjectFactory {
public:
  SelfRegisteringParametricObjectFactory(const char *_name) : SelfRegisteringParametricObjectFactoryBase(),
                                                              RegisteredObjectFactory(_name) {}
  virtual ~SelfRegisteringParametricObjectFactory() {}

  /*--------------------------------------------------------------------------------*/
  /** create an instance of the specified type
   */
  /*--------------------------------------------------------------------------------*/
  virtual SelfRegisteringParametricObject *Create(const ParameterSet& parameters) {return new TYPE(parameters);}

  /*--------------------------------------------------------------------------------*/
  /** Get a list of parameters for this object
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list) const {return TYPE::GetParameterDescriptions(list);}
};

/*--------------------------------------------------------------------------------*/
/** Template for singleton self-registering parametric object factory
 */
/*--------------------------------------------------------------------------------*/
template<class TYPE>
class SelfRegisteringParametricSingletonFactory : public SelfRegisteringParametricObjectFactory<TYPE> {
public:
  SelfRegisteringParametricSingletonFactory(const char *_name) : SelfRegisteringParametricObjectFactory<TYPE>(_name) {}
  virtual ~SelfRegisteringParametricSingletonFactory() {}

  /*--------------------------------------------------------------------------------*/
  /** Return whether the object this factory makes is a singleton
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool IsSingleton() const {return true;}

  /*--------------------------------------------------------------------------------*/
  /** create a singleton instance of the specified type and return its address
   */
  /*--------------------------------------------------------------------------------*/
  virtual SelfRegisteringParametricObject *Create(const ParameterSet& parameters)
  {
    // create single instance of object only if this function is called
    static TYPE   obj(parameters); 
    static uint_t count = 0;    // reference count
    // on second and subsequent calls, update the parameters of the object 
    if ((++count) > 1) obj.SetParameters(parameters);
    return &obj;
  }
};

/*--------------------------------------------------------------------------------*/
/** Base objects for self-registering objects that can all be created with parameters
 *
 */
/*--------------------------------------------------------------------------------*/
class SelfRegisteringParametricObject
{
public:
  SelfRegisteringParametricObject() : objectvalid(true) {}
  SelfRegisteringParametricObject(const ParameterSet& parameters) : objectvalid(true) {
    SetParameters(parameters);
  }
  virtual ~SelfRegisteringParametricObject() {}

  /*--------------------------------------------------------------------------------*/
  /** Return user-supplied ID for this object
   */
  /*--------------------------------------------------------------------------------*/
  const std::string& GetRegisteredObjectID() const {return registeredobjectid;}

  /*--------------------------------------------------------------------------------*/
  /** Return whether object is valid/constructed successfully      
   */
  /*--------------------------------------------------------------------------------*/
  bool IsObjectValid() const {return objectvalid;}

  /*--------------------------------------------------------------------------------*/
  /** Set parameters within object
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetParameters(const ParameterSet& parameters);

  /*--------------------------------------------------------------------------------*/
  /** Get a list of parameters for this object
   */
  /*--------------------------------------------------------------------------------*/
  static void GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list);

protected:
  /*--------------------------------------------------------------------------------*/
  /** Invalidate object (usually during construction)
   */
  /*--------------------------------------------------------------------------------*/
  virtual void InvalidateObject(); 

protected:
  std::string registeredobjectid;
  bool        objectvalid;
};

/*--------------------------------------------------------------------------------*/
/** Self registering macro
 *
 * @param type class name of type to be registered
 * @param name textual name of object
 *
 * @note this should be put in the cpp file *only*
 */
/*--------------------------------------------------------------------------------*/
#define SELF_REGISTERING_PARAMETRIC_OBJECT(type, name)                  \
static   SelfRegisteringParametricObjectFactory<type> __factory_##type(name); \
volatile RegisteredObjectFactory *factory_##type = &__factory_##type;

#define SELF_REGISTERING_PARAMETRIC_SINGLETON(type, name)               \
static   SelfRegisteringParametricSingletonFactory<type> __factory_##type(name); \
volatile RegisteredObjectFactory *factory_##type = &__factory_##type;

/*----------------------------------------------------------------------------------------------------*/

class SelfRegisteringParametricObjectContainer {
public:
  SelfRegisteringParametricObjectContainer() {}
  virtual ~SelfRegisteringParametricObjectContainer() {}

  /*--------------------------------------------------------------------------------*/
  /** Create an object of the specified type
   */
  /*--------------------------------------------------------------------------------*/
  static SelfRegisteringParametricObject *CreateObject(const char *name, const ParameterSet& parameters, SelfRegisteringParametricObjectFactoryBase **factory = NULL);

  /*--------------------------------------------------------------------------------*/
  /** Create (self-registered-parametric) object of given name and add it to this object
   */
  /*--------------------------------------------------------------------------------*/
  virtual int Create(const char *name, const ParameterSet& parameters);
  
  /*--------------------------------------------------------------------------------*/
  /** Register a self-registering-parametric-object or return -1
   *
   * @return index (if applicable) or -1 for unrecognized type
   */
  /*--------------------------------------------------------------------------------*/
  virtual int Register(SelfRegisteringParametricObject *obj, const ParameterSet& parameters) = 0;
};

BBC_AUDIOTOOLBOX_END

#endif
