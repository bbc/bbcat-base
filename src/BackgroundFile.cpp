
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "OSCompiler.h"

#ifdef TARGET_OS_UNIXBSD
#include <unistd.h>
#endif

#ifdef TARGET_OS_WINDOWS
#include "Windows_uSleep.h"
#endif

#define BBCDEBUG_LEVEL 2
#include "BackgroundFile.h"

BBC_AUDIOTOOLBOX_START

BackgroundFile::BackgroundFile() : EnhancedFile(),
                                   enablebackground(false),
                                   first(NULL),
                                   last(NULL)
{
}

BackgroundFile::BackgroundFile(const char *filename, const char *mode) : EnhancedFile(),
                                                                         enablebackground(false),
                                                                         first(NULL),
                                                                         last(NULL)
{
  fopen(filename, mode);
}

BackgroundFile::BackgroundFile(const BackgroundFile& obj) : EnhancedFile(),
                                                            enablebackground(false),
                                                            first(NULL),
                                                            last(NULL)
{
  operator = (obj);
}

BackgroundFile::~BackgroundFile()
{
  fclose();
}

/*--------------------------------------------------------------------------------*/
/** Enable background writing behaviour
 */
/*--------------------------------------------------------------------------------*/
void BackgroundFile::EnableBackground(bool enable)
{
  enablebackground = enable;

  // if background writing becomes disabled, flush buffers to disk and kill thread
  if (!enablebackground) FlushToDisk();
}

/*--------------------------------------------------------------------------------*/
/** Return whether it will be 'quick' to close the file now - indicating the close will be quick
 *
 * @note this essentially returns whether there is more than one block queued to write
 */
/*--------------------------------------------------------------------------------*/
bool BackgroundFile::ReadyToClose() const
{
  // return true in the case of none or only one block queued to write
  // there appears to be a race condition here (first *could* become NULL between the two checks below)
  // BUT first only becomes NULL when the file closes so as long as ReadyToClose() is called by
  // the SAME thread as that will close the file (highly likely) this will never be an issue 
  return (isopen() && !(first && first->next));
}

/*--------------------------------------------------------------------------------*/
/** Write the first block to disk
 */
/*--------------------------------------------------------------------------------*/
void BackgroundFile::WriteBlock()
{
  BLOCK *block = (BLOCK *)first;

  size_t res = EnhancedFile::fwrite(block->data, block->size, block->count);
  if (res == 0) BBCERROR("Failed to write %s * %s bytes to file in background: %s", StringFrom(block->size).c_str(), StringFrom(block->count).c_str(), strerror(ferror()));

  first = first->next;
  free(block);
}

/*--------------------------------------------------------------------------------*/
/** Flush any queued blocks to disk and shutdown thread
 */
/*--------------------------------------------------------------------------------*/
void BackgroundFile::FlushToDisk()
{
  if (thread.IsRunning() || first)
  {
    BBCDEBUG2(("Flushing queued blocks to disk"));

    // tell thread to quit
    thread.Stop();

    // write remaining (including last) blocks
    while (first)
    {
      WriteBlock();
    }

    first = last = NULL;

    BBCDEBUG2(("Flushed all queued blocks to disk"));
  }
}

/*--------------------------------------------------------------------------------*/
/** Thread
 */
/*--------------------------------------------------------------------------------*/
void *BackgroundFile::Run()
{
  while (!thread.StopRequested())
  {
    // ONLY write block if there is a next block to become first
    // (the last block will be handled by FlushToDisk())
    while (first && first->next)
    {
      WriteBlock();
    }

    // thread yielding sleep
    usleep(10000);
  }
  
  return NULL;
}

void BackgroundFile::fclose()
{
  FlushToDisk();
  EnhancedFile::fclose();
}

size_t BackgroundFile::fread(void *ptr, size_t size, size_t count)
{
  // must make sure that all queued blocks are flushed to disk before reading
  FlushToDisk();
  return EnhancedFile::fread(ptr, size, count);
}

size_t BackgroundFile::fwrite(const void *ptr, size_t size, size_t count)
{
  size_t res = 0;

  // if file is open and background writing is enabled
  if (isopen() && enablebackground)
  {
    // create a block and queue it
    BLOCK *block;

    if ((block = (BLOCK *)calloc(1, sizeof(*block) + (size * count))) != NULL)
    {
      // set block up with correct size and count
      block->size  = size;
      block->count = count;

      // copy data
      memcpy(block->data, ptr, size * count);

      // queue block
      if (last) last->next = block;
      last = block;
      if (!first) first = block;

      // if the thread is not running, start it
      if (!thread.IsRunning())
      {
        if (thread.Start(&__ThreadStart, (void *)this))
        {
          BBCDEBUG2(("Created thread for background file writing"));
        }
        else
		{
          BBCERROR("Failed to create thread (%s)", strerror(errno));
		}
      }

      // indicate all data has been written
      res = count;
    }
  }
  else res = EnhancedFile::fwrite(ptr, size, count);

  return res;
}

off_t BackgroundFile::ftell()
{
  // must make sure that all queued blocks are flushed to disk before performing any normal file operations
  FlushToDisk();
  return EnhancedFile::ftell();
}

int BackgroundFile::fseek(off_t offset, int origin)
{
  // must make sure that all queued blocks are flushed to disk before performing any normal file operations
  FlushToDisk();
  return EnhancedFile::fseek(offset, origin);
}

int BackgroundFile::fflush()
{
  // must make sure that all queued blocks are flushed to disk before performing any normal file operations
  FlushToDisk();
  return EnhancedFile::fflush();
}

void BackgroundFile::rewind()
{
  // must make sure that all queued blocks are flushed to disk before performing any normal file operations
  FlushToDisk();
  return EnhancedFile::rewind();
}

int BackgroundFile::fprintf(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  int res = vfprintf(fmt, ap);  // use this class's function to flush queued blocks to disk
  va_end(ap);
  return res;
}

int BackgroundFile::vfprintf(const char *fmt, va_list ap)
{
  // must make sure that all queued blocks are flushed to disk before performing any normal file operations
  FlushToDisk();
  return EnhancedFile::vfprintf(fmt, ap);  
}

BBC_AUDIOTOOLBOX_END
