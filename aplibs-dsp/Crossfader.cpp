/*
 * Crossfader.cpp
 *
 *  Created on: 02 Aug 2013
 *      Author: Andrew Bonney
 */

#include "Crossfader.hpp"

Crossfader::Crossfader(uint32_t windowSize) : mWindowSize(windowSize)
{
	mpLinearPattern = new float[mWindowSize];
	for (uint32_t i=0; i<mWindowSize; i++)
	{
		mpLinearPattern[i] = (float)i / (mWindowSize - 1);
	}
	mpCurrentPattern = mpLinearPattern;
}

Crossfader::~Crossfader()
{
	delete mpLinearPattern;
}

void Crossfader::Crossfade(float* pInputFrom, float* pInputTo, float* pOutput)
{
	for (uint32_t i=0; i<mWindowSize; i++)
	{
		pOutput[i] = (pInputTo[i] * mpCurrentPattern[i]) + (pInputFrom[i] * mpCurrentPattern[mWindowSize - (i + 1)]);
	}
}

void Crossfader::SetPattern(float* pPattern)
{
	mpCurrentPattern = pPattern;
}
