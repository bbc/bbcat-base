
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>

#ifdef __MACH__
#include <mach/mach_time.h>
#endif

#include <vector>

#define BBCDEBUG_LEVEL 1

#include "misc.h"
// explicit use of current directory's ByteSwap.h
#include "./ByteSwap.h"
#include "ThreadLock.h"
#include "EnhancedFile.h"

BBC_AUDIOTOOLBOX_START

static ThreadLockObject debuglock;

static DEBUGHANDLER debughandler = NULL;
static void         *debughandler_context = NULL;

static DEBUGHANDLER errorhandler = NULL;
static void         *errorhandler_context = NULL;

const char *DoubleFormatHuman = "0.32";
const char *DoubleFormatExact = "x";

/*--------------------------------------------------------------------------------*/
/** Set debug handler (replacing printf())
 *
 * @param handler debug handler
 * @param context optional parameter to pass to handler
 *
 */
/*--------------------------------------------------------------------------------*/
void SetDebugHandler(DEBUGHANDLER handler, void *context)
{
  ThreadLock lock(debuglock);

  debughandler = handler;
  debughandler_context = context;
}

/*--------------------------------------------------------------------------------*/
/** Set error handler (replacing printf())
 *
 * @param handler error handler
 * @param context optional parameter to pass to handler
 *
 */
/*--------------------------------------------------------------------------------*/
void SetErrorHandler(DEBUGHANDLER handler, void *context)
{
  ThreadLock lock(debuglock);

  errorhandler = handler;
  errorhandler_context = context;
}

/*--------------------------------------------------------------------------------*/
/** By default, errors are NOT logged to a file
 */
/*--------------------------------------------------------------------------------*/
static bool error_logging_enabled = false;

/*--------------------------------------------------------------------------------*/
/** Enable logging to file (in a file returned by GetErrorLoggingFile())
 */
/*--------------------------------------------------------------------------------*/
void EnableErrorLogging(bool enable)
{
  error_logging_enabled = enable;
}

/*--------------------------------------------------------------------------------*/
/** Return filename used to log errors to
 */
/*--------------------------------------------------------------------------------*/
const std::string& GetErrorLoggingFile()
{
  static const std::string filename = EnhancedFile::catpath(getenv("HOME"), "bbcat-errors.txt").c_str();
  return filename;
}

void debug_msg(const char *fmt, ...)
{
  va_list     ap;
  std::string str;

  va_start(ap, fmt);
  VPrintf(str, fmt, ap);
  va_end(ap);

  {
    ThreadLock lock(debuglock);
    if (debughandler)
    {
      (*debughandler)(str.c_str(), debughandler_context);
    }
    else
    {
      printf("%s\n", str.c_str());
      fflush(stdout);
    }
  }
}

void debug_err(const char *fmt, ...)
{
  FILE *errstr = stdout;
  va_list     ap;
  std::string str;

  va_start(ap, fmt);
  VPrintf(str, fmt, ap);
  va_end(ap);

  {
    ThreadLock lock(debuglock);
    static bool _within = false;        // protect against recursive calls
    bool        within  = _within;

    _within = true;

    // MUST NOT allow this section to be called recursively!
    if (!within && error_logging_enabled)
    {
      static EnhancedFile file(GetErrorLoggingFile().c_str(), "w");

      file.fprintf("%s\n", str.c_str());
      file.fflush();
    }
    
    if (errorhandler)
    {
      (*errorhandler)(str.c_str(), errorhandler_context);
    }
    else
    {
      fprintf(errstr, "%s\n", str.c_str());
      fflush(errstr);
    }

    _within = false;
  }
}

#ifndef TARGET_OS_WINDOWS
ulong_t GetTickCount()
{
#ifdef __MACH__
  static mach_timebase_info_data_t timebase;
  static bool inited = false;

  if (!inited)
  {
    mach_timebase_info(&timebase);
    inited = true;
  }

  uint64_t tick = mach_absolute_time();
  tick = (tick * timebase.numer) / timebase.denom;

  return (ulong_t)(tick / 1000000);
#else
  struct timespec timespec;

#ifdef ANDROID
  clock_gettime(CLOCK_MONOTONIC_HR, &timespec);
#elif defined(__CYGWIN__)
  clock_gettime(CLOCK_MONOTONIC, &timespec);
#else
  clock_gettime(CLOCK_MONOTONIC_RAW, &timespec);
#endif

  return timespec.tv_sec * 1000UL + (timespec.tv_nsec / 1000000UL);
#endif
}
#endif

/*--------------------------------------------------------------------------------*/
/** Multiply 64-bit unsigned integer by 32-bit fraction without overflow
 */
/*--------------------------------------------------------------------------------*/
uint64_t muldiv(uint64_t val, uint32_t mul, uint32_t div)
{
  // split val into 32-bit parts for multiplying
  uint32_t parts[] = {(uint32_t)val, (uint32_t)(val >> 32)};

  // result (3 x 32 bits)
  uint32_t res[NUMBEROF(parts) + 1];    // maximum number of 32-bit parts required for product

  uint64_t carry = 0;                   // accumulator
  uint_t i;
  
  // first, multiply up by multiplier
  for (i = 0; i < NUMBEROF(parts); i++)
  {
    carry += mul * (uint64_t)parts[i];  // 64-bit multiply of 32-bit parts
    res[i] = (uint32_t)carry;           // save lower 32-bits of result
    carry >>= 32;                       // and shift down to carry to next stage
  }
  // save final value
  res[i] = (uint32_t)carry;

  BBCDEBUG2(("Mul-result:%08lx:%08lx:%08lx", (ulong_t)res[2], (ulong_t)res[1], (ulong_t)res[0]));
  
  // now divide by divider
  carry = 0;
  for (i = NUMBEROF(res); i > 0; /* i pre-decremented below */)
  {
    carry <<= 32;                           // shift carry up
    carry  += res[--i];                     // add value
    res[i]  = (uint32_t)(carry / div);      // store result of divide
    carry  %= div;                          // save remainder
  }

  BBCDEBUG2(("Div-result:%08lx:%08lx:%08lx", (ulong_t)res[2], (ulong_t)res[1], (ulong_t)res[0]));

  return res[0] + ((uint64_t)res[1] << 32);
}

/*--------------------------------------------------------------------------------*/
/** Return machine time on in nanoseconds
 */
/*--------------------------------------------------------------------------------*/
uint64_t GetNanosecondTicks()
{
#ifdef TARGET_OS_WINDOWS
  static uint32_t div = 0;
  LARGE_INTEGER time;

  // find divider if not known
  if (!div)
  {
    LARGE_INTEGER freq;
    
    // calculate divisor to get from ns from performance counter
    QueryPerformanceFrequency(&freq);

    // assume divider is 32-bits or less
    div = (uint32_t)freq.QuadPart;
  }
  
  QueryPerformanceCounter(&time);

  // multiply time by 1e9 (s->ns) then divide by divisor
  return muldiv(time.QuadPart, 1000000000, div);
#elif __MACH__
  static mach_timebase_info_data_t timebase;
  static bool inited = false;

  if (!inited)
  {
    mach_timebase_info(&timebase);
    inited = true;
  }

  // scale by timebase.numer / timebase.denom which results in nano-seconds
  return muldiv(mach_absolute_time(), timebase.numer, timebase.denom);
#else
  struct timespec timespec;

#ifdef ANDROID
  clock_gettime(CLOCK_MONOTONIC_HR, &timespec);
#elif defined(__CYGWIN__)
  clock_gettime(CLOCK_MONOTONIC, &timespec);
#else
  clock_gettime(CLOCK_MONOTONIC_RAW, &timespec);
#endif

  return (uint64_t)timespec.tv_sec * 1000000000ULL + (uint64_t)timespec.tv_nsec;
#endif
}

uint32_t IEEEExtendedToINT32u(const IEEEEXTENDED *num)
{
  /* Format of 80-bit IEEE floating point number is:
   *
   * sign(1).exponent(15).mantissa(64)
   *
   * NOTE: mantissa has explicit 1
   */

  const uint8_t *p = num->b;
  int16_t  expo = (int16_t)(((uint16_t)(p[0] & 0x7f) << 8) | (uint16_t)p[1]) - 16383;
  uint64_t mant;
  uint32_t val;

  /* generate mantissa... */
  memcpy(&mant, p + 2, sizeof(mant));
  /* ...and swap bytes if necessary */
  if (!MACHINE_IS_BIG_ENDIAN) BYTESWAP_VAR(mant);

  /* mantissa has decimal point between bits 63 and 62
   * whereas we want it between bits 32 and 31 so we need
   * to bias expo by 31 bits */
  expo -= 31;

  /* shift mantissa appropriately */
  if      (expo < 0) mant >>= -expo;
  else if (expo > 0) mant <<=  expo;

  /* round 64-bit mantissa (which is now 32.32) and return integer part */
  val = (uint32_t)((mant + 0x80000000) >> 32);

  return val;
}

/*------------------------------------------------------------
  Function: Convert uint32_t to IEEE Extended
  ----------------------------------------------------------*/
void INT32uToIEEEExtended(uint32_t val, IEEEEXTENDED *num)
{
  int16_t expo = 0;
  int64_t mant = 0;

  memset(num, 0, sizeof(*num));

  while (!(val & 0x80000000))
  {
    val <<= 1;
    expo--;
  }

  mant  = (uint64_t)val << 32;
  expo += 31 + 16383;

  if (!MACHINE_IS_BIG_ENDIAN)
  {
    BYTESWAP_VAR(expo);
    BYTESWAP_VAR(mant);
  }

  memcpy(num->b, &expo, sizeof(expo));
  num->b[0] &= 0x7f;
  memcpy(num->b + 2, &mant, sizeof(mant));
}

std::string CreateIndent(const std::string& indent, uint_t count)
{
  uint_t len = (uint_t)indent.size();
  std::string str;
  char *buf;

  if ((count * len) > 0)
  {
    if ((buf = new char[count * len]) != NULL)
    {
      uint_t pos = 0, endpos = count * len;

      memcpy(buf, indent.c_str(), len);

      for (pos = len; pos < endpos;)
      {
        uint_t n = std::min(endpos - pos, pos);

        memcpy(buf + pos, buf, n);
        pos += n;
      }

      str.assign(buf, pos);
    }
  }

  return str;
}

/*--------------------------------------------------------------------------------*/
/** Printf for std::string
 *
 * @param str string to be added to
 * @param fmt printf-style format information
 *
 */
/*--------------------------------------------------------------------------------*/
void Printf(std::string& str, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  VPrintf(str, fmt, ap);
  va_end(ap);
}

#ifdef COMPILER_MSVC
/*--------------------------------------------------------------------------------*/
/** vasprintf doesn't exist in Windows
 */
/*--------------------------------------------------------------------------------*/
int vasprintf(char **buf, const char *fmt, va_list ap)
{
  *buf = NULL;
    
  int l = vsnprintf(NULL, 0, fmt, ap);
  if (l >= 0) {
    if ((*buf = (char *)malloc(l + 1)) != NULL)
    {
      l = vsnprintf(*buf, l + 1, fmt, ap);
    }
  }
  return l;
}
#endif

/*--------------------------------------------------------------------------------*/
/** vprintf for std::string
 *
 * @param str string to be added to
 * @param fmt printf-style format information
 * @param ap ap_list of arguments
 *
 */
/*--------------------------------------------------------------------------------*/
void VPrintf(std::string& str, const char *fmt, va_list ap)
{
  char *buf = NULL;
  if (vasprintf(&buf, fmt, ap) > 0)
  {
    str += buf;
    free(buf);
  }
}


/*--------------------------------------------------------------------------------*/
/** Split a string by a delimiter, allowing for quotes to prevent splitting in the wrong place
 *
 * @param str string split
 * @param list list to be populated
 * @param delim delimiter character
 * @param keepempty true to keep empty strings
 * @param maxstrings if non-zero specifies the maximum number of entries in list
 *
 * @return position in string when scanning stopped
 * 
 * @note whitespace is IGNORED!
 */
/*--------------------------------------------------------------------------------*/
uint_t SplitString(const std::string& str, std::vector<std::string>& list, char delim, bool keepempty, uint_t maxstrings)
{
  uint_t p = 0, l = (uint_t)str.length();

  while ((p < l) && (!maxstrings || (list.size() < maxstrings)))
  {
    // ignore whitespace before string
    while ((p < l) && ((str[p] == ' ') || (str[p] == '\t'))) p++;

    // detect opening quote
    char quote = ((str[p] == '\'') || (str[p] == '\"')) ? str[p] : 0;
    
    // skip quote, if any
    if (quote) p++;

    // save string start
    uint_t p1 = p;

    // advance until either end of string, delimiter is found or closing quote is found
    while ((p < l) && ((!quote && (str[p] != delim)) || (quote && (str[p] != quote)))) p++;

    // save string end
    uint_t p2 = p;

    // if quotes were *not* used, move string end back over any whitespace
    if (!quote)
    {
      while ((p2 > p1) && ((str[p2 - 1] == ' ') || (str[p2 - 1] == '\t'))) p2--;
    }

    // if string is not empty (or keepempty is true), add it to the list
    if (keepempty || (p2 > p1)) list.push_back(str.substr(p1, p2 - p1));

    // if a closing quote was found, skip quote and then find delimiter
    if ((p < l) && quote && (str[p] == quote))
    {
      p++;

      while ((p < l) && (str[p] != delim)) p++;
    }

    // skip over delimiters
    while ((p < l) && (str[p] == delim)) p++;

    // ignore whitespace at end
    while ((p < l) && ((str[p] == ' ') || (str[p] == '\t'))) p++;
  }

  return p;
}

/*--------------------------------------------------------------------------------*/
/** Interpolate current towards target at rate coeff, protecting against denormals
 */
/*--------------------------------------------------------------------------------*/
void Interpolate(double& current, double target, double coeff, double limit)
{
  current += (target - current) * coeff;
  if (fabs(target - current) < limit) current = target;
}

const char *StringStream::eol = "\n";

StringStream::StringStream()
{
  clear();
}

StringStream::~StringStream()
{
}

StringStream& StringStream::operator << (const std::string& str)
{
  data += str;
  return *this;
}

StringStream& StringStream::operator << (const char *str)
{
  Printf(data, "%s", str);
  return *this;
}

StringStream& StringStream::operator << (sint_t n)
{
  Printf(data, "%d", n);
  return *this;
}

StringStream& StringStream::operator << (uint_t n)
{
  Printf(data, "%u", n);
  return *this;
}

StringStream& StringStream::operator << (slong_t n)
{
  Printf(data, "%ld", n);
  return *this;
}

StringStream& StringStream::operator << (ulong_t n)
{
  Printf(data, "%lu", n);
  return *this;
}

StringStream& StringStream::operator << (sllong_t n)
{
  Printf(data, "%lld", n);
  return *this;
}

StringStream& StringStream::operator << (ullong_t n)
{
  Printf(data, "%llu", n);
  return *this;
}

const char *StringStream::get() const
{
  return data.c_str();
}

void StringStream::clear()
{
  data = "";
}

void debug_msg(StringStream& str)
{
  ThreadLock lock(debuglock);
  const char *p = str.get();
  if (debughandler)
  {
    (*debughandler)(p, debughandler_context);
  }
  else
  {
    printf("%s\n", p);
    fflush(stdout);
  }

  str.clear();
}

void debug_err(StringStream& str)
{
  ThreadLock lock(debuglock);
  const char *p = str.get();
  if (errorhandler)
  {
    (*errorhandler)(p, errorhandler_context);
  }
  else
  {
    printf("%s\n", p);
    fflush(stdout);
  }

  str.clear();
}

/*--------------------------------------------------------------------------------*/
/** Prevent value becoming denormalized (bad for performance!)
 */
/*--------------------------------------------------------------------------------*/
float fix_denormal(float val)
{
  volatile float res = val;     // volatile here means 'don't optimize me!'

  // by adding and subtracting a small value, if the value is significantly less than the small value
  // the result will become zero instead of being a denormalized number
  res += 1.0e-31f;
  res -= 1.0e-31f;

  return res;
}

double fix_denormal(double val)
{
  volatile double res = val;    // volatile here means 'don't optimize me!'

  // by adding and subtracting a small value, if the value is significantly less than the small value
  // the result will become zero instead of being a denormalized number
  res += 1.0e-291;
  res -= 1.0e-291;

  return res;
}

/*--------------------------------------------------------------------------------*/
/** Factorial of an unsigned integer.
 *
 */
/*--------------------------------------------------------------------------------*/
uint_t factorial(uint_t n)
{
  return (n == 0) ? 1 : (factorial(n-1)*n);
}

/*--------------------------------------------------------------------------------*/
/** Add a list of parameters/controls to a list
 */
/*--------------------------------------------------------------------------------*/
void AddParametersToList(const PARAMETERDESC *parameters, uint_t n, std::vector<const PARAMETERDESC *>& list)
{
  uint_t i;

  for (i = 0; i < n; i++) list.push_back(parameters + i);
}

static const double mindb = -120.0;

/*--------------------------------------------------------------------------------*/
/** Convert dB to gain assuming <=-120 is cut
 */
/*--------------------------------------------------------------------------------*/
double dBToGain(double db)
{
  // treat -120 as cut
  if (db > mindb) return pow(10.0, .05 * db);
  return 0.0;
}

/*--------------------------------------------------------------------------------*/
/** Convert gain to dB to gain (-120 is the minimum, including cut)
 */
/*--------------------------------------------------------------------------------*/
double GainTodB(double gain)
{
  static const double mingain = pow(10.0, .05 * mindb);  // minimum value
  double db = mindb;

  gain = fabs(gain);
  if (gain >= mingain)
  {
    db = 20.0 * log10(gain);
    db = std::max(db, mindb);
  }

  return db;
}

/*--------------------------------------------------------------------------------*/
/** Attempt to evaluate values from strings
 */
/*--------------------------------------------------------------------------------*/
bool Evaluate(const std::string& str, bool& val)
{
  bool success = false;
  uint_t n;

  if (sscanf(str.c_str(), "%u", &n) > 0)
  {
    val = (n != 0);
    success = true;
  }
  else if (str == "true")
  {
    val = true;
    success = true;
  }
  else if (str == "false")
  {
    val = false;
    success = true;
  }

  return success;
}

bool Evaluate(const std::string& str, sint_t& val, bool hex)
{
  uint_t hexoffset = ((str[0] == '$') || (str[0] == '#'));
  return (sscanf(str.c_str() + hexoffset, (hexoffset || hex) ? "%x" : "%d", &val) > 0);
}

bool Evaluate(const std::string& str, uint_t& val, bool hex)
{
  uint_t hexoffset = ((str[0] == '$') || (str[0] == '#'));
  return (sscanf(str.c_str() + hexoffset, (hexoffset || hex) ? "%x" : "%d", &val) > 0);
}

bool Evaluate(const std::string& str, slong_t& val, bool hex)
{
  uint_t hexoffset = ((str[0] == '$') || (str[0] == '#'));
  return (sscanf(str.c_str() + hexoffset, (hexoffset || hex) ? "%lx" : "%ld", &val) > 0);
}

bool Evaluate(const std::string& str, ulong_t& val, bool hex)
{
  uint_t hexoffset = ((str[0] == '$') || (str[0] == '#'));
  return (sscanf(str.c_str() + hexoffset, (hexoffset || hex) ? "%lx" : "%lu", &val) > 0);
}

bool Evaluate(const std::string& str, sllong_t& val, bool hex)
{
  uint_t hexoffset = ((str[0] == '$') || (str[0] == '#'));
  return (sscanf(str.c_str() + hexoffset, (hexoffset || hex) ? "%llx" : "%lld", &val) > 0);
}

bool Evaluate(const std::string& str, ullong_t& val, bool hex)
{
  uint_t hexoffset = ((str[0] == '$') || (str[0] == '#'));
  return (sscanf(str.c_str() + hexoffset, (hexoffset || hex) ? "%llx" : "%llu", &val) > 0);
}

bool Evaluate(const std::string& str, float& val)
{
  // evaluate as double and convert
  double dval;
  bool   success = Evaluate(str, dval);
  if (success) val = (float)dval;
  return success;
}

bool Evaluate(const std::string& str, double& val)
{
  // values starting with a '#' are a 64-bit hex representation of the double value
  // otherwise try to scan the string as a double
  if (str[0] == '#') return Evaluate(str, *(uint64_t *)&val);
  else               return (sscanf(str.c_str(), "%lf", &val) > 0);
}

bool Evaluate(const std::string& str, std::string& val)
{
  val = str;
  return true;
}

/*--------------------------------------------------------------------------------*/
/** Generate full format string from supplied format string
 *
 * Essentially:
 *  if no type specifier is specified, a default type based on the supplied variable type is appended
 *  the correct number of 'l's are inserted before the type specifier for the supplied variable type
 */
/*--------------------------------------------------------------------------------*/
const char *GetFormat(char *fmt, const char *_fmt, const char *insert, const char *defsuffix)
{
  // if original is fully specified, just return it
  if (_fmt[0] == '%') return _fmt;
  
  size_t p = 0, l = strlen(_fmt);   // length of supplied format string

  // if last char of supplied format string is alpha, decrement length by 1
  if (l && (limited::inrange(_fmt[l - 1], 'a', 'z') || limited::inrange(_fmt[l - 1], 'A', 'Z'))) l--;

  // set suffix to point to last char (if alpha) or end of string (otherwise)
  const char *suffix = _fmt + l;
  // if no suffix supplied, set it to default suffix
  if (!suffix[0]) suffix = defsuffix;
  
  // start full format string
  fmt[p++] = '%';
  // copy supplied format string (sans last char if it is alpha)
  memcpy(fmt + p, _fmt, l); p += l;
  // add string of 'l's as necessary to format string
  strcpy(fmt + p, insert);
  // then add suffix
  strcat(fmt, suffix);

  BBCDEBUG9(("GetFormat('%s', '%s', '%s') = '%s'", _fmt, insert, defsuffix, fmt));

  return fmt;
}

/*--------------------------------------------------------------------------------*/
/** Convert various types to strings
 *
 * @param fmt format specifier (WITHOUT 'l's) and optionally without types
 *
 * @note example value fmt values:
 *  "" default display ('d', 'u' or 'f'), no field formatting
 *  "x" hex display 
 *  "016x" hex display with field size of 16, '0' padded 
 *  "0.32" default display ('f') with field size of 32
 *
 * Essentially:
 *  if no type specifier is specified, a default type based on the supplied variable type is appended
 *  the correct number of 'l's are inserted before the type specifier for the supplied variable type
 */
/*--------------------------------------------------------------------------------*/
std::string StringFrom(bool val)
{
  std::string str;
  Printf(str, "%u", val ? 1 : 0);
  return str;
}

std::string StringFrom(sint_t val, const char *fmt)
{
  std::string str;
  char tmpfmt[16];
  Printf(str, GetFormat(tmpfmt, fmt, "", "d"), val);  
  return str;
}

std::string StringFrom(uint_t val, const char *fmt)
{
  std::string str;
  char tmpfmt[16];
  Printf(str, GetFormat(tmpfmt, fmt, "", "u"), val);  
  return str;
}

std::string StringFrom(slong_t val, const char *fmt)
{
  std::string str;
  char tmpfmt[16];
  Printf(str, GetFormat(tmpfmt, fmt, "l", "d"), val);  
  return str;
}

std::string StringFrom(ulong_t val, const char *fmt)
{
  std::string str;
  char tmpfmt[16];
  Printf(str, GetFormat(tmpfmt, fmt, "l", "u"), val);  
  return str;
}

std::string StringFrom(sllong_t val, const char *fmt)
{
  std::string str;
  char tmpfmt[16];
  Printf(str, GetFormat(tmpfmt, fmt, "ll", "d"), val);  
  return str;
}

std::string StringFrom(ullong_t val, const char *fmt)
{
  std::string str;
  char tmpfmt[16];
  Printf(str, GetFormat(tmpfmt, fmt, "ll", "u"), val);  
  return str;
}

std::string StringFrom(float val, const char *fmt)
{
  // use double version
  return StringFrom((double)val, fmt);
}

std::string StringFrom(double val, const char *fmt)
{
  std::string str;
  size_t l;
  
  if (((l = strlen(fmt)) > 0) && (fmt[l - 1] == 'x'))
  {
    uint64_t uval;
    memcpy(&uval, &val, sizeof(uval));
    str = "#" + StringFrom(uval, "016x");
  }
  else
  {
    char tmpfmt[16];
    Printf(str, GetFormat(tmpfmt, fmt, "l", "f"), val);
  }
  return str;
}

std::string StringFrom(const std::string& val)
{
  return val;
}

std::string StringFrom(const void *val)
{
  return "$" + StringFrom((uint64_t)val, (sizeof(val) == 4) ? "08x" : "016x");
}

/*--------------------------------------------------------------------------------*/
/** Bog-standard string search and replace that *should* be in std::string!
 */
/*--------------------------------------------------------------------------------*/
std::string SearchAndReplace(const std::string& str, const std::string& search, const std::string& replace)
{
  std::string res = str;
  size_t p = 0;

  // make sure search doesn't end up in an endless loop by
  // gradually moving the search start through the string
  while ((p = res.find(search, p)) < std::string::npos)
  {
    res = res.substr(0, p) + replace + res.substr(p + search.length());
    p  += replace.length();     // start searching just after replacement
  }

  return res;
}

/*--------------------------------------------------------------------------------*/
/** Very simple wildcard matching
 *
 * @param pat pattern containing characters and/or '*' / '?' 
 * @param str string to match
 *
 * @return true if string matches pattern
 *
 * @note this routine is highly recursive!
 */
/*--------------------------------------------------------------------------------*/
bool matchstring(const char *pat, const char *str)
{
  if      (!pat[0])       return !str[0];
  else if (!str[0])       return !pat[0];
  else if (pat[0] == '*') return (matchstring(pat, str + 1) || matchstring(pat + 1, str + 1));
  else if (pat[0] == '?') return matchstring(pat + 1, str + 1);

  return ((pat[0] == str[0]) && matchstring(pat + 1, str + 1));
}

#if ENABLE_JSON
bool FromJSON(const json_spirit::mValue& _val, bool& val)
{
  bool success = false;
  if (_val.type() == json_spirit::bool_type)
  {
    val = _val.get_bool();
    success = true;
  }
  else if (_val.type() == json_spirit::int_type)
  {
    val = (_val.get_int() != 0);
    success = true;
  }
  return success;
}

bool FromJSON(const json_spirit::mValue& _val, sint_t& val)
{
  bool success = (_val.type() == json_spirit::int_type);
  if (success) val = _val.get_int();
  return success;
}

bool FromJSON(const json_spirit::mValue& _val, uint_t& val)
{
  bool success = (_val.type() == json_spirit::int_type);
  if (success) val = std::max(_val.get_int(), 0);
  return success;
}

bool FromJSON(const json_spirit::mValue& _val, sint64_t& val)
{
  bool success = (_val.type() == json_spirit::int_type);
  if (success) val = _val.get_int64();
  return success;
}

bool FromJSON(const json_spirit::mValue& _val, uint64_t& val)
{
  bool success = (_val.type() == json_spirit::int_type);
  if (success) val = std::max(_val.get_int64(), (sint64_t)0);
  return success;
}

bool FromJSON(const json_spirit::mValue& _val, float& val)
{
  double dval;
  bool success = FromJSON(_val, dval);
  if (success) val = (float)dval;
  return success;
}

bool FromJSON(const json_spirit::mValue& _val, double& val)
{
  bool success = false;
  if (_val.type() == json_spirit::real_type)
  {
    val = _val.get_real();
    success = true;
  }
  else if (_val.type() == json_spirit::int_type)
  {
    val = (double)_val.get_int();
    success = true;
  }
  return success;
}

bool FromJSON(const json_spirit::mValue& _val, std::string& val)
{
  bool success = (_val.type() == json_spirit::str_type);
  if (success) val = _val.get_str();
  return success;
}

json_spirit::mValue ToJSON(const bool& val)
{
  return json_spirit::mValue(val);
}

json_spirit::mValue ToJSON(const sint_t& val)
{
  return json_spirit::mValue(val);
}

json_spirit::mValue ToJSON(const uint_t& val)
{
  return json_spirit::mValue((sint_t)val);
}

json_spirit::mValue ToJSON(const sint64_t& val)
{
  return json_spirit::mValue(val);
}

json_spirit::mValue ToJSON(const uint64_t& val)
{
  return json_spirit::mValue((sint64_t)val);
}

json_spirit::mValue ToJSON(const float& val)
{
  return json_spirit::mValue(val);
}

json_spirit::mValue ToJSON(const double& val)
{
  return json_spirit::mValue(val);
}

json_spirit::mValue ToJSON(const std::string& val)
{
  return json_spirit::mValue(val);
}
#endif

/*--------------------------------------------------------------------------------*/
/** Return ns-resolution time from textual [[hh:]mm:]ss.SSSSS
 *
 * @param str time in ASCII format
 *
 * @return ns time
 */
/*--------------------------------------------------------------------------------*/
bool CalcTime(uint64_t& t, const std::string& str)
{
  double secs = 0.0;
  uint_t hrs = 0, mins = 0;
  int n;

  if      ((n = sscanf(str.c_str(), "%u:%u:%lf", &hrs, &mins, &secs)) == 3) ;                 // do nothing
  else if ((n = sscanf(str.c_str(), "%u:%lf",    &mins, &secs))       == 2) hrs = 0;          // MUST zero this here because it will have be set by the line above!
  else if ((n = sscanf(str.c_str(), "%lf",       &secs))              == 1) hrs = mins = 0;   // MUST zero these here because they will have be set by the lines above!

  // has a valid time been extracted?
  if (n > 0)
  {
    BBCDEBUG2(("Converted time '%s' to %u:%u:%0.3lf", str.c_str(), hrs, mins, secs));

    t = (uint64_t)(secs * 1.0e9) + (uint64_t)(hrs * 60UL + mins) * 60UL * 1000UL * 1000000UL;
  }
  else BBCERROR("Unable to extract time from '%s'", str.c_str());

  return (n > 0);
}

/*--------------------------------------------------------------------------------*/
/** Convert ns time to text time
 *
 * @param t ns time
 *
 * @return str time in text format 'hh:mm:ss.SSSSS'
 */
/*--------------------------------------------------------------------------------*/
std::string GenerateTime(uint64_t t)
{
  std::string str;
  uint_t hr, mn, s, ss;

  t /= 10000;
  ss = (uint_t)(t % 100000);
  t /= 100000;
  s  = (uint_t)(t % 60);
  t /= 60;
  mn = (uint_t)(t % 60);
  t /= 60;
  hr = (uint_t)t;

  Printf(str, "%02u:%02u:%02u.%05u", hr, mn, s, ss);

  return str;
}

BBC_AUDIOTOOLBOX_END
