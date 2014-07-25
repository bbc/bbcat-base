#ifndef __SOUND_FORMAT_CONVERSIONS__
#define __SOUND_FORMAT_CONVERSIONS__

/*--------------------------------------------------------------------------------*/
/** This file provides a number of routines for handling multi-channel audio
 *
 * SampleFormatOf() describes the sample format for a particular variable/buffer type
 *
 * ConvertSamples() converts between one audio format and another
 *
 * Interleave() converts an audio buffer (made up of a number of frames of multi-channel audio) into another buffer which has MORE channels
 * DeInterleave() converts an audio buffer into another buffer which has LESS channels
 *
 */
/*--------------------------------------------------------------------------------*/
#include "misc.h"

BBC_AUDIOTOOLBOX_START

typedef enum {
  SampleFormat_Unknown = 0,

  SampleFormat_16bit,         // 2 bytes per sample
  SampleFormat_24bit,         // 3 bytes per sample
  SampleFormat_32bit,         // 4 bytes per sample

  SampleFormat_Float,         // 4 bytes per sample
  SampleFormat_Double,        // 8 bytes per sample

  SampleFormat_Count,

  _SampleFormat_Integer_First = SampleFormat_16bit,
  _SampleFormat_Integer_Last  = SampleFormat_32bit,
  _SampleFormat_Float_First   = SampleFormat_Float,
  _SampleFormat_Float_Last    = SampleFormat_Double,
} SampleFormat_t;

class Ditherer {
public:
  Ditherer() {}
  virtual ~Ditherer() {}

  virtual void Dither(uint_t channel, sint32_t& data, uint_t bits) {UNUSED_PARAMETER(channel); UNUSED_PARAMETER(data); UNUSED_PARAMETER(bits);}
  virtual void Dither(uint_t channel, float&    data, uint_t bits) {UNUSED_PARAMETER(channel); UNUSED_PARAMETER(data); UNUSED_PARAMETER(bits);}
  virtual void Dither(uint_t channel, double&   data, uint_t bits) {UNUSED_PARAMETER(channel); UNUSED_PARAMETER(data); UNUSED_PARAMETER(bits);}
};

typedef enum {
  Dither_None = 0,
  Dither_TPDF,
} Dither_t;

/*--------------------------------------------------------------------------------*/
/** Return sample format enumeration for a variable/buffer type
 */
/*--------------------------------------------------------------------------------*/
inline SampleFormat_t SampleFormatOf(sint16_t) {return SampleFormat_16bit;}
inline SampleFormat_t SampleFormatOf(sint32_t) {return SampleFormat_32bit;}
inline SampleFormat_t SampleFormatOf(float)    {return SampleFormat_Float;}
inline SampleFormat_t SampleFormatOf(double)   {return SampleFormat_Double;}

inline SampleFormat_t SampleFormatOf(const sint16_t *) {return SampleFormat_16bit;}
inline SampleFormat_t SampleFormatOf(const sint32_t *) {return SampleFormat_32bit;}
inline SampleFormat_t SampleFormatOf(const float    *) {return SampleFormat_Float;}
inline SampleFormat_t SampleFormatOf(const double   *) {return SampleFormat_Double;}

/*--------------------------------------------------------------------------------*/
/** Return whether sample format is integer or floating point
 */
/*--------------------------------------------------------------------------------*/
extern bool IsSampleInteger(SampleFormat_t type);
extern bool IsSampleFloat(SampleFormat_t type);

/*--------------------------------------------------------------------------------*/
/** Return number of bytes per sample for a given sample format
 */
/*--------------------------------------------------------------------------------*/
extern uint8_t GetBytesPerSample(SampleFormat_t type);

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
extern void TransferSamples(const void  *vsrc,       SampleFormat_t srctype, bool src_be,
                            uint_t      src_channel, uint_t src_channels,
                            void        *vdst,       SampleFormat_t dsttype, bool dst_be,
                            uint_t      dst_channel, uint_t dst_channels,
                            uint_t      nchannels = ~0,
                            uint_t      nframes   = 1,
                            Ditherer    *ditherer = NULL);

/*--------------------------------------------------------------------------------*/
/** Memory to memory conversions/transfers
 *
 * ASSUMES endianness of data is the same as the machine
 */
/*--------------------------------------------------------------------------------*/
extern void TransferSamples(const sint16_t *src, uint_t src_channel, uint_t src_channels, sint16_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const sint16_t *src, uint_t src_channel, uint_t src_channels, sint32_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const sint16_t *src, uint_t src_channel, uint_t src_channels, float *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const sint16_t *src, uint_t src_channel, uint_t src_channels, double *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const sint32_t *src, uint_t src_channel, uint_t src_channels, sint16_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const sint32_t *src, uint_t src_channel, uint_t src_channels, sint32_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const sint32_t *src, uint_t src_channel, uint_t src_channels, float *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const sint32_t *src, uint_t src_channel, uint_t src_channels, double *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const float *src, uint_t src_channel, uint_t src_channels, sint16_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const float *src, uint_t src_channel, uint_t src_channels, sint32_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const float *src, uint_t src_channel, uint_t src_channels, float *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const float *src, uint_t src_channel, uint_t src_channels, double *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const double *src, uint_t src_channel, uint_t src_channels, sint16_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const double *src, uint_t src_channel, uint_t src_channels, sint32_t *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const double *src, uint_t src_channel, uint_t src_channels, float *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);
extern void TransferSamples(const double *src, uint_t src_channel, uint_t src_channels, double *dst, uint_t dst_channel, uint_t dst_channels, uint_t nchannels = ~0, uint_t nframes = 1, Ditherer *ditherer = NULL);

BBC_AUDIOTOOLBOX_END

#endif
