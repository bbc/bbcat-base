# Object Registration and Creation

The object creation code is designed to allow creating of objects, by
specifying their names and parameters in a configuration file, rather than
creating them directly in the application.

This removes the need for glue logic in the main application.

To make this happen, classes are registered with a central registry, which
knows the name, parameter types, and how to create objects. The main
application uses the registry to create the objects and link them together,
using parameters specified in the config file.

## Class Registry

Objects are registered into the ObjectRegistry singleton, which stores a map
from the name of the class, to the factory object used to create it.

Object factories provide an interface to the class which they create; allowing
overloaded behaviour. They provide the following information about their class:

- Whether the class is a singleton.
- The textual name of the class.
- The specification of the parameters and controls.
- Methods to create an instance of the class.
- A priority for this factory, compared to other factories of the same name
  (allows overriding of functionality at run time).

There are two current uses for objects that factories can create:

- An object that has parameters.
- An object that has controls. See render library documentation for details.

As far as objects with parameters are concerned, two types of objects can be
created:

- non-singletons, of which there may be many instances, this is the default
- singletons, of which there may only be one. Singletons must specify this in
  their class definition (see below).

## Class Registration

Classes to be created should derive from the SelfRegisteringParametricObject
class, overriding the appropriate methods. To create the appropriate factories,
a macro is provided:

- SELF_REGISTERING_PARAMETRIC_OBJECT() creates a static instance of the
  SelfRegisteringParametricObjectFactory template class, which registers itself
  with the ObjectRegistry singleton on initialisation.

To specify a singleton, a static member function *and* virtual member function
must be specified, a macro is provided for this purpose:

- SELF_REGISTERING_PARAMETRIC_OBJECT_IS_SINGLETON() when included in the class
  definition will make any objects of this type (or derived type) a singleton.

### An Example

This creates an registered class Foo (registered as "foo"), with a signle integer parameter, "bar".

*WARNING:* This example is a bit half baked!

.h file:

~~~{.cpp}
#include <bbcat-base/SelfRegisteringParametricObject.h>
#include <bbcat-base/ParameterSet.h>

BBC_AUDIOTOOLBOX_START

class Foo : public SelfRegisteringParametricObject
{
    int bar;
  public:
    Foo(const ParameterSet& parameters);
    virtual void SetParameters(const ParameterSet& parameters);
    static void GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list);

    // enable if this class is a singleton
    // SELF_REGISTERING_PARAMETRIC_OBJECT_IS_SINGLETON()
};

BBC_AUDIOTOOLBOX_END
~~~

.cpp file:

~~~{.cpp}
#include "foo.h"

BBC_AUDIOTOOLBOX_START

static const PARAMETERDESC _parameters[] =
{
  {"bar", "Description of bar"},
};

// these MUST be in the same order as the above
enum
{
  Parameter_bar = 0,
};
  
SELF_REGISTERING_PARAMETRIC_OBJECT(Foo, "foo");

Foo::Foo(const ParameterSet& parameters)
  :SelfRegisteringParametricObject(parameters)
  ,bar(5) // default value of bar if no parameter is given
{
  // extract and process any parameters here that
  // can ONLY be set at construction

  // call generic set parameters function (below)
  SetParameters(parameters);
}

void Foo::SetParameters(const ParameterSet& parameters)
{
  // ensure parent class function(s) are called!
  SelfRegisteringParametricObject::SetParameters(parameters);
  
  // extract and process any parameters here that
  // can be set multiple times or changed
  parameters.Get(_parameters.bar.name, bar);
}

void Foo::GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list)
{
  // add parameters from parent class(es)
  SelfRegisteringParametricObject::GetParameterDescriptions(list);

  // add parameters from this class
  AddParametersToList(_parameters, NUMBEROF(_parameters), list);
}

BBC_AUDIOTOOLBOX_END
~~~

## Class Construction

## Configuration
