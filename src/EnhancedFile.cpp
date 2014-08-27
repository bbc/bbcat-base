
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_LEVEL 1
#include "EnhancedFile.h"

BBC_AUDIOTOOLBOX_START

EnhancedFile::EnhancedFile() : fp(NULL),
                               allowclose(false)
{
}

EnhancedFile::EnhancedFile(const EnhancedFile& obj) : fp(NULL),
                                                      allowclose(false)
{
  operator = (obj);
}

EnhancedFile::~EnhancedFile()
{
  fclose();
}

/*--------------------------------------------------------------------------------*/
/** Duplicate file by assignment
 */
/*--------------------------------------------------------------------------------*/
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
    if (strcmp(filename, "stdout") == 0)
    {
      fp             = stdout;
      this->filename = filename;
      this->mode     = "w";
      allowclose     = false;
      success        = true;
    }
    else if (strcmp(filename, "stderr") == 0)
    {
      fp             = stderr;
      this->filename = filename;
      this->mode     = "w";
      allowclose     = false;
      success        = true;
    }
    else if (strcmp(filename, "stdin") == 0)
    {
      fp             = stdin;
      this->filename = filename;
      this->mode     = "r";
      allowclose     = false;
      success        = true;
    }
    else if ((fp = ::fopen(filename, mode)) != NULL)
    {
      DEBUG2(("Opened '%s' for '%s'", filename, mode));
      this->filename = filename;
      this->mode     = mode;
      allowclose     = true;
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
    if (allowclose) ::fclose(fp);
    fp         = NULL;
    allowclose = false;

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

/*--------------------------------------------------------------------------------*/
/** Read a line of text from an open file
 *
 * @param line buffer to receive text
 * @param maxlen maximum number of bytes that 'line' can hold (INCLUDING terminator)
 *
 * @return number of chracters in buffer (excluding terminator), EOF on end of file (with no characters stored)
 */
/*--------------------------------------------------------------------------------*/
int EnhancedFile::readline(char *line, uint_t maxlen)
{
  int l = EOF;

  if (fp)
  {
    uint_t i;
    int    c;   // characters read as int to allow EOF to be detected

    // reduce buffer space by one for terminator
    maxlen--;

    // loop reading characters until EOF or no more space or linefeed character read
    for (i = 0; ((c = fgetc(fp)) != EOF) && (c != '\n');)
    {
      // ignore overspill characters carriage-returns
      if ((i < maxlen) && (c != '\r')) line[i++] = c;
    }

    // add terminator
    line[i] = 0;

    // if any characters stored or last character wasn't an EOF then return line length
    // if no characters stored and the last character read was an EOF, return EOF
    l = (i || (c != EOF)) ? i : EOF;
  }

  return l;
}

BBC_AUDIOTOOLBOX_END

