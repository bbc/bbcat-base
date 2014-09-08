
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

#define DEBUG_LEVEL 1

#include "misc.h"
#include "ByteSwap.h"
#include "ThreadLock.h"

BBC_AUDIOTOOLBOX_START

static std::vector<char *> AllocatedStrings;

static ThreadLockObject debuglock;

static DEBUGHANDLER debughandler = NULL;
static void         *debughandler_context = NULL;

static DEBUGHANDLER errorhandler = NULL;
static void         *errorhandler_context = NULL;

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

    FreeStrings();
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
    if (errorhandler)
    {
      (*errorhandler)(str.c_str(), errorhandler_context);
    }
    else
    {
      fprintf(errstr, "%s\n", str.c_str());
      fflush(errstr);
    }

    FreeStrings();
  }
}

static bool pipe_enabled = false;
void enable_pipe(bool enable)
{
  pipe_enabled = enable;
}

void pipe_msg(const char *fmt, ...)
{
  if (pipe_enabled) {
    va_list     ap;
    std::string str;

    va_start(ap, fmt);
    VPrintf(str, fmt, ap);
    va_end(ap);

    ThreadLock lock(debuglock);
    static ulong_t tick0 = GetTickCount();
    ulong_t tick = GetTickCount() - tick0;

    printf("INFO[%010lu]: %s\n", tick, str.c_str());
    fflush(stdout);

    FreeStrings();
  }
}

bool get_pipe_msg(const char *str, ulong_t& tick, std::string& str2)
{
  bool ispipemsg = false;

  if (sscanf(str, "INFO[%lu]: ", &tick) > 0)
  {
    str2.assign(strstr(str, ": ") + 2);
    ispipemsg = true;
  }

  return ispipemsg;
}

const char *CreateString(const char *data, uint_t len)
{
  char *str;

  if ((str = new char[len + 1]) != NULL)
  {
    memcpy(str, data, len);
    str[len] = 0;

    AllocatedStrings.push_back(str);
  }

  return str;
}

void FreeStrings()
{
  uint_t i;

  for (i = 0; i < AllocatedStrings.size(); i++)
  {
    delete[] AllocatedStrings[i];
  }

  AllocatedStrings.clear();
}

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
  uint_t len = indent.size();
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
        uint_t n = MIN(endpos - pos, pos);

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
  va_start(ap,fmt);

  char *buf = NULL;
  if (vasprintf(&buf, fmt, ap) > 0)
  {
    str += buf;
    free(buf);
  }

  va_end(ap);
}

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
/** Interpolate current towards target at rate coeff, protecting against denormals
 */
/*--------------------------------------------------------------------------------*/
void Interpolate(double& current, double target, double coeff, double limit)
{
  current += (target - current) * coeff;
  if (fabs(target - current) < limit) current = target;
}

const char *DebugStream::eol = "\n";

DebugStream::DebugStream()
{
  clear();
}

DebugStream::~DebugStream()
{
}

DebugStream& DebugStream::operator << (const std::string& str)
{
  ThreadLock lock(debuglock);
  data += str;
  return *this;
}

DebugStream& DebugStream::operator << (const char *str)
{
  ThreadLock lock(debuglock);
  Printf(data, "%s", str);
  return *this;
}

DebugStream& DebugStream::operator << (sint_t n)
{
  ThreadLock lock(debuglock);
  Printf(data, "%d", n);
  return *this;
}

DebugStream& DebugStream::operator << (uint_t n)
{
  ThreadLock lock(debuglock);
  Printf(data, "%u", n);
  return *this;
}

DebugStream& DebugStream::operator << (slong_t n)
{
  ThreadLock lock(debuglock);
  Printf(data, "%ld", n);
  return *this;
}

DebugStream& DebugStream::operator << (ulong_t n)
{
  ThreadLock lock(debuglock);
  Printf(data, "%lu", n);
  return *this;
}

DebugStream& DebugStream::operator << (sllong_t n)
{
  ThreadLock lock(debuglock);
  Printf(data, "%lld", n);
  return *this;
}

DebugStream& DebugStream::operator << (ullong_t n)
{
  ThreadLock lock(debuglock);
  Printf(data, "%llu", n);
  return *this;
}

const char *DebugStream::get() const
{
  ThreadLock lock(debuglock);
  return data.c_str();
}

void DebugStream::clear()
{
  ThreadLock lock(debuglock);
  data = "";
}

DebugStream dbg;

void debug_msg(DebugStream& str)
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

  FreeStrings();
  str.clear();
}

void debug_err(DebugStream& str)
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

  FreeStrings();
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
  res += 1.0e-31;
  res -= 1.0e-31;

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

BBC_AUDIOTOOLBOX_END
