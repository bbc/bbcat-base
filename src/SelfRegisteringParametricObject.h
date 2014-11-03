#ifndef __SELF_REGISTERING_PARAMETRIC_OBJECT__
#define __SELF_REGISTERING_PARAMETRIC_OBJECT__

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
  /** typedef for creator function
   */
  /*--------------------------------------------------------------------------------*/
  typedef SelfRegisteringParametricObject *(*CREATOR)(const ParameterSet& parameters);

  /*--------------------------------------------------------------------------------*/
  /** registration function (called by SELF_REGISTER macro below)
   */
  /*--------------------------------------------------------------------------------*/
  static uint_t Register(const char *type, CREATOR creator);

  /*--------------------------------------------------------------------------------*/
  /** create an instance of the specified type
   */
  /*--------------------------------------------------------------------------------*/
  static SelfRegisteringParametricObject *Create(const char *type, const ParameterSet& parameters);

  /*--------------------------------------------------------------------------------*/
  /** Get a list of objects that can be created (optionally restricted)
   *
   * @param match a string that *must* appear at the start of the name for it to be entered into the list
   */
  /*--------------------------------------------------------------------------------*/
  static void GetList(std::vector<const char *>& list, const char *match = ""); 

protected:
  /*--------------------------------------------------------------------------------*/
  /** this is deliberately a pointer, see code for reasons 
   */
  /*--------------------------------------------------------------------------------*/
  static const std::map<const char *,CREATOR> *creators;
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
#define SELF_REGISTER(type, name) uint_t type::_dummy = SelfRegisteringParametricObject::Register(name, &type::Create);

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
  static uint_t _dummy;                                                 \
public:                                                                 \
  type(const ParameterSet& parameters);                                 \
  static SelfRegisteringParametricObject *Create(const ParameterSet& parameters) {return new type(parameters);}

BBC_AUDIOTOOLBOX_END

#endif


