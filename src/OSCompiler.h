#ifndef __OS_COMPILER__
#define __OS_COMPILER__

/*--------------------------------------------------------------------------------*/
/** Some simple macros to identify compiler and OS
 */
/*--------------------------------------------------------------------------------*/

#if defined(__clang__) || defined(__GNUC__)
#define COMPILER_GCC
#endif

#ifdef _MSC_VER
#define COMPILER_MSVC
#endif

#if defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(_WIN32) || defined(_WIN64)
#define TARGET_OS_WINDOWS
#endif

#if defined(__APPLE__) || defined(__unix__)
#define TARGET_OS_UNIXBSD
#endif

#endif
