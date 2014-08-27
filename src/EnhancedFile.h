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

  EnhancedFile& operator = (const EnhancedFile& obj);

  virtual EnhancedFile *dup() const {return new EnhancedFile(*this);}

  virtual bool     fopen(const char *filename, const char *mode = "rb");
  bool             isopen() const {return (fp != NULL);}
  virtual void     fclose();

  operator FILE *() {return fp;}

  virtual size_t   fread(void *ptr, size_t size, size_t count)  {return ::fread(ptr, size, count, fp);}
  virtual size_t   fwrite(void *ptr, size_t size, size_t count) {return ::fwrite(ptr, size, count, fp);}
  virtual long int ftell() const {return ::ftell(fp);}
  virtual int      fseek(long int offset, int origin) {return ::fseek(fp, offset, origin);}
  virtual int      ferror() const {return ::ferror(fp);}
  virtual int      fflush() const {return ::fflush(fp);}
  virtual void     rewind() {::rewind(fp);}

  virtual int      fprintf(const char *fmt, ...) __attribute__ ((format (printf,2,3)));
  virtual int      vfprintf(const char *fmt, va_list ap);

  const std::string& getfilename() const {return filename;}

protected:
  std::string filename;
  std::string mode;
  FILE        *fp;
};

BBC_AUDIOTOOLBOX_END

#endif
