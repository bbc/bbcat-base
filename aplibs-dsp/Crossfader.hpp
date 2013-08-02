/*
 * Crossfader.hpp
 *
 *  Created on: 02 Aug 2013
 *      Author: Andrew Bonney
 */

#ifndef CROSSFADE_HPP_
#define CROSSFADE_HPP_

#include <stdint.h>

class Crossfader {
public:
	Crossfader(uint32_t windowSize);
	~Crossfader();
	void Crossfade(float* pInputFrom, float* pInputTo, float* pOutput);
	void SetPattern(float* pPattern);
private:
	const uint32_t mWindowSize;
	float* mpLinearPattern; // Linear slope from zero to one
	float* mpCurrentPattern;
};

#endif
