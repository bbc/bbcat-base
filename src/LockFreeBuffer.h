#ifndef __LOCK_FREE_BUFFER__
#define __LOCK_FREE_BUFFER__

#include <vector>

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Lock-free fixed-size circular buffer
 *
 * To write:
 * Call GetWriteBuffer(), if it returns non-NULL, write to the buffer and then call IncrementWrite()
 *
 * To read:
 * Call GetReadBuffer(), if it returns non-NULL, read from the buffer and then call IncrementRead()
 */
/*--------------------------------------------------------------------------------*/
template<typename T>
class LockFreeBuffer
{
public:
  /*--------------------------------------------------------------------------------*/
  /** Initialise the buffer
   *
   * @note the buffer is initialised to one more than required because there must
   * always be an unused item in the list
   */
  /*--------------------------------------------------------------------------------*/
  LockFreeBuffer(uint_t l = 0) : buffer(l + 1),
                                 rd(0),
                                 wr(0) {}
  virtual ~LockFreeBuffer() {}

  /*--------------------------------------------------------------------------------*/
  /** Resize the buffer and reset the pointers
   *
   * @note the buffer is initialised to one more than required because there must
   * always be an unused item in the list
   *
   * @note this will effectively empty the buffer
   */
  /*--------------------------------------------------------------------------------*/
  void Resize(uint_t l) {buffer.resize(l + 1); rd = wr = 0;}

  /*--------------------------------------------------------------------------------*/
  /** Return ptr to item at write position
   *
   * @return ptr to data to write to or NULL if no free items available
   *
   * @note this function deliberately prevents the last item before the read item
   * from being used
   */
  /*--------------------------------------------------------------------------------*/
  T *GetWriteBuffer() {return WriteBufferAvailable() ? &buffer[wr] : NULL;}

  /*--------------------------------------------------------------------------------*/
  /** Increment the write pointer (after writing data)
   *
   * @return true if pointer increments, false if it is not possible
   */
  /*--------------------------------------------------------------------------------*/
  bool IncrementWrite()
  {
    if (WriteBufferAvailable()) {wr = (wr + 1) % buffer.size(); return true;}
    return false;
  }

  /*--------------------------------------------------------------------------------*/
  /** Return ptr to item at read position
   *
   * @return ptr to data to read from or NULL if no items available
   */
  /*--------------------------------------------------------------------------------*/
  const T *GetReadBuffer() const {return ReadBufferAvailable() ? &buffer[rd] : NULL;}
  T       *GetReadBuffer()       {return ReadBufferAvailable() ? &buffer[rd] : NULL;}

  /*--------------------------------------------------------------------------------*/
  /** Increment the read pointer (after reading data)
   *
   * @return true if pointer increments, false if it is not possible
   */
  /*--------------------------------------------------------------------------------*/
  bool IncrementRead()
  {
    if (ReadBufferAvailable()) {rd = (rd + 1) % buffer.size(); return true;}
    return false;
  }

  /*--------------------------------------------------------------------------------*/
  /** Reset the buffer (losing all data)
   */
  /*--------------------------------------------------------------------------------*/
  void Reset() {rd = wr;}

protected:
  /*--------------------------------------------------------------------------------*/
  /** Return whether a free write buffer is available
   */
  /*--------------------------------------------------------------------------------*/
  bool WriteBufferAvailable() const {return (((wr + 1) % buffer.size()) != rd);}

  /*--------------------------------------------------------------------------------*/
  /** Return whether a read buffer is available
   */
  /*--------------------------------------------------------------------------------*/
  bool ReadBufferAvailable()  const {return (wr != rd);}

protected:
  std::vector<T> buffer;
  volatile uint_t rd, wr;
};

BBC_AUDIOTOOLBOX_END

#endif
