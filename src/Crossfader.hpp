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
  // interleavedChannels: Set to one for non-interleaved
  Crossfader(uint32_t windowSize, uint32_t interleavedChannels);
  ~Crossfader();
  void Crossfade(float* pInputFrom, float* pInputTo, float* pOutput);
  void SetPattern(float* pPattern);
  void SetPattern(float gainStart, float gainEnd);
private:
  const uint32_t mWindowSize;
  const uint32_t mInterleavedChannels;
  float* mpLinearPattern; // Linear slope from zero to one
  float* mpCurrentPattern;
};

#endif
