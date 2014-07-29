
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEBUG_LEVEL 2 
#include "FractionalSample.h"
#include "SoundDelayBuffer.h"

BBC_AUDIOTOOLBOX_START

SoundDelayBuffer::SoundDelayBuffer() : buf(NULL),
                                       format(SampleFormat_Double),
                                       channels(0),
                                       buflen(0),
                                       writepos(0)
{
}

SoundDelayBuffer::~SoundDelayBuffer()
{
  if (buf) delete[] buf;
}

void SoundDelayBuffer::SetSize(uint_t chans, uint_t length, SampleFormat_t type)
{
  chans  = MAX(chans, 1);
  length = MAX(length, 1);

  if ((chans != channels) || (length != buflen) || (type != format))
  {
    uint8_t *newbuf;
    uint_t  bps = GetBytesPerSample(type);

    // add space for an extra double at the end of the buffer (irrespective of the format of the buffer) 
    // to allow space for conversion
    if ((newbuf = new uint8_t[chans * length * bps]) != NULL)
    {
      // clear new allocation
      memset(newbuf, 0, chans * length * bps);

      if (buf)
      {
        // map existing delay data into new buffer
        TransferSamples(buf, format, MACHINE_IS_BIG_ENDIAN, 0, channels,
                        newbuf,   format, MACHINE_IS_BIG_ENDIAN, 0, chans,
                        ~0, // this will be limited to the smaller of channels and chans
                        buflen);
        delete[] buf;
      }

      buf       = newbuf;
      channels  = chans;
      buflen    = length;
      format    = type;
      writepos %= buflen;
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Write samples into buffer
 *
 * @param src source of samples
 * @param srcformat format of source samples (ASSUMES same endianness as machine)
 * @param channel channel within BUFFER (not source) to start write at
 * @param nchannels number of channels within BUFFER (not source) to write
 * @param nframes number of frames to write
 *
 * @return number of frames written
 *
 * @note the source data is assumed to be interleaved but contiguous (i.e. width of source == nchannels)
 */
/*--------------------------------------------------------------------------------*/
uint_t SoundDelayBuffer::WriteSamples(const uint8_t *src, SampleFormat_t srcformat, uint_t channel, uint_t nchannels, uint_t nframes)
{
  uint_t frames = 0;

  if (buf)
  {
    uint_t srclen = GetBytesPerSample(srcformat);
    uint_t dstlen = GetBytesPerSample(format);
    uint_t pos    = writepos;

    // limit requested channels to those held in the buffer
    channel   = MIN(channel,   channels - 1);
    nchannels = MIN(nchannels, channels - channel);

    while (nframes)
    {
      uint8_t *dst = buf + pos * channels * dstlen;   // destination for copy
      uint_t n = MIN(nframes, buflen - pos);          // maximum number of frames that can be stored this time

      TransferSamples(src, srcformat, MACHINE_IS_BIG_ENDIAN, 0,       nchannels,      // note differing number of channels for source
                      dst, format,    MACHINE_IS_BIG_ENDIAN, channel, channels,       // and destination -> this essentially interleaves the data
                      nchannels,
                      n);

      src     += nchannels * srclen * n;
      pos     += n;
      pos     %= buflen;
      nframes -= n;
      frames  += n;
    }
  }

  return frames;
}

/*--------------------------------------------------------------------------------*/
/** Read samples from buffer from some time previous to the current write position
 *
 * @param dst destination for samples
 * @param dstformat destination format for samples (ASSUMES same endianness as machine)
 * @param delay delay in samples (back from current write position)
 * @param channel channel within BUFFER (not destination) to start read from
 * @param nchannels number of channels within BUFFER (not destination) to read
 * @param nframes number of frames to read
 *
 * @return number of frames read
 *
 * @note the destination data is written as interleaved but contiguous (i.e. width of destination == nchannels)
 * @note the number of frames read will be LIMITED by the data in the buffer (i.e. the delay parameter)
 */
/*--------------------------------------------------------------------------------*/
uint_t SoundDelayBuffer::ReadSamples(uint8_t *dst, SampleFormat_t dstformat, uint_t delay, uint_t channel, uint_t nchannels, uint_t nframes)
{
  uint_t frames = 0;

  if (buf)
  {
    uint_t srclen = GetBytesPerSample(format);
    uint_t dstlen = GetBytesPerSample(dstformat);
    uint_t pos    = (writepos + buflen - delay) % buflen;

    // limit requested channels to those held in the buffer
    channel   = MIN(channel,   channels - 1);
    nchannels = MIN(nchannels, channels - channel);

    // limit the number of frames to the delay parameter since this is the maximum amount of data
    // that can be safely read
    nframes   = MIN(nframes, delay);

    while (nframes)
    {
      const uint8_t *src = buf + pos * channels * srclen; // source for copy
      uint_t n = MIN(nframes, buflen - pos);                  // maximum number of frames that can be stored this time

      TransferSamples(src, format,    MACHINE_IS_BIG_ENDIAN, channel, channels,       // note differing number of channels for source
                      dst, dstformat, MACHINE_IS_BIG_ENDIAN, 0,       nchannels,      // and destination -> this essentially de-interleaves the data
                      nchannels,
                      n);
            
      dst     += nchannels * dstlen * n;
      pos     += n;
      pos     %= buflen;
      nframes -= n;
      frames  += n;
    }
  }

  return frames;
}

Sample_t SoundDelayBuffer::ReadSample(uint_t channel, uint_t delay) const
{
  uint_t   srclen = GetBytesPerSample(format);
  Sample_t res;

  TransferSamples(buf + ((writepos + buflen - delay) % buflen) * channels * srclen,
                  format, MACHINE_IS_BIG_ENDIAN,
                  channel, channels,
                  &res, SampleFormatOf(res), MACHINE_IS_BIG_ENDIAN,
                  0, 1);
  return res;
}

/*----------------------------------------------------------------------------------------------------*/

SoundRingBuffer::SoundRingBuffer() : SoundDelayBuffer(),
                                     readpos(0)
{
}

SoundRingBuffer::~SoundRingBuffer()
{
}

/*--------------------------------------------------------------------------------*/
/** Resize buffer
 *
 * @param chans number of channels (i.e. width)
 * @param length number of samples (i.e. number of frames)
 * @param type format of buffer
 *
 */
/*--------------------------------------------------------------------------------*/
void SoundRingBuffer::SetSize(uint_t chans, uint_t length, SampleFormat_t type)
{
  SoundDelayBuffer::SetSize(chans, length, type);
  readpos %= buflen;
}

/*--------------------------------------------------------------------------------*/
/** Write samples into buffer
 *
 * @param src source of samples
 * @param srcformat format of source samples (ASSUMES same endianness as machine)
 * @param channel channel within BUFFER (not source) to start write at
 * @param nchannels number of channels within BUFFER (not source) to write
 * @param nframes number of frames to write
 *
 * @return number of frames written
 *
 * @note the source data is assumed to be interleaved but contiguous (i.e. width of source == nchannels)
 * @note the number of frames written will be LIMITED by the space available dictated by the read and write positions
 */
/*--------------------------------------------------------------------------------*/
uint_t SoundRingBuffer::WriteSamples(const uint8_t  *src, SampleFormat_t srcformat, uint_t channel, uint_t nchannels, uint_t nframes)
{
  /*
   * the buffer can be viewed like this:
   *
   *               write                            read
   * |---------------|-------------------------------|--------------|
   *                 |                              |
   *                 |----------------------------->|
   *                            maxframes
   *
   * Therefore:
   *   maxframes = read - write - 1
   *
   * - 1 is to stop write position being coincident with read position after a write
   *
   * (All modulo maths)
   */
  // simply limit frames to write and call SoundDelayBuffer::WriteSamples()
  uint_t maxframes = GetWriteFramesAvailable();

  return SoundDelayBuffer::WriteSamples(src, srcformat, channel, nchannels, MIN(nframes, maxframes));
}

/*--------------------------------------------------------------------------------*/
/** Read samples from buffer from some time previous to the current READ position
 *
 * @param dst destination for samples
 * @param dstformat destination format for samples (ASSUMES same endianness as machine)
 * @param delay delay in samples (back from current write position)
 * @param channel channel within BUFFER (not destination) to start read from
 * @param nchannels number of channels within BUFFER (not destination) to read
 * @param nframes number of frames to read
 *
 * @return number of frames read
 *
 * @note the destination data is written as interleaved but contiguous (i.e. width of destination == nchannels)
 * @note the number of frames read will be LIMITED by the data in the buffer (taking into consideration the delay parameter)
 */
/*--------------------------------------------------------------------------------*/
uint_t SoundRingBuffer::ReadSamples(uint8_t  *dst, SampleFormat_t dstformat, uint_t delay, uint_t channel, uint_t nchannels, uint_t nframes)
{
  /*
   * the buffer can be viewed like this:
   *
   *               write                            read
   * |---------------|-------------------------------|--------------|
   *                                                 |
   *                                          |<-----|
   *                                          | delay
   *                                          |
   *                                          |---------------------|
   * |-------------->|                               maxframes    >>>
   * >>> maxframes
   *
   * Therefore:
   *   0 <= delay <= read - write
   *
   *   maxframes = write - (read - delay)
   *   maxframes = write + delay - read
   *
   * (All modulo maths)
   */
  // limit delay parameter to data available
  delay = MIN(delay, ((readpos + buflen - writepos) % buflen));

  // calculate maximum number of frames that can be read
  uint_t maxframes = (writepos + buflen + delay - readpos) % buflen;

  return SoundDelayBuffer::ReadSamples(dst, dstformat, delay, channel, nchannels, MIN(nframes, maxframes));
}

BBC_AUDIOTOOLBOX_END
