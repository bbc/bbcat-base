#ifndef __MISC__
#define __MISC__

#include <stdint.h>
#include <stdarg.h>

#include <vector>
#include <string>

#include "OSCompiler.h"

#ifdef TARGET_OS_WINDOWS
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include "windows.h"
#endif

#if ENABLE_JSON
#include <json_spirit/json_spirit.h>
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef LIMIT
#define LIMIT(a, b, c) (((a) < (b)) ? (b) : (((a) > (c)) ? (c) : (a)))
#endif
#ifndef NUMBEROF
#define NUMBEROF(x) (sizeof(x) / sizeof(x[0]))
#endif
#ifndef RANGE
#define RANGE(a, b, c) (((a) >= (b)) && ((b) <= (c)))
#endif

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884
#endif

#define IFFID(name) (((uint32_t)name[0] << 24) | ((uint32_t)name[1] << 16) | ((uint32_t)name[2] << 8) | (uint32_t)name[3])

#define UNUSED_PARAMETER(name) ((void)(name))

#define USE_BBC_AUDIOTOOLBOX using namespace bbcat;
#define BBC_AUDIOTOOLBOX_START namespace bbcat {
#define BBC_AUDIOTOOLBOX_END }

typedef int16_t       sint16_t;
typedef int32_t       sint32_t;
typedef int64_t       sint64_t;
typedef signed   int  sint_t;
typedef unsigned int  uint_t;
typedef signed   long slong_t;
typedef unsigned long ulong_t;
typedef signed   long long sllong_t;
typedef unsigned long long ullong_t;

#ifdef TARGET_OS_WINDOWS
#include "Windows_uSleep.h"
#endif

#ifdef GCC_BUILD
#define PACKEDSTRUCT   struct __attribute__ ((packed))
#define PRINTF_FORMAT  __attribute__ ((format (printf,1,2)))
#define PRINTF_FORMAT2 __attribute__ ((format (printf,2,3)))
#else
#define PACKEDSTRUCT   struct
#define PRINTF_FORMAT
#define PRINTF_FORMAT2
#endif

BBC_AUDIOTOOLBOX_START

#ifdef __BYTE_ORDER__
// endianness can be determined at compile time
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
// use #define to allow compiler to optimize
#define MACHINE_IS_BIG_ENDIAN false
#endif

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
// use #define to allow compiler to optimize
#define MACHINE_IS_BIG_ENDIAN true
#endif
#else
// endianness cannot be determined at compile time, determine at run-time
// defined in ByteSwap.cpp
extern const bool MACHINE_IS_BIG_ENDIAN;
#endif

/*--------------------------------------------------------------------------------*/
/** Static library linking causes objects not explicitly referenced to be STRIPPED from the final executable
 *
 * This causes problems for some auto-registering mechanisms
 *
 * Therefore, any objects files in this category MUST include:
 * 1.  BBC_AUDIOTOOLBOX_KEEP(<id>) in the LIBRARY .c/.cpp file
 * 2.  BBC_AUDIOTOOLBOX_REQUIRE(<id>) in an APPLICATION .c/.cpp file
 */
/*--------------------------------------------------------------------------------*/
#define BBC_AUDIOTOOLBOX_KEEP(x) volatile uint_t __keep_##x = 0
#define BBC_AUDIOTOOLBOX_REQUIRE(x)                 \
BBC_AUDIOTOOLBOX_START                              \
extern volatile uint_t __keep_##x;                  \
volatile uint_t __keep_##x##_in_app = __keep_##x;   \
BBC_AUDIOTOOLBOX_END

typedef void (*DEBUGHANDLER)(const char *str, void *context);

/*--------------------------------------------------------------------------------*/
/** Enable logging to file (in a file returned by GetErrorLoggingFile())
 */
/*--------------------------------------------------------------------------------*/
extern void EnableErrorLogging(bool enable = true);

/*--------------------------------------------------------------------------------*/
/** Return filename used to log errors to
 */
/*--------------------------------------------------------------------------------*/
extern const std::string& GetErrorLoggingFile();

/*--------------------------------------------------------------------------------*/
/** These are called by macros and do not need to be called explicitly
 */
/*--------------------------------------------------------------------------------*/
extern void debug_msg(const char *fmt, ...) PRINTF_FORMAT;
extern void debug_err(const char *fmt, ...) PRINTF_FORMAT;

#define BBCERROR debug_err
#define BBCDEBUG debug_msg

/*--------------------------------------------------------------------------------*/
/** Debug output control
 *
 * Before including any include files from bbcat-*, #define BBCDEBUG_LEVEL to a value
 *
 * The value dictates which debug informatin (in that file) is output:
 * BBCDEBUG_LEVEL  Debug output
 * 0            BBCERROR() and BBCDEBUG() ONLY
 * 1            As above plus BBCDEBUG1(())
 * 2            As above plus BBCDEBUG2(())
 * 3            As above plus BBCDEBUG3(())
 * 4            As above plus BBCDEBUG4(())
 * 5            As above plus BBCDEBUG5(())
 *
 * Note: the double-parentheses are required because they allow debug lines that are
 *       NOT going to be outputted to be converted to (void)0 statements (unlike
 *       lower level controls which will evaluate the entrie string and arguments
 *       and then throw it away)
 *
 * If BBCDEBUG_LEVEL is not defined, it is set to 0
 */
/*--------------------------------------------------------------------------------*/
#ifndef BBCDEBUG_LEVEL
#define BBCDEBUG_LEVEL 0
#endif

#if BBCDEBUG_LEVEL >= 1
#define BBCDEBUG1(x) debug_msg x
#else
#define BBCDEBUG1(x) (void)0
#endif

#if BBCDEBUG_LEVEL >= 2
#define BBCDEBUG2(x) debug_msg x
#else
#define BBCDEBUG2(x) (void)0
#endif

#if BBCDEBUG_LEVEL >= 3
#define BBCDEBUG3(x) debug_msg x
#else
#define BBCDEBUG3(x) (void)0
#endif

#if BBCDEBUG_LEVEL >= 4
#define BBCDEBUG4(x) debug_msg x
#else
#define BBCDEBUG4(x) (void)0
#endif

#if BBCDEBUG_LEVEL >= 5
#define BBCDEBUG5(x) debug_msg x
#else
#define BBCDEBUG5(x) (void)0
#endif

#if BBCDEBUG_LEVEL >= 6
#define BBCDEBUG6(x) debug_msg x
#else
#define BBCDEBUG6(x) (void)0
#endif

#if BBCDEBUG_LEVEL >= 7
#define BBCDEBUG7(x) debug_msg x
#else
#define BBCDEBUG7(x) (void)0
#endif

#if BBCDEBUG_LEVEL >= 8
#define BBCDEBUG8(x) debug_msg x
#else
#define BBCDEBUG8(x) (void)0
#endif

#if BBCDEBUG_LEVEL >= 9
#define BBCDEBUG9(x) debug_msg x
#else
#define BBCDEBUG9(x) (void)0
#endif

// fundamental audio processing unit - don't change without careful consideration!
typedef float Sample_t;

typedef PACKEDSTRUCT
{
  uint8_t b[10];
} IEEEEXTENDED;

#ifndef TARGET_OS_WINDOWS
/*--------------------------------------------------------------------------------*/
/** Return machine time on in milliseconds
 */
/*--------------------------------------------------------------------------------*/
extern ulong_t  GetTickCount();
#endif

/*--------------------------------------------------------------------------------*/
/** Multiply 64-bit unsigned integer by 32-bit fraction without overflow
 */
/*--------------------------------------------------------------------------------*/
extern uint64_t muldiv(uint64_t val, uint32_t mul, uint32_t div);

/*--------------------------------------------------------------------------------*/
/** Return machine time on in nanoseconds
 */
/*--------------------------------------------------------------------------------*/
extern uint64_t GetNanosecondTicks();

extern uint32_t IEEEExtendedToINT32u(const IEEEEXTENDED *num);
extern void     INT32uToIEEEExtended(uint32_t val, IEEEEXTENDED *num);

/*--------------------------------------------------------------------------------*/
/** Set debug handler (replacing printf())
 *
 * @param handler debug handler
 * @param context optional parameter to pass to handler
 *
 */
/*--------------------------------------------------------------------------------*/
extern void SetDebugHandler(DEBUGHANDLER handler, void *context = NULL);

/*--------------------------------------------------------------------------------*/
/** Set error handler (replacing printf())
 *
 * @param handler error handler
 * @param context optional parameter to pass to handler
 *
 */
/*--------------------------------------------------------------------------------*/
extern void SetErrorHandler(DEBUGHANDLER handler, void *context = NULL);

/*--------------------------------------------------------------------------------*/
/** Create indentation string
 *
 * @param indent a string representing one level of indentation (e.g. a tab or spaces)
 * @param count indentation level
 *
 * @return a string containing <count> copies of <indent>
 */
/*--------------------------------------------------------------------------------*/
extern std::string CreateIndent(const std::string& indent, uint_t count);

/*--------------------------------------------------------------------------------*/
/** printf for std::string
 *
 * @param str string to be added to
 * @param fmt printf-style format information
 *
 */
/*--------------------------------------------------------------------------------*/
extern void Printf(std::string& str, const char *fmt, ...) PRINTF_FORMAT2;

/*--------------------------------------------------------------------------------*/
/** vprintf for std::string
 *
 * @param str string to be added to
 * @param fmt printf-style format information
 * @param ap ap_list of arguments
 *
 */
/*--------------------------------------------------------------------------------*/
extern void VPrintf(std::string& str, const char *fmt, va_list ap);

/*--------------------------------------------------------------------------------*/
/** Split a string by a delimiter, allowing for quotes to prevent splitting in the wrong place
 *
 * @param str string split
 * @param list list to be populated
 * @param delim delimiter character
 * @param maxstrings if non-zero specifies the maximum number of entries in list
 *
 * @return position in string when scanning stopped
 * 
 * @note whitespace is IGNORED!
 */
/*--------------------------------------------------------------------------------*/
extern uint_t SplitString(const std::string& str, std::vector<std::string>& list, char delim = ' ', uint_t maxstrings = 0);

/*--------------------------------------------------------------------------------*/
/** Interpolate current towards target at rate coeff, protecting against denormals
 */
/*--------------------------------------------------------------------------------*/
extern void Interpolate(double& current, double target, double coeff, double limit = 1.0e-7);

/*--------------------------------------------------------------------------------*/
/** 'cout' like debug stream handling
 * 
 * Basically use local instance of StringStream instead of 'cout' and then wrap entire line in BBCDEBUG() macro
 *
 * For example:
 * BBCDEBUG2((StringStream() << "Test " << i << " of " << n << ":" << StringStream::eol));
 *
 */
/*--------------------------------------------------------------------------------*/
class StringStream
{
public:
  StringStream();
  ~StringStream();

  StringStream& operator << (const std::string& str);
  StringStream& operator << (const char *str);
  StringStream& operator << (sint_t n);
  StringStream& operator << (uint_t n);
  StringStream& operator << (slong_t n);
  StringStream& operator << (ulong_t n);
  StringStream& operator << (sllong_t n);
  StringStream& operator << (ullong_t n);

  const char *get() const;
  operator const char *() const {return get();}
  void clear();

  static const char *eol;

protected:
  std::string data;
};

extern void debug_msg(StringStream& str);
extern void debug_err(StringStream& str);

template <typename Map>
bool map_compare (Map const &lhs, Map const &rhs)
{
  // No predicate needed because there is operator== for pairs already.
  return ((lhs.size() == rhs.size()) &&
          std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

/*--------------------------------------------------------------------------------*/
/** Prevent value becoming denormalized (bad for performance!)
 */
/*--------------------------------------------------------------------------------*/
extern float  fix_denormal(float val);
extern double fix_denormal(double val);

/*--------------------------------------------------------------------------------*/
/** Factorial of an unsigned integer.
 */
/*--------------------------------------------------------------------------------*/
extern uint_t factorial(uint_t n);

typedef struct {
  const char *name;
  const char *desc;
} PARAMETERDESC;

/*--------------------------------------------------------------------------------*/
/** Add a list of parameters/controls to a list
 */
/*--------------------------------------------------------------------------------*/
extern void AddParametersToList(const PARAMETERDESC *parameters, uint_t n, std::vector<const PARAMETERDESC *>& list);

/*--------------------------------------------------------------------------------*/
/** Convert dB to gain assuming <=-120 is cut
 */
/*--------------------------------------------------------------------------------*/
extern double dBToGain(double db);

/*--------------------------------------------------------------------------------*/
/** Convert gain to dB to gain (-120 is the minimum, including cut)
 */
/*--------------------------------------------------------------------------------*/
extern double GainTodB(double gain);

#if defined(COMPILER_MSVC)
// MSDev using %I64d etc for 64-bit
#define PRINTF_64BIT "I64"
#elif defined(__APPLE__) || !defined(__x86_64__)
// 64-bit type is long long in Apple compilers and 32-bit Linux compilers
#define PRINTF_64BIT "ll"
#else
// 64-bit is long in 64-bit Linux
#define PRINTF_64BIT "l"
#endif

/*--------------------------------------------------------------------------------*/
/** Attempt to evaluate values from strings
 */
/*--------------------------------------------------------------------------------*/
extern bool Evaluate(const std::string& str, bool& val);
extern bool Evaluate(const std::string& str, sint_t& val);
extern bool Evaluate(const std::string& str, uint_t& val);
extern bool Evaluate(const std::string& str, slong_t& val);
extern bool Evaluate(const std::string& str, ulong_t& val);
extern bool Evaluate(const std::string& str, sllong_t& val);
extern bool Evaluate(const std::string& str, ullong_t& val);
extern bool Evaluate(const std::string& str, double& val);
extern bool Evaluate(const std::string& str, std::string& val);

extern const char *DoubleFormatHuman;         // format as human-readable, scientific format with 32 decimal places
extern const char *DoubleFormatExact;         // format as hex-encoded double (exact)
extern std::string StringFrom(bool val);
extern std::string StringFrom(sint_t val, const char *fmt = "%d");
extern std::string StringFrom(uint_t val, const char *fmt = "%u");
extern std::string StringFrom(slong_t val, const char *fmt = "%ld");
extern std::string StringFrom(ulong_t val, const char *fmt = "%lu");
extern std::string StringFrom(sllong_t val, const char *fmt = "%lld");
extern std::string StringFrom(ullong_t val, const char *fmt = "%llu");
extern std::string StringFrom(double val, const char *fmt = DoubleFormatHuman);
extern std::string StringFrom(const void *val);
extern std::string StringFrom(const std::string& val); 

/*--------------------------------------------------------------------------------*/
/** Bog-standard string search and replace that *should* be in std::string!
 */
/*--------------------------------------------------------------------------------*/
extern std::string SearchAndReplace(const std::string& str, const std::string& search, const std::string& replace);

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
extern bool matchstring(const char *pat, const char *str);

#if ENABLE_JSON
extern bool                FromJSON(const json_spirit::mValue& _val, bool& val);
extern bool                FromJSON(const json_spirit::mValue& _val, sint_t& val);
extern bool                FromJSON(const json_spirit::mValue& _val, sint64_t& val);
extern bool                FromJSON(const json_spirit::mValue& _val, double& val);
extern bool                FromJSON(const json_spirit::mValue& _val, std::string& val);
extern json_spirit::mValue ToJSON(const bool& val);
extern json_spirit::mValue ToJSON(const sint_t& val);
extern json_spirit::mValue ToJSON(const sint64_t& val);
extern json_spirit::mValue ToJSON(const double& val);
extern json_spirit::mValue ToJSON(const std::string& val);
#endif

/*--------------------------------------------------------------------------------*/
/** Convert text time to ns time
 *
 * @param t ns time variable to be modified
 * @param str time in text format 'hh:mm:ss.SSSSS'
 *
 * @return true if conversion was successful
 */
/*--------------------------------------------------------------------------------*/
extern bool CalcTime(uint64_t& t, const std::string& str);

/*--------------------------------------------------------------------------------*/
/** Convert ns time to text time
 *
 * @param t ns time
 *
 * @return str time in text format 'hh:mm:ss.SSSSS'
 */
/*--------------------------------------------------------------------------------*/
extern std::string GenerateTime(uint64_t t);

/*--------------------------------------------------------------------------------*/
/** Safe (limited) addition for *unsigned* types (DOES NOT work for signed types)
 */
/*--------------------------------------------------------------------------------*/
template<typename T>
T addm(const T& a, const T& b)
{
  T c = a + b;
  return (c >= a) ? c : (T)-1;
}

/*--------------------------------------------------------------------------------*/
/** Safe (limited) subtraction for *unsigned* types (DOES NOT work for signed types)
 */
/*--------------------------------------------------------------------------------*/
template<typename T>
T subz(const T& a, const T& b)
{
  return (a >= b) ? a - b : 0;
}

/*--------------------------------------------------------------------------------*/
/** Convert a vector of pointers to a vector of pointers of another [compatible] type
 */
/*--------------------------------------------------------------------------------*/
template<typename T1,typename T2>
std::vector<T1 *> ConvertList(const std::vector<T2 *>& vec) {
  std::vector<T1 *> res(vec.size());
  uint_t i;
  for (i = 0; i < vec.size(); i++) res[i] = vec[i];
  return res;
}

/*--------------------------------------------------------------------------------*/
/** Convert a vector of pointers to a vector of pointers of another [compatible] type
 */
/*--------------------------------------------------------------------------------*/
template<typename T1,typename T2>
void ConvertList(const std::vector<T2 *>& vec, std::vector<T1 *>& res) {
  uint_t i;
  for (i = 0; i < vec.size(); i++) res.push_back(vec[i]);
}

/*--------------------------------------------------------------------------------*/
/** Convert a vector of pointers to a vector of pointers of another [compatible] type
 */
/*--------------------------------------------------------------------------------*/
template<typename T1,typename T2>
void ConvertList(const std::vector<T2 *>& vec, std::vector<const T1 *>& res) {
  uint_t i;
  for (i = 0; i < vec.size(); i++) res.push_back(vec[i]);
}

BBC_AUDIOTOOLBOX_END

#endif
