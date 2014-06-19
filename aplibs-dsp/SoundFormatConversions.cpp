
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEBUG_LEVEL 1
#include "SoundFormatConversions.h"
#include "SoundFormatRawConversions.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Array of number of bytes for each format
 */
/*--------------------------------------------------------------------------------*/
const uint8_t SoundFormatBytes[SampleFormat_Count] = {
	1, // SampleFormat_Unknown
	2, // SampleFormat_16bit
	3, // SampleFormat_24bit
	4, // SampleFormat_32bit
	4, // SampleFormat_Float
	8, // SampleFormat_Double
};

/*--------------------------------------------------------------------------------*/
/** Return number of bytes per sample for a given sample format
 */
/*--------------------------------------------------------------------------------*/
uint8_t GetBytesPerSample(SampleFormat_t type)
{
	return SoundFormatBytes[type];
}

/*--------------------------------------------------------------------------------*/
/** Move/convert samples from one format to another and from one buffer to another
 *
 * @param vsrc pointer to source buffer (void * for easy casting) 
 * @param srctype format type of source
 * @param src_be true if source samples are big-endian
 * @param src_channel starting channel to read from
 * @param src_channels total number of source channels in buffer
 * @param vdst pointer to destination buffer (void * for easy casting)
 * @param dsttype format type of destination
 * @param dst_be true if destination samples are big-endian
 * @param dst_channel starting channel to write to
 * @param dst_channels total number of destination channels in buffer
 * @param nchannels number of channels to transfer/convert
 * @param nframes number of frames to transfer/convert
 * @param ditherer pointer to ditherer object or NULL
 *
 * @note this function provides copying, converting, interleaving and de-interleaving functionality
 * @note it allows converting in-place (i.e. dst == src)
 * @note HOWEVER, src and dst MUST NOT OVERLAP UNLESS dst == src
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
 * |   |sss===============================>ddd|    |   | nframes
 * |   |sssss|     |             |      |ddddd|    |   |
 * |   |sssss|     |             |      |ddddd|    |   |
 * |   |sssss|     |             |      |ddddd|    |   |
 * |   |sssss|     |             |      |ddddd|    |   |
 * |   |sssss|     |             |      |ddddd|    |   v
 * +---+-----+-----+             +------+-----+----+   -
 * |-------------->|             |---------------->|
 *   src_channels                   dst_channels
 *
 * s = source format srctype / src_be
 * d = destination format dsttype / dst_be
 *
 * Notes:
 *   dst and src CAN be the same but otherwise MUST NOT overlap
 *
 *   0 <= src_channel <= (src_channels - nchannels)
 *   0 <= dst_channel <= (dst_channels - nchannels)
 *   0 <  nchannels   <= src_channels
 *   0 <  nchannels   <= dst_channels
 */
/*--------------------------------------------------------------------------------*/
void TransferSamples(const void *vsrc,		 SampleFormat_t srctype, bool src_be,
					 uint_t 	src_channel, uint_t src_channels,
					 void		*vdst,	     SampleFormat_t dsttype, bool dst_be,
					 uint_t 	dst_channel, uint_t dst_channels,
					 uint_t 	nchannels,
					 uint_t		nframes,
					 Ditherer	*ditherer)
{
	// sanity checks
	if (src_channels && dst_channels &&
		nframes && nchannels &&
		(srctype != SampleFormat_Unknown) && (srctype < SampleFormat_Count) &&
		(dsttype != SampleFormat_Unknown) && (dsttype < SampleFormat_Count)) {
		const uint8_t *src = (const uint8_t *)vsrc;
		uint8_t       *dst = (uint8_t       *)vdst;
		sint_t srclen  = GetBytesPerSample(srctype); // (signed so that the direction of operation can be backwards as well as forwards)
		sint_t dstlen  = GetBytesPerSample(dsttype); // (signed so that the direction of operation can be backwards as well as forwards)

		// restrict input data to sensible values
		src_channel    = MIN(src_channel, src_channels - 1);
		dst_channel    = MIN(dst_channel, dst_channels - 1);

		nchannels      = MIN(nchannels,   src_channels - src_channel);
		nchannels	   = MIN(nchannels,   dst_channels - dst_channel);

		// final sanity check
		if (!nchannels) return;

		if ((nchannels == src_channels) && (nchannels == dst_channels)) {
			// optimization: if source and destination samples are all contiguous, reduce the process to a single frame of many channels
			nchannels *= nframes;
			nframes    = 1;
		}

		sint_t srcflen = src_channels * srclen;	// source frame    length (signed so that the direction of operation can be backwards as well as forwards)
		sint_t dstflen = dst_channels * dstlen;	// dest   frame    length (signed so that the direction of operation can be backwards as well as forwards)

		// move to desired offsets (starting channel)
		src += src_channel * srclen;
		dst += dst_channel * dstlen;

		if (dstflen > srcflen) {
			// destination rectangle is BIGGER than source rectangle, switch to running backwards from the end of the buffers
			src += (nframes - 1) * srcflen;
			dst += (nframes - 1) * dstflen;
			srcflen = -srcflen;
			dstflen = -dstflen;
		}

		// look up correct function for conversion/copy
		CONVERTSAMPLES fn = SoundFormatConversions[(uint_t)src_be][(uint_t)dst_be][srctype][dsttype];
		if (!fn) {
			ERROR("Unknown copying routine for (%u/%u/%u/%u)", (uint_t)src_be, (uint_t)dst_be, srctype, dsttype);
			return;
		}

		// finally, run the functions
		(*fn)(src, dst, nchannels, nframes, srcflen, dstflen, ditherer);
	}
}

void TransferSamples(const sint16_t *src, uint_t src_channel, uint_t src_channels, sint16_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_16bit, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_16bit, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const sint16_t *src, uint_t src_channel, uint_t src_channels, sint32_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_16bit, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_32bit, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const sint16_t *src, uint_t src_channel, uint_t src_channels, float *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_16bit, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_Float, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const sint16_t *src, uint_t src_channel, uint_t src_channels, double *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_16bit, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_Double, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const sint32_t *src, uint_t src_channel, uint_t src_channels, sint16_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_32bit, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_16bit, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const sint32_t *src, uint_t src_channel, uint_t src_channels, sint32_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_32bit, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_32bit, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const sint32_t *src, uint_t src_channel, uint_t src_channels, float *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_32bit, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_Float, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const sint32_t *src, uint_t src_channel, uint_t src_channels, double *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_32bit, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_Double, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const float *src, uint_t src_channel, uint_t src_channels, sint16_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_Float, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_16bit, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const float *src, uint_t src_channel, uint_t src_channels, sint32_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_Float, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_32bit, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const float *src, uint_t src_channel, uint_t src_channels, float *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_Float, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_Float, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const float *src, uint_t src_channel, uint_t src_channels, double *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_Float, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_Double, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const double *src, uint_t src_channel, uint_t src_channels, sint16_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_Double, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_16bit, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const double *src, uint_t src_channel, uint_t src_channels, sint32_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_Double, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_32bit, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const double *src, uint_t src_channel, uint_t src_channels, float *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_Double, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_Float, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

void TransferSamples(const double *src, uint_t src_channel, uint_t src_channels, double *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels, uint_t nframes, Ditherer *ditherer)
{
	TransferSamples(src, SampleFormat_Double, MACHINE_IS_BIG_ENDIAN, src_channel, src_channels, dst, SampleFormat_Double, MACHINE_IS_BIG_ENDIAN, dst_channel, dst_channels, nchannels, nframes, ditherer);
}

BBC_AUDIOTOOLBOX_END
