#ifndef __SELF_REGISTERING_PARAMETRIC_OBJECT__
#define __SELF_REGISTERING_PARAMETRIC_OBJECT__

#include <string>
#include <map>
#include <vector>

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
  /** Get a list of parameters for this object
   */
  /*--------------------------------------------------------------------------------*/
  static void GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list);

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
  /** typedef for GetParameterDescriptions function
   */
  /*--------------------------------------------------------------------------------*/
  typedef void (*GETPARAMETERS)(std::vector<const PARAMETERDESC *>& list);

  /*--------------------------------------------------------------------------------*/
  /** registration function (called by SELF_REGISTER macro below)
   */
  /*--------------------------------------------------------------------------------*/
  static uint_t RegisterSelfRegisteringParameterObjectCreator(const char *type, CREATOR creator, GETPARAMETERS getparameters);

  /*--------------------------------------------------------------------------------*/
  /** Get a list of objects that can be created (optionally restricted)
   *
   * @param match a string that *must* appear at the start of the name for it to be entered into the list
   */
  /*--------------------------------------------------------------------------------*/
  static void GetList(std::vector<std::string>& list, const char *match = NULL); 

  /*--------------------------------------------------------------------------------*/
  /** create an instance of the specified type
   */
  /*--------------------------------------------------------------------------------*/
  static SelfRegisteringParametricObject *CreateObject(const char *type, const ParameterSet& parameters);

  /*--------------------------------------------------------------------------------*/
  /** Get a list of parameters for a type
   */
  /*--------------------------------------------------------------------------------*/
  static void GetParameters(const char *type, std::vector<const PARAMETERDESC *>& list);

protected:
  typedef struct {
    std::string   name;
    CREATOR       creator;
    GETPARAMETERS getparameters;
  } OBJECTDATA;

  typedef std::map<std::string,OBJECTDATA>::const_iterator Iterator;

protected:
  std::string registeredobjectid;

  /*--------------------------------------------------------------------------------*/
  /** this is deliberately a pointer, see code for reasons 
   */
  /*--------------------------------------------------------------------------------*/
  static const std::map<std::string,OBJECTDATA> *creators;
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
const uint_t type::_selfregisteringparametricobject_dummy = SelfRegisteringParametricObject::RegisterSelfRegisteringParameterObjectCreator(name, &type::CreateRegisteredObjectImplementation, &type::GetParameterDescriptions);

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
  static const uint_t _selfregisteringparametricobject_dummy;           \
  static SelfRegisteringParametricObject *CreateRegisteredObjectImplementation(const ParameterSet& parameters) {return new type(parameters);} \
public:                                                                 \
  type(const ParameterSet& parameters);                                 \
  virtual const char *GetRegisteredObjectTypeName() const;

BBC_AUDIOTOOLBOX_END

#endif


