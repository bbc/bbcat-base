#ifndef __SOUND_DELAY_BUFFER__
#define __SOUND_DELAY_BUFFER__

#include "SoundFormatConversions.h"

BBC_AUDIOTOOLBOX_START

class SoundDelayBuffer
{
public:
  SoundDelayBuffer();
  virtual ~SoundDelayBuffer();

  /*--------------------------------------------------------------------------------*/
  /** Resize buffer
   *
   * @param chans number of channels (i.e. width)
   * @param length number of samples (i.e. number of frames)
   * @param type format of buffer
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetSize(uint_t chans, uint_t length, SampleFormat_t type = SampleFormat_Double);

  uint_t         GetChannels()      const {return channels;}
  uint_t         GetLength()        const {return buflen;}
  uint_t         GetWritePosition() const {return writepos;}
  SampleFormat_t GetFormat()        const {return format;}

  bool           GetBuffer(const double   **p) const {*p = (format == SampleFormatOf(*p)) ? (const double   *)buf : NULL; return (*p != NULL);}
  bool           GetBuffer(const float    **p) const {*p = (format == SampleFormatOf(*p)) ? (const float    *)buf : NULL; return (*p != NULL);}
  bool           GetBuffer(const sint32_t **p) const {*p = (format == SampleFormatOf(*p)) ? (const sint32_t *)buf : NULL; return (*p != NULL);}
  bool           GetBuffer(const sint16_t **p) const {*p = (format == SampleFormatOf(*p)) ? (const sint16_t *)buf : NULL; return (*p != NULL);}

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
  virtual uint_t WriteSamples(const uint8_t  *src, SampleFormat_t srcformat, uint_t channel = 0, uint_t nchannels = ~0, uint_t nframes = 1);
  virtual uint_t WriteSamples(const sint16_t *src, uint_t channel = 0, uint_t nchannels = ~0, uint_t nframes = 1) {return WriteSamples((const uint8_t *)src, SampleFormatOf(src), channel, nchannels, nframes);}
  virtual uint_t WriteSamples(const sint32_t *src, uint_t channel = 0, uint_t nchannels = ~0, uint_t nframes = 1) {return WriteSamples((const uint8_t *)src, SampleFormatOf(src), channel, nchannels, nframes);}
  virtual uint_t WriteSamples(const float    *src, uint_t channel = 0, uint_t nchannels = ~0, uint_t nframes = 1) {return WriteSamples((const uint8_t *)src, SampleFormatOf(src), channel, nchannels, nframes);}
  virtual uint_t WriteSamples(const double   *src, uint_t channel = 0, uint_t nchannels = ~0, uint_t nframes = 1) {return WriteSamples((const uint8_t *)src, SampleFormatOf(src), channel, nchannels, nframes);}

  /*--------------------------------------------------------------------------------*/
  /** Increment write position by specified amount
   */
  /*--------------------------------------------------------------------------------*/
  virtual void IncrementWritePosition(uint_t nframes = 1) {writepos = (writepos + nframes) % buflen;}

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
  virtual uint_t ReadSamples(uint8_t  *dst, SampleFormat_t dstformat, uint_t delay, uint_t channel = 0, uint_t nchannels = ~0, uint_t nframes = 1);
  virtual uint_t ReadSamples(sint16_t *dst, uint_t delay, uint_t channel = 0, uint_t nchannels = ~0, uint_t nframes = 1) {return ReadSamples((uint8_t *)dst, SampleFormatOf(dst), delay, channel, nchannels, nframes);}
  virtual uint_t ReadSamples(sint32_t *dst, uint_t delay, uint_t channel = 0, uint_t nchannels = ~0, uint_t nframes = 1) {return ReadSamples((uint8_t *)dst, SampleFormatOf(dst), delay, channel, nchannels, nframes);}
  virtual uint_t ReadSamples(float    *dst, uint_t delay, uint_t channel = 0, uint_t nchannels = ~0, uint_t nframes = 1) {return ReadSamples((uint8_t *)dst, SampleFormatOf(dst), delay, channel, nchannels, nframes);}
  virtual uint_t ReadSamples(double   *dst, uint_t delay, uint_t channel = 0, uint_t nchannels = ~0, uint_t nframes = 1) {return ReadSamples((uint8_t *)dst, SampleFormatOf(dst), delay, channel, nchannels, nframes);}

  /*--------------------------------------------------------------------------------*/
  /** Simple single sample reading
   */
  /*--------------------------------------------------------------------------------*/
  virtual Sample_t ReadSample(uint_t channel, uint_t delay) const;

protected:
  uint8_t        *buf;
  SampleFormat_t format;
  uint_t         channels;
  uint_t         buflen, writepos;
};

class SoundRingBuffer : public SoundDelayBuffer
{
public:
  SoundRingBuffer();
  virtual ~SoundRingBuffer();

  /*--------------------------------------------------------------------------------*/
  /** Resize buffer
   *
   * @param chans number of channels (i.e. width)
   * @param length number of samples (i.e. number of frames)
   * @param type format of buffer
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetSize(uint_t chans, uint_t length, SampleFormat_t type = SampleFormat_Double);

  virtual uint_t GetReadPosition()         const {return readpos;}
  virtual uint_t GetReadFramesAvailable()  const {return (writepos + buflen - readpos) % buflen;}
  virtual uint_t GetWriteFramesAvailable() const {return (readpos  + buflen - writepos - 1) % buflen;}        // note additional subtract of 1

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
  virtual uint_t WriteSamples(const uint8_t  *src, SampleFormat_t srcformat, uint_t channel = 0, uint_t nchannels = ~0, uint_t nframes = 1);

  /*--------------------------------------------------------------------------------*/
  /** Increment write position by specified amount
   */
  /*--------------------------------------------------------------------------------*/
  virtual void IncrementWritePosition(uint_t nframes = 1) {nframes = MIN(nframes, GetWriteFramesAvailable()); writepos = (writepos + nframes) % buflen;}

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
  virtual uint_t ReadSamples(uint8_t  *dst, SampleFormat_t dstformat, uint_t delay, uint_t channel = 0, uint_t nchannels = ~0, uint_t nframes = 1);

  /*--------------------------------------------------------------------------------*/
  /** Increment read position by specified amount
   */
  /*--------------------------------------------------------------------------------*/
  virtual void IncrementReadPosition(uint_t nframes = 1) {nframes = MIN(nframes, GetReadFramesAvailable()); readpos = (readpos + nframes) % buflen;}

protected:
  uint_t readpos;
};

BBC_AUDIOTOOLBOX_END

#endif

