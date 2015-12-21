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
 *
 * GetReadBuffersAvailable() always returns number of buffers that can be read
 * Read-ahead allows valid read buffers beyond the next one to be accessed, using GetReadBuffer(<x>)
 *
 * GetWriteBuffersAvailable() always returns number of buffers that can be written to
 * Write-ahead allows buffers to be written (but not committed) using GetWriteBuffer(<x>)
 *
 * Notes:
 *  1. to detect empty/full, *one* of the slots is unavailable (to detect the difference between empty and full)
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
   * @param offset write-ahead buffer requested (i.e. write buffers beyond the current write buffer)
   *
   * @return ptr to data to write to or NULL if no free items available
   *
   * @note this function deliberately prevents the last item before the read item
   * from being used
   */
  /*--------------------------------------------------------------------------------*/
  T *GetWriteBuffer(uint_t offset = 0) {return (offset < WriteBuffersAvailable()) ? &buffer[(wr + offset) % buffer.size()] : NULL;}
  
  /*--------------------------------------------------------------------------------*/
  /** Return number of write buffers available
   * 
   * @note number of write buffers = rd - wr - 1 but each subtraction requires addition of buffer size to prevent underflow
   */
  /*--------------------------------------------------------------------------------*/
  uint_t WriteBuffersAvailable() const {return ((rd + 2 * buffer.size() - wr - 1) % buffer.size());}

  /*--------------------------------------------------------------------------------*/
  /** Increment the write pointer (after writing data, essentially committing buffers)
   *
   * @param n number of buffers to commit
   *
   * @return true if pointer increments, false if it is not possible
   */
  /*--------------------------------------------------------------------------------*/
  bool IncrementWrite(uint_t n = 1)
  {
    uint_t avail = WriteBuffersAvailable();
    if ((n = MIN(n, avail)) > 0) {wr = (wr + n) % buffer.size(); return true;}
    return false;
  }

  /*--------------------------------------------------------------------------------*/
  /** Return how many occupied read buffers are available
   * 
   * @note number of read buffers = wr - rd but the subtraction requires addition of buffer size to prevent underflow
   */
  /*--------------------------------------------------------------------------------*/
  uint_t ReadBuffersAvailable() const {return ((wr + buffer.size() - rd) % buffer.size());}

  /*--------------------------------------------------------------------------------*/
  /** Return ptr to item at read position
   *
   * @param offset offset from current read pointer to get buffer for
   *
   * @return ptr to data to read from or NULL if no items available
   */
  /*--------------------------------------------------------------------------------*/
  const T *GetReadBuffer(uint_t offset = 0) const {return (offset < ReadBuffersAvailable()) ? &buffer[(rd + offset) % buffer.size()] : NULL;}
  T       *GetReadBuffer(uint_t offset = 0)       {return (offset < ReadBuffersAvailable()) ? &buffer[(rd + offset) % buffer.size()] : NULL;}

  /*--------------------------------------------------------------------------------*/
  /** Increment the read pointer (after reading data)
   *
   * @param n number of slots to increment read pointer by
   *
   * @return true if pointer increments, false if it is not possible
   */
  /*--------------------------------------------------------------------------------*/
  bool IncrementRead(uint_t n = 1)
  {
    uint_t avail = ReadBuffersAvailable();
    if ((n = MIN(n, avail)) > 0) {rd = (rd + n) % buffer.size(); return true;}
    return false;
  }

  /*--------------------------------------------------------------------------------*/
  /** Reset the buffer (losing all data)
   */
  /*--------------------------------------------------------------------------------*/
  void Reset() {rd = wr;}

protected:
  std::vector<T> buffer;
  volatile uint_t rd, wr;
};

BBC_AUDIOTOOLBOX_END

#endif
