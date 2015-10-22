#ifndef __SELF_REGISTERING_PARAMETRIC_OBJECT__
#define __SELF_REGISTERING_PARAMETRIC_OBJECT__

#include <string>
#include <map>
#include <vector>

#include "ParameterSet.h"
#include "ObjectRegistry.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Base class for SelfRegisteringParametricObject factories which provide the
 * interface for creating such objects
 */
/*--------------------------------------------------------------------------------*/
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
  /** Create an instance of the correct type
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
  SelfRegisteringParametricObjectFactory(const std::string& _name) : SelfRegisteringParametricObjectFactoryBase(),
                                                                     RegisteredObjectFactory(_name) {Register();}
  // alternate constructor to avoid Register() being called
  SelfRegisteringParametricObjectFactory(const std::string& _name, bool dummy) : SelfRegisteringParametricObjectFactoryBase(),
                                                                                 RegisteredObjectFactory(_name) {UNUSED_PARAMETER(dummy);}
  virtual ~SelfRegisteringParametricObjectFactory() {}

  /*--------------------------------------------------------------------------------*/
  /** Return whether the object this factory makes is a singleton
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool IsSingleton() const {return TYPE::IsTypeSingleton();}

  /*--------------------------------------------------------------------------------*/
  /** Create an instance of the correct type
   */
  /*--------------------------------------------------------------------------------*/
  virtual SelfRegisteringParametricObject *Create(const ParameterSet& parameters)
  {
    if (IsSingleton())
    {
      // create single instance of object only if this function is called
      static TYPE   obj(parameters); 
      static uint_t count = 0;    // reference count
      // on second and subsequent calls, update the parameters of the object 
      if ((++count) > 1) obj.SetParameters(parameters);
      return &obj;
    }
    else return new TYPE(parameters);
  }

  /*--------------------------------------------------------------------------------*/
  /** Get a list of parameters for object
   */
  /*--------------------------------------------------------------------------------*/
  virtual void GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list) const {return TYPE::GetParameterDescriptions(list);}

  /*--------------------------------------------------------------------------------*/
  /** Return relative priority of this factory over other factories using the same name
   */
  /*--------------------------------------------------------------------------------*/
  virtual int GetPriority() const {return TYPE::GetFactoryPriority();}
};

/*--------------------------------------------------------------------------------*/
/** Base objects for self-registering objects that can all be created with parameters
 *
 * @note this is the base class for objects created by the factories above
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
  /** Return whether this object type is a singleton
   */
  /*--------------------------------------------------------------------------------*/
  static bool IsTypeSingleton() {return false;}
  
  /*--------------------------------------------------------------------------------*/
  /** Return whether this object is a singleton
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool IsSingleton() const {return IsTypeSingleton();}

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
  /** Set parameters within object (*only* parameters that can be set more than once)
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetParameters(const ParameterSet& parameters);

  /*--------------------------------------------------------------------------------*/
  /** Get a list of parameters for this object
   */
  /*--------------------------------------------------------------------------------*/
  static void GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list);

  /*--------------------------------------------------------------------------------*/
  /** Return relative priority of this object's factory
   */
  /*--------------------------------------------------------------------------------*/
  static int GetFactoryPriority() {return 0;}

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
static SelfRegisteringParametricObjectFactory<type> __factory_##type(name); \
const RegisteredObjectFactory *factory_##type = &__factory_##type;

/*--------------------------------------------------------------------------------*/
/** Add this to any class definitions that should be singletons
 */
/*--------------------------------------------------------------------------------*/
#define SELF_REGISTERING_PARAMETRIC_OBJECT_IS_SINGLETON()       \
  static  bool IsTypeSingleton() {return true;}                 \
  virtual bool IsSingleton() const {return IsTypeSingleton();}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Useful base class for objects that create and register SelfRegisteringParametricObject objects
 */
/*--------------------------------------------------------------------------------*/
class SelfRegisteringParametricObjectContainer {
public:
  SelfRegisteringParametricObjectContainer() {}
  virtual ~SelfRegisteringParametricObjectContainer() {}

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
  static SelfRegisteringParametricObject *CreateObject(const std::string& name, const ParameterSet& parameters, SelfRegisteringParametricObjectFactoryBase **factory = NULL);

  /*--------------------------------------------------------------------------------*/
  /** Create (self-registered-parametric) object of given name and add it to this object
   *
   * @param name object type name
   * @param parameters a set of parameters to create the object with
   *
   * @return index that object was registered using (may be global or local to a category) or -1 for failure
   */
  /*--------------------------------------------------------------------------------*/
  virtual int Create(const std::string& name, const ParameterSet& parameters);
  
  /*--------------------------------------------------------------------------------*/
  /** Register a self-registering-parametric-object or return -1
   *
   * @param obj object created by the above
   * @param parameters the set of parameters used to create the object with
   *
   * @return index that object was registered using (may be global or local to a category) or -1 for failure
   */
  /*--------------------------------------------------------------------------------*/
  virtual int Register(SelfRegisteringParametricObject *obj, const ParameterSet& parameters) = 0;
};

BBC_AUDIOTOOLBOX_END

#endif
