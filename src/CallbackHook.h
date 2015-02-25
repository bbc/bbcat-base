#ifndef __CALLBACK_LIST__
#define __CALLBACK_LIST__

#include <vector>

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Simple class that calls a hook - can be used to create callback lists 
 */
/*--------------------------------------------------------------------------------*/
class CallbackHook
{
public:
  CallbackHook(void (*_fn)(void *arg), void *_arg) : fn(_fn), arg(_arg) {}
  CallbackHook(const CallbackHook& obj) : fn(obj.fn), arg(obj.arg) {}
  ~CallbackHook() {}

  void Call() {(*fn)(arg);}

  typedef std::vector<CallbackHook> LIST;

protected:
  void (*fn)(void *arg);
  void *arg;
};

BBC_AUDIOTOOLBOX_END

#endif
