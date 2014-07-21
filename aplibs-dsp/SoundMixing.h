#ifndef __SOUND_MIXING__
#define __SOUND_MIXING__

#include "misc.h"

BBC_AUDIOTOOLBOX_START

extern void MixSamples(const Sample_t *src,
                       uint_t src_channel, uint_t src_channels,
                       Sample_t *dst,
                       uint_t dst_channel, uint_t dst_channels,
                       uint_t nchannels = ~0,
                       uint_t nframes = 1,
                       Sample_t mul = 1.0);

BBC_AUDIOTOOLBOX_END

#endif

