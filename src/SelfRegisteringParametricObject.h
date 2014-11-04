#ifndef __SELF_REGISTERING_PARAMETRIC_OBJECT__
#define __SELF_REGISTERING_PARAMETRIC_OBJECT__

#include <string>
#include <map>

#include "ParameterSet.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Base objects for self-registering objects that can all be created with parameters
 *
 * Derive an object from this, then:
 * 1. Put the SELF_REGISTER_CREATOR() macro in the class definition, straight after the opening brace
 * 2. Put the SELF_REGISTER() macro in the cpp file
 *
 * From then on, anything can create an instance of your object with parameters!
 */
/*--------------------------------------------------------------------------------*/
class SelfRegisteringParametricObject
{
public:
  SelfRegisteringParametricObject() {}
  virtual ~SelfRegisteringParametricObject() {}

  /*--------------------------------------------------------------------------------*/
  /** Set parameters within object
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetParameters(const ParameterSet& parameters);

  /*--------------------------------------------------------------------------------*/
  /** Return user-supplied ID for this object
   */
  /*--------------------------------------------------------------------------------*/
  const std::string& GetRegisteredObjectID() const {return registeredobjectid;}

  /*--------------------------------------------------------------------------------*/
  /** typedef for creator function
   */
  /*--------------------------------------------------------------------------------*/
  typedef SelfRegisteringParametricObject *(*CREATOR)(const ParameterSet& parameters);

  /*--------------------------------------------------------------------------------*/
  /** typedef for IsObjectOfType function
   */
  /*--------------------------------------------------------------------------------*/
  typedef bool (*ISOBJECTOFTYPE)(const SelfRegisteringParametricObject *obj);

  /*--------------------------------------------------------------------------------*/
  /** registration function (called by SELF_REGISTER macro below)
   */
  /*--------------------------------------------------------------------------------*/
  static uint_t Register(const char *type, CREATOR creator, ISOBJECTOFTYPE isobjectoftype);

  /*--------------------------------------------------------------------------------*/
  /** create an instance of the specified type
   */
  /*--------------------------------------------------------------------------------*/
  static SelfRegisteringParametricObject *CreateObject(const char *type, const ParameterSet& parameters);

  /*--------------------------------------------------------------------------------*/
  /** Get a list of objects that can be created (optionally restricted)
   *
   * @param match a string that *must* appear at the start of the name for it to be entered into the list
   */
  /*--------------------------------------------------------------------------------*/
  static void GetList(std::vector<const std::string>& list, const char *match = ""); 

  /*--------------------------------------------------------------------------------*/
  /** Return whether a previously created object is of the specified type
   */
  /*--------------------------------------------------------------------------------*/
  static bool IsObjectOfType(const SelfRegisteringParametricObject *obj, const char *type);

protected:
  typedef struct {
    CREATOR        creator;
    ISOBJECTOFTYPE isobjectoftype;
  } OBJECTDATA;

  typedef std::map<const std::string,OBJECTDATA>::const_iterator Iterator;

protected:
  std::string registeredobjectid;

  /*--------------------------------------------------------------------------------*/
  /** this is deliberately a pointer, see code for reasons 
   */
  /*--------------------------------------------------------------------------------*/
  static const std::map<const std::string,OBJECTDATA> *creators;
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
#define SELF_REGISTER(type, name)                                       \
const char *type::GetRegisteredObjectTypeName() const {return name;}    \
const volatile uint_t type::_dummy = SelfRegisteringParametricObject::Register(name, &type::CreateRegisteredObjectImplementation, &type::IsRegisteredObjectOfTypeImplementation);

/*--------------------------------------------------------------------------------*/
/** Creator macro
 *
 * @param type class name of type to be registered
 *
 * @note this should appear once in the class definition, preferrably near the top as
 * @note it includes a public: access descriptor
 */
/*--------------------------------------------------------------------------------*/
#define SELF_REGISTER_CREATOR(type)                                     \
private:                                                                \
  static const volatile uint_t _dummy;                                  \
  static SelfRegisteringParametricObject *CreateRegisteredObjectImplementation(const ParameterSet& parameters) {return new type(parameters);} \
  static bool                            IsRegisteredObjectOfTypeImplementation(const SelfRegisteringParametricObject *obj) {return (dynamic_cast<const type *>(obj) != NULL);} \
public:                                                                 \
  type(const ParameterSet& parameters);                                 \
  virtual const char *GetRegisteredObjectTypeName() const;

BBC_AUDIOTOOLBOX_END

#endif


