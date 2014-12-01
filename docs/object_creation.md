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

- Is the class a singleton?
- The textual name of the class.
- The specification of the parameters and controlls.
- Methods to create an instance of the class.
- A priority for this factory, compared to other factories of the same name.

There are two current uses for objects that factories can create:

- An object that has parameters.
- An object that has controls. See render library documentation for details.

As far as objects with parameters are concerned, two types of objects can be created:

- non-singletons, of which there may be many instances
- singletons, of which there may only be one.


## Class Registration

Classes to be created should derive from the SelfRegisteringParametricObject
class, overriding the appropriate methods. To create the appropriate factories,
two macros are provided:

- SELF_REGISTERING_PARAMETRIC_OBJECT() creates a static instance of the
  SelfRegisteringParametricObjectFactory template class, which registers itself
  with the ObjectRegistry singleton on initialisation.
- SELF_REGISTERING_PARAMETRIC_SINGLETON() creates a static instance of the
  SelfRegisteringParametricSingletonFactory template class, which registers
  itself with the ObjectRegistry singleton on initialisation.

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
    static void GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list);
};

BBC_AUDIOTOOLBOX_END
~~~

.cpp file:

~~~{.cpp}
#include "foo.h"

BBC_AUDIOTOOLBOX_START

static const struct {
  PARAMETERDESC bar;
} _parameters =
{
  {"bar", "Description of bar"},
};

SELF_REGISTERING_PARAMETRIC_OBJECT(Foo, "foo");

Foo::Foo(const ParameterSet& parameters)
{
  parameters.Get(_parameters.bar.name, bar);
  SetParameters(parameters);
}

void Foo::GetParameterDescriptions(std::vector<const PARAMETERDESC *>& list)
{
  const PARAMETERDESC *pparameters = (const PARAMETERDESC *)&_parameters;
  uint_t i, n = sizeof(_parameters) / sizeof(pparameters[0]);

  SelfRegisteringParametricObject::GetParameterDescriptions(list);

  for (i = 0; i < n; i++) list.push_back(pparameters + i);
}

BBC_AUDIOTOOLBOX_END
~~~

## Class Construction

## Configuration
