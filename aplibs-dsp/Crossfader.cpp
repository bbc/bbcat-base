/*
 * Crossfader.cpp
 *
 *  Created on: 02 Aug 2013
 *      Author: Andrew Bonney
 */

#include <stdexcept>

#include "Crossfader.hpp"

Crossfader::Crossfader(uint32_t windowSize, uint32_t interleavedChannels) :
	mWindowSize(windowSize), mInterleavedChannels(interleavedChannels)
{
	if (mInterleavedChannels < 1)
	{
		throw std::invalid_argument("Channels must be greater than or equal to one");
	}
	mpLinearPattern = new float[mWindowSize * mInterleavedChannels];
	for (uint32_t i=0; i<mWindowSize; i++)
	{
		for (uint32_t j=0; j<mInterleavedChannels; j++)
		{
			mpLinearPattern[i*mInterleavedChannels + j] = (float)i / (mWindowSize - 1);
		}
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
		for (uint32_t j=0; j<mInterleavedChannels; j++)
		{
			pOutput[i*mInterleavedChannels + j] = (pInputTo[i*mInterleavedChannels + j] * mpCurrentPattern[i*mInterleavedChannels + j]) +
					(pInputFrom[i*mInterleavedChannels + j] * mpCurrentPattern[mWindowSize*mInterleavedChannels - (i*mInterleavedChannels + (mInterleavedChannels - j))]);
		}
	}
}

void Crossfader::SetPattern(float* pPattern)
{
	mpCurrentPattern = pPattern;
}
