/*
 * logging.hpp
 *
 *  Created on: 15 Aug 2013
 *      Author: chrisp
 */

#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include <iostream>

// some preprocessor macros:

/// turn the argument into a string
#define __STR__(x)  #x
/// workaround to evaluate the argument and pass the result to __STR__
#define __XSTR__(x) __STR__(x)
/// make a string with filename and line number
#define __POS__ "(" __FILE__ ":" __XSTR__(__LINE__) ")"

/// Write a warning message to stderr.
#define APLIBS_DSP_INFO(msg) \
		std::cerr << "aplibs-dsp Info: " << msg << " " __POS__ << std::endl

/// Write a warning message to stderr.
#define APLIBS_DSP_WARNING(msg) \
		std::cerr << "aplibs-dsp Warning: " << msg << " " __POS__ << std::endl
/// Write an error message to stderr.
#define APLIBS_DSP_ERROR(msg) \
		std::cerr << "aplibs-dsp Error: " << msg << " " __POS__ << std::endl


#endif /* LOGGING_HPP_ */
