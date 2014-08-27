
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_LEVEL 1
#include "EnhancedFile.h"

BBC_AUDIOTOOLBOX_START

EnhancedFile::EnhancedFile() : fp(NULL)
{
}

EnhancedFile::EnhancedFile(const EnhancedFile& obj) : fp(NULL)
{
  operator = (obj);
}

EnhancedFile::~EnhancedFile()
{
  fclose();
}

EnhancedFile& EnhancedFile::operator = (const EnhancedFile& obj)
{
  fclose();

  if (obj.isopen())
  {
    if (fopen(obj.filename.c_str(), obj.mode.c_str()))
    {
      fseek(obj.ftell(), SEEK_SET);
    }
  }

  return *this;
}

bool EnhancedFile::fopen(const char *filename, const char *mode)
{
  bool success = false;

  if (!isopen())
  {
    if ((fp = ::fopen(filename, mode)) != NULL)
    {
      DEBUG2(("Opened '%s' for '%s'", filename, mode));
      this->filename = filename;
      this->mode     = mode;
      success        = true;
    }
    else DEBUG2(("Failed to open '%s' for '%s'", filename, mode));
  }

  return success;
}

void EnhancedFile::fclose()
{
  if (fp)
  {
    ::fclose(fp);
    fp = NULL;

    filename = "";
    mode     = "";
  }
}

int EnhancedFile::fprintf(const char *fmt, ...)
{
  int res = -1;

  if (fp)
  {
    va_list ap;
    va_start(ap, fmt);
    res = vfprintf(fmt, ap);
    va_end(ap);
  }

  return res;
}

int EnhancedFile::vfprintf(const char *fmt, va_list ap)
{
  int res = -1;

  if (fp) ::vfprintf(fp, fmt, ap);

  return res;
}

BBC_AUDIOTOOLBOX_END

