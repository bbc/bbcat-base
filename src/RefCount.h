#ifndef __REF_COUNT__
#define __REF_COUNT__

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Simple reference count management template
 *
 * For an object that has IncRef() and DecRef() member functions for reference counting, this
 * template can make managing those objects easier
 *
 * Classes used must contain the member functions:
 *   void IncRef();
 *   bool DecRef();
 *
 * Or derive the class from RefCountedObject below
 *
 * For example, EnhancedFile has IncRef() and DecRef() member functions so:
 *
 * RefCount<> fileref(file = new EnhancedFile(...));
 * 
 */
/*--------------------------------------------------------------------------------*/
template<typename T>
class RefCount
{
public:
  /*--------------------------------------------------------------------------------*/
  /** Constructor
   */
  /*--------------------------------------------------------------------------------*/
  RefCount(T *_obj = NULL) : obj(NULL) {Attach(_obj);}
  RefCount(const RefCount& ref) : obj(NULL) {Attach(ref.Obj());}
  /*--------------------------------------------------------------------------------*/
  /** Destructor
   */
  /*--------------------------------------------------------------------------------*/
  ~RefCount() {Release();}

  /*--------------------------------------------------------------------------------*/
  /** Copy the target object
   */
  /*--------------------------------------------------------------------------------*/
  RefCount& operator = (const RefCount& ref) {Attach(ref.Obj()); return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Set the target object
   */
  /*--------------------------------------------------------------------------------*/
  RefCount& operator = (T *_obj) {Attach(_obj); return *this;}

  /*--------------------------------------------------------------------------------*/
  /** Access target object in various ways
   */
  /*--------------------------------------------------------------------------------*/
  operator T *() const {return obj;}

  T *Obj() const {return obj;}

protected:
  /*--------------------------------------------------------------------------------*/
  /** Set the target object for this object and increment its refcount
   */
  /*--------------------------------------------------------------------------------*/
  void Attach(T *_obj)
  {
    if (_obj != obj)
    {
      // increment ref on new object before decrementing ref on old object
      if (_obj) _obj->IncRef();
      // decrement ref on old object and delete it if neessary
      Release();
      // set new object
      obj = _obj;
    }
  }

  /*--------------------------------------------------------------------------------*/
  /** Decrement the target object's refcount and delete it if is zero (no longer referenced)
   */
  /*--------------------------------------------------------------------------------*/
  void Release() {
    if (obj && obj->DecRef()) delete obj;
    obj = NULL;
  }

protected:
  T *obj;
};

/*--------------------------------------------------------------------------------*/
/** Base class (optional) for ref-counting objects
 */
/*--------------------------------------------------------------------------------*/
class RefCountedObject
{
public:
  RefCountedObject(bool _preventdeletion = false) : refcount(0),
                                                    preventdeletion(_preventdeletion) {}
  virtual ~RefCountedObject() {}

  /*--------------------------------------------------------------------------------*/
  /** Prevent deletion of this object
   *
   * @note NOT thread safe!
   */
  /*--------------------------------------------------------------------------------*/
  virtual void PreventDeletion(bool prevent = true) {preventdeletion = prevent;}

  /*--------------------------------------------------------------------------------*/
  /** Increment reference count for this object
   *
   * @note NOT thread safe!
   */
  /*--------------------------------------------------------------------------------*/
  virtual void IncRef() {refcount++;}

  /*--------------------------------------------------------------------------------*/
  /** Decrement reference count for this object and return whether the result is zero (i.e. the object can be deleted)
   *
   * @note NOT thread safe!
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool DecRef() {return (((--refcount) == 0) && !preventdeletion);}

  /*--------------------------------------------------------------------------------*/
  /** Return whether this object is shared by more than one owner
   *
   * @note this can be used for copy-on-write behaviour
   */
  /*--------------------------------------------------------------------------------*/
  bool IsShared() const {return (refcount > 1);}

protected:
  uint_t refcount;
  bool   preventdeletion;
};

BBC_AUDIOTOOLBOX_END

#endif
