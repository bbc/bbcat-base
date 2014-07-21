
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SoundMixing.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Mix source samples to destination samples (like TransferSamples() but adding instead of over-writing)
 *
 * @param src pointer to source buffer
 * @param src_channel starting channel to read from
 * @param src_channels total number of source channels in buffer
 * @param dst pointer to destination buffer
 * @param dst_channel starting channel to write to
 * @param dst_channels total number of destination channels in buffer
 * @param nchannels number of channels to transfer/convert
 * @param nframes number of frames to transfer/convert
 * @param mul scaling factor
 *
 * The process can be viewed as the following:
 *
 * src:                          dst:
 * src_channel                   dst_channel
 * |-->|                         |----->|
 *     nchannels                        nchannels
 *     |---->|                          |---->|
 * +---+-----+-----+             +------+-----+----+   -
 * |   |sssss|     |             |      |ddddd|    |   |
 * |   |sssss|     |             |      |ddddd|    |   |
 * |   |sssss|     |             |      |ddddd|    |   |
 * |   |sssss|     |             |      |ddddd|    |   |
 * |   |sssss|     |             |      |ddddd|    |   |
 * |   |sss======== Scale and Add ========>ddd|    |   | nframes
 * |   |sssss|     |             |      |ddddd|    |   |
 * |   |sssss|     |             |      |ddddd|    |   |
 * |   |sssss|     |             |      |ddddd|    |   |
 * |   |sssss|     |             |      |ddddd|    |   |
 * |   |sssss|     |             |      |ddddd|    |   v
 * +---+-----+-----+             +------+-----+----+   -
 * |-------------->|             |---------------->|
 *   src_channels                   dst_channels
 *
 * s = source
 * d = destination
 *
 * @note UNLIKE TransferSamples(), src and dst MUST NOT overlap in anyway (would make no sense anyway)
 *
 *   0 <= src_channel <= (src_channels - nchannels)
 *   0 <= dst_channel <= (dst_channels - nchannels)
 *   0 <  nchannels   <= src_channels
 *   0 <  nchannels   <= dst_channels
 */
/*--------------------------------------------------------------------------------*/
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
      nframes      && nchannels    &&
      (mul > 0.0))
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
