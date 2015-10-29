#ifndef __BACKGROUND_FILE__
#define __BACKGROUND_FILE__

#include "EnhancedFile.h"
#include "Thread.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** An enhancement of EnhancedFile that allows writes to be queued and written to
 * disk in a background thread
 *
 * By default this class operates exactly as EnhancedFile until EnableBackground() is
 * called
 *
 * This class is thread safe as long as ONLY a single thread performs the high-level
 * file operations
 */
/*--------------------------------------------------------------------------------*/
class BackgroundFile : public EnhancedFile {
public:
  BackgroundFile();
  BackgroundFile(const char *filename, const char *mode = "rb");
  BackgroundFile(const BackgroundFile& obj);
  virtual ~BackgroundFile();

  /*--------------------------------------------------------------------------------*/
  /** Enable background writing behaviour
   */
  /*--------------------------------------------------------------------------------*/
  virtual void   EnableBackground(bool enable = true);

  virtual void   fclose();

  /*--------------------------------------------------------------------------------*/
  /** Explicit duplication via copy-constructor
   */
  /*--------------------------------------------------------------------------------*/
  virtual BackgroundFile *dup() const {return new BackgroundFile(*this);}

  virtual size_t fread(void *ptr, size_t size, size_t count);
  virtual size_t fwrite(const void *ptr, size_t size, size_t count);
  virtual off_t  ftell();
  virtual int    fseek(off_t offset, int origin);
  virtual int    fflush();
  virtual void   rewind();

  virtual int    fprintf(const char *fmt, ...) PRINTF_FORMAT2;
  virtual int    vfprintf(const char *fmt, va_list ap);

protected:
  /*--------------------------------------------------------------------------------*/
  /** Write the first block to disk
   */
  /*--------------------------------------------------------------------------------*/
  virtual void WriteBlock();

  /*--------------------------------------------------------------------------------*/
  /** Flush any queued blocks to disk and shutdown thread
   */
  /*--------------------------------------------------------------------------------*/
  virtual void   FlushToDisk();

  /*--------------------------------------------------------------------------------*/
  /** Thread entry point
   */
  /*--------------------------------------------------------------------------------*/
  static void *__ThreadStart(Thread& thread, void *arg)
  {
    UNUSED_PARAMETER(thread);
    BackgroundFile& writer = *(BackgroundFile *)arg;
    return writer.Run();
  }

  /*--------------------------------------------------------------------------------*/
  /** Thread
   */
  /*--------------------------------------------------------------------------------*/
  void *Run();

  typedef struct _BLOCK
  {
    struct _BLOCK *next;
    size_t  size;
    size_t  count;
    uint8_t data[0];
  } BLOCK;
  
protected:
  bool                   enablebackground;
  Thread                 thread;
  volatile BLOCK		 *first, *last;
};

BBC_AUDIOTOOLBOX_END

#endif
