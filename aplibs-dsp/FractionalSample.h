#ifndef __FRACTIONAL_SAMPLE__
#define __FRACTIONAL_SAMPLE__

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Return the sample given by a floating point position within a buffer, assuming the buffer is circular
 *
 * @param buffer buffer containing channels x length samples
 * @param channel channel number read from
 * @param channels number of channels buffer contains (and therefore the step for each sample of the same channel)
 * @param length number of sample frames in the buffer
 * @param pos position
 *
 * @return interpolated sample at the desired position (offset by filter length)
 *
 * @note this function uses an SRC filter to allow 128 times over sampling of the position
 * @note it is equivalent to upsampling the audio by a factor of 128, find the integer position in the upsampled audio
 * @note and returning the sample from that position
 * 
 * @note in truth, it actually returns the sample from 7 (half of 14) positions BACK from the requested position because
 * @note it cannot assume that audio exists beyond the position requested and needs 14 valid samples for the filter,
 * @note it therefore assumes that there is at least 14 samples of valid historical samples available and moves the filter
 * @note back by 14 samples.  Since the peak filter value is in the centre of the filter, the result appears to be from 7 samples back
 *
 * @note because it uses a filter (of 14 taps), it is relatively CPU hungry 
 */
/*--------------------------------------------------------------------------------*/
extern double FractionalSample(const double *buffer, uint_t channel, uint_t channels, uint_t length, double pos);
extern double FractionalSample(const float  *buffer, uint_t channel, uint_t channels, uint_t length, double pos);

BBC_AUDIOTOOLBOX_END

#endif

