#ifndef __SOUND_FORMAT_RAW_CONVERSIONS__
#define __SOUND_FORMAT_RAW_CONVERSIONS__

#include "SoundFormatConversions.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Conversion routines - DO NOT CALL directly.  Use TransferSamples()
 */
/*--------------------------------------------------------------------------------*/
typedef void (*CONVERTSAMPLES)(const uint8_t *src, uint8_t *dst, uint_t nchannels, uint_t nframes, sint_t srcflen, sint_t dstflen, Ditherer *ditherer);
extern const CONVERTSAMPLES SoundFormatConversions[2][2][SampleFormat_Count][SampleFormat_Count];

BBC_AUDIOTOOLBOX_END

#endif
