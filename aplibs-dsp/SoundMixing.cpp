
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SoundMixing.h"

BBC_AUDIOTOOLBOX_START

void MixSamples(const Sample_t *src,
                uint_t src_channel, uint_t src_channels,
                Sample_t *dst,
                uint_t dst_channel, uint_t dst_channels,
                uint_t nchannels,
                uint_t nframes,
                Sample_t mul)
{
  // sanity checks
  if (src_channels && dst_channels &&
      nframes      && nchannels)
  {
    // restrict input data to sensible values
    src_channel = MIN(src_channel, src_channels - 1);
    dst_channel = MIN(dst_channel, dst_channels - 1);

    nchannels   = MIN(nchannels,   src_channels - src_channel);
    nchannels   = MIN(nchannels,   dst_channels - dst_channel);

    // final sanity check
    if (!nchannels) return;

    if ((nchannels == src_channels) && (nchannels == dst_channels))
    {
      // optimization: if source and destination samples are all contiguous, reduce the process to a single frame of many channels
      nchannels *= nframes;
      nframes    = 1;
    }

    // move to desired offsets (starting channel)
    src += src_channel;
    dst += dst_channel;

    uint_t i, j;
    for (i = 0; i < nframes; i++, src += src_channels, dst += dst_channels)
    {
      for (j = 0; j < nchannels; j++) dst[j] += mul * src[j];
    }
  }
}

BBC_AUDIOTOOLBOX_END
