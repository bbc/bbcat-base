#ifndef __ENHANCED_FILE__
#define __ENHANCED_FILE__

#include <stdarg.h>

#include <string>

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Class mimicking FILE functions but which keeps the filename and open mode allowing duplication 
 */
/*--------------------------------------------------------------------------------*/
class EnhancedFile
{
public:
  EnhancedFile();
  EnhancedFile(const EnhancedFile& obj);
  virtual ~EnhancedFile();

  /*--------------------------------------------------------------------------------*/
  /** Duplicate file by assignment
   */
  /*--------------------------------------------------------------------------------*/
  EnhancedFile& operator = (const EnhancedFile& obj);

  /*--------------------------------------------------------------------------------*/
  /** Explicit duplication via copy-constructor
   */
  /*--------------------------------------------------------------------------------*/
  virtual EnhancedFile *dup() const {return new EnhancedFile(*this);}

  /*--------------------------------------------------------------------------------*/
  /** Mirrors of standard fxxxx() functions
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool   fopen(const char *filename, const char *mode = "rb");
  bool           isopen() const {return (fp != NULL);}
  virtual void   fclose();

  virtual size_t fread(void *ptr, size_t size, size_t count)        {return ::fread(ptr, size, count, fp);}
  virtual size_t fwrite(const void *ptr, size_t size, size_t count) {return ::fwrite(ptr, size, count, fp);}
  virtual off_t  ftell() const {return ::ftello(fp);}
  virtual int    fseek(off_t offset, int origin) {return ::fseeko(fp, offset, origin);}
  virtual int    ferror() const {return ::ferror(fp);}
  virtual int    fflush() const {return ::fflush(fp);}
  virtual void   rewind() {::rewind(fp);}

  virtual int    fprintf(const char *fmt, ...) __attribute__ ((format (printf,2,3)));
  virtual int    vfprintf(const char *fmt, va_list ap);

  /*--------------------------------------------------------------------------------*/
  /** Read a line of text from an open file
   *
   * @param line buffer to receive text
   * @param maxlen maximum number of bytes that 'line' can hold (INCLUDING terminator)
   *
   * @return number of chracters in buffer (excluding terminator), EOF on end of file (with no characters stored)
   */
  /*--------------------------------------------------------------------------------*/
  int readline(char *line, uint_t maxlen);

  const std::string& getfilename() const {return filename;}

  /*--------------------------------------------------------------------------------*/
  /** Return whether a file exists
   */
  /*--------------------------------------------------------------------------------*/
  static bool exists(const char *filename);

  /*--------------------------------------------------------------------------------*/
  /** Concatenate two paths
   */
  /*--------------------------------------------------------------------------------*/
  static std::string catpath(const std::string& dir1, const std::string& dir2);

protected:
  std::string filename;
  std::string mode;
  FILE        *fp;
  bool        allowclose;
};

BBC_AUDIOTOOLBOX_END

#endif
