/*
 * Convolver.h
 *
 *  Created on: May 13, 2014
 *      Author: chrisp
 */

#ifndef CONVOLVER_H_
#define CONVOLVER_H_

#include <string>
#include <vector>
#include <pthread.h>
#include <semaphore.h>

#include "misc.h"
#include "ThreadLock.h"
#if ENABLE_SOFA
#include "SOFA.h"
#endif

// APF convolver header files cannot be included in header files because of function implementations in the header files
// which lead to multiple definitions when linking!
// So this acts as a prototype for the required objects from convolver.h
namespace apf {
  namespace conv {
    struct Convolver;
    struct StaticConvolver;
    struct Filter;
  }
}

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** ConvolverManager acts as manager for the individual convolvers, it holds all the
 * impulse responses (as frequency-domain filters) and creates and destroys convolver
 * objects
 */
/*--------------------------------------------------------------------------------*/
class Convolver;
class ConvolverManager {
public:
  /*--------------------------------------------------------------------------------*/
  /** Constructor for convolver manager
   *
   * @param partitionsize the convolution partition size - essentially the block size of the processing
   *
   */
  /*--------------------------------------------------------------------------------*/
  ConvolverManager(uint_t partitionsize);

  /*--------------------------------------------------------------------------------*/
  /** Constructor for convolver manager
   *
   * @param irfile file containing IRs (either WAV or SOFA) - SOFA file can also contain delays
   * @param partitionsize the convolution partition size - essentially the block size of the processing
   *
   */
  /*--------------------------------------------------------------------------------*/
  ConvolverManager(const char *irfile, uint_t partitionsize);

  /*--------------------------------------------------------------------------------*/
  /** Constructor for convolver manager.
   *  The delays in irdelayfile will overwrite any specified within irfile if
   *  irfile is a SOFA file.
   *
   * @param irfile file containing IRs (either WAV or SOFA)
   * @param irdelayfile text file containing the required delays (in SAMPLES) of each IR in irfile
   * @param partitionsize the convolution partition size - essentially the block size of the processing
   *
   */
  /*--------------------------------------------------------------------------------*/
  ConvolverManager(const char *irfile, const char *irdelayfile, uint_t partitionsize);
  virtual ~ConvolverManager();

  /*--------------------------------------------------------------------------------*/
  /** Sets the expected impulse response length (for static convolvers ONLY)
   *  Should be called before creating convolvers, will clear all initialised ones
   *  if existing.
   *
   * @param irlength the length of the irs to be added
   *
   */
  /*--------------------------------------------------------------------------------*/
  void SetIRLength(ulong_t irlength);

  /*--------------------------------------------------------------------------------*/
  /** Create impulse responses (IRs) from sample data.
   *  IRs are sequential and data is contiguous.
   *
   * @param irdata pointer to impulse response data
   * @param numirs the number of impulse responses
   * @param irlength the length of each impulse response
   */
  /*--------------------------------------------------------------------------------*/
  void CreateIRs(const float *irdata, const uint_t numirs, const ulong_t irlength);

  /*--------------------------------------------------------------------------------*/
  /** Load IRs from a file (either WAV or SOFA if enabled).
   *
   * @param filename file containing IRs
   */
  /*--------------------------------------------------------------------------------*/
  void LoadIRs(const char *filename);

#if ENABLE_SOFA
  /*--------------------------------------------------------------------------------*/
  /** Load IR data from SOFA file (including delays if available)
   */
  /*--------------------------------------------------------------------------------*/
  bool LoadSOFA(const char *filename);
#endif

  /*--------------------------------------------------------------------------------*/
  /** Load IR data from WAV file
   */
  /*--------------------------------------------------------------------------------*/
  bool LoadIRsSndFile(const char *filename);

  /*--------------------------------------------------------------------------------*/
  /** Load IR delays from text file
   */
  /*--------------------------------------------------------------------------------*/
  void LoadIRDelays(const char *filename);

  /*--------------------------------------------------------------------------------*/
  /** Set IR delays
   */
  /*--------------------------------------------------------------------------------*/
  void SetIRDelays(const double *delays, const uint_t num_delays);

  /*--------------------------------------------------------------------------------*/
  /** Create a static convolver with the correct parameters for inclusion in this manager
   *
   * @param irdata pointer to impulse response data buffer
   * @param irlength length of the buffer in samples
   * @param delay a delay associated with the static convolver
   */
  /*--------------------------------------------------------------------------------*/
  void CreateStaticConvolver(const float *irdata, const ulong_t irlength, double delay);

  /*--------------------------------------------------------------------------------*/
  /** Set delay scaling to compensate for factors such as ITD
   */
  /*--------------------------------------------------------------------------------*/
  void SetDelayScale(double scale = 1.0) {delayscale = scale; updateparameters = true;}

  /*--------------------------------------------------------------------------------*/
  /** Enable/disable high quality delay processing
   */
  /*--------------------------------------------------------------------------------*/
  void EnableHQProcessing(bool enable = true) {hqproc = enable; updateparameters = true;}

  /*--------------------------------------------------------------------------------*/
  /** Set the number of convolvers to run - creates and destroys convolvers as necessary
   */
  /*--------------------------------------------------------------------------------*/
  void SetConvolverCount(uint_t nconvolvers);

  /*--------------------------------------------------------------------------------*/
  /** Select a IR for a particular convolver
   *
   * @param convolver convolver number 0 .. nconvolvers as set above
   * @param ir IR number 0 .. number of IRs loaded by LoadIRs()
   * @param level audio output level
   * @param delay additional delay to be applied to the convolver (in SAMPLES)
   *
   * @return true if IR selected
   */
  /*--------------------------------------------------------------------------------*/
  bool SelectIR(uint_t convolver, uint_t ir, double level = 1.0, double delay = 0.0);

  /*--------------------------------------------------------------------------------*/
  /** Perform convolution on all convolvers
   *
   * @param input input data array (inputchannels wide by partitionsize frames long) containing input data
   * @param output output data array (outputchannels wide by partitionsize frames long) for receiving output data
   * @param inputchannels number of input channels (>= nconvolvers * outputchannels) 
   * @param outputchannels number of output channels
   *
   * @note this kicks off nconvolvers parallel threads to do the processing - can be VERY CPU hungry!
   */
  /*--------------------------------------------------------------------------------*/
  void Convolve(const float *input, float *output, uint_t inputchannels, uint_t outputchannels);

  /*--------------------------------------------------------------------------------*/
  /** Get the number of IRs that have been loaded.
   *
   * @return number of IRs loaded
   */
  /*--------------------------------------------------------------------------------*/
  uint_t NumIRs() const;

  typedef struct {
    apf::conv::Convolver       *dynamicconvolver;
    apf::conv::StaticConvolver *staticconvolver;
  } APFConvolver;

protected:
  typedef apf::conv::Filter APFFilter;

  /*--------------------------------------------------------------------------------*/
  /** Update the parameters of an individual convolver
   *
   * @param convolver convolver number
   *
   * @note this function updates the filter, delay and HQ processing flag
   */
  /*--------------------------------------------------------------------------------*/
  virtual void UpdateConvolverParameters(uint_t convolver);

  typedef struct {
    uint_t irindex;
    double delay;
    double level;
  } PARAMETERS;

protected:
#if ENABLE_SOFA
  /*--------------------------------------------------------------------------------*/
  /** Load impulse reponse data from a SOFA file.
   *
   * @param file SOFA file object, opened for reading
   *
   */
  /*--------------------------------------------------------------------------------*/
  void LoadIRsSOFA(SOFA& file);

  /*--------------------------------------------------------------------------------*/
  /** Load delay data from a SOFA file.
   *
   * @param file SOFA file object, opened for reading
   *
   */
  /*--------------------------------------------------------------------------------*/
  void LoadDelaysSOFA(SOFA& file);
#endif

  uint_t                   blocksize;
  uint_t                   partitions;
  std::vector<Convolver *> convolvers;
  std::vector<APFFilter>   filters;
  std::vector<double>      irdelays;
  std::vector<PARAMETERS>  parameters;
  double                   delayscale;
  float                    audioscale;
  bool                     hqproc;
  bool                     updateparameters;
};

class Convolver {
public:
  // no public constructor
  virtual ~Convolver();

  // no public members - everything is driven by a ConvolverManager

protected:
  friend class ConvolverManager;

  typedef apf::conv::Filter APFFilter;

  /*--------------------------------------------------------------------------------*/
  /** Protected constructor so that only ConvolverManager can create convolvers
   */
  /*--------------------------------------------------------------------------------*/
  Convolver(uint_t _convindex, uint_t _blocksize, float _scale, double _delay = 0.0);

  /*--------------------------------------------------------------------------------*/
  /** Start convolution thread
   *
   * @param _input input buffer (assumed to by blocksize * inputchannels long)
   * @param inputchannels number of channels in _input
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void StartConvolution(const float *_input, uint_t inputchannels);

  /*--------------------------------------------------------------------------------*/
  /** Wait for end of convolution and mix output
   *
   * @param _output buffer to mix locally generated output to (assumed to by blocksize * outputchannels long)
   * @param outputchannels number of channels in _output
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void EndConvolution(float *_output, uint_t outputchannels);

  /*--------------------------------------------------------------------------------*/
  /** Set parameters and options for convolution
   *
   * @param level audio output level
   * @param delay audio delay (due to ITD and source delay) (in SAMPLES)
   * @param hqproc true for high-quality and CPU hungry processing
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetParameters(double level, double delay, bool hqproc);

  /*--------------------------------------------------------------------------------*/
  /** Stop processing thread
   */
  /*--------------------------------------------------------------------------------*/
  void StopThread();

protected:
  /*--------------------------------------------------------------------------------*/
  /** Processing thread entry point
   */
  /*--------------------------------------------------------------------------------*/
  static void *__Process(void *arg)
  {
    Convolver& convolver = *(Convolver *)arg;
    return convolver.Process();
  }

  virtual void *Process();
  virtual void Convolve(float *dest) {UNUSED_PARAMETER(dest);}

  std::string DebugHeader() const;

protected:
  ThreadBoolSignalObject   startsignal;
  ThreadBoolSignalObject   donesignal;
  pthread_t                thread;
  uint_t                   blocksize;
  uint_t                   convindex;
  float                    scale;

  volatile float           *input;
  volatile float           *output;
  volatile double          outputdelay;
  volatile double          outputlevel;
  volatile uint_t          maxadditionaldelay;
  volatile bool            hqproc;
  volatile bool            quitthread;
};

/*----------------------------------------------------------------------------------------------------*/

class DynamicConvolver : public Convolver {
public:
  // no public constructor
  virtual ~DynamicConvolver();

  // no public members - everything is driven by a ConvolverManager

protected:
  friend class ConvolverManager;
  typedef apf::conv::Convolver APFConvolver;

  /*--------------------------------------------------------------------------------*/
  /** Protected constructor so that only ConvolverManager can create convolvers
   */
  /*--------------------------------------------------------------------------------*/
  DynamicConvolver(uint_t _convindex, uint_t _blocksize, APFConvolver *_convolver, float _scale);
  
  /*--------------------------------------------------------------------------------*/
  /** Set IR filter for convolution
   *
   * @param newfilter new IR filter from ConvolverManager
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetFilter(const APFFilter& newfilter);
  
  /*--------------------------------------------------------------------------------*/
  /** Actually perform convolution on the input and store it in the provided buffer
   *
   * @param dest destination buffer
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Convolve(float *dest);

protected:
  APFConvolver    *convolver;
  const APFFilter *convfilter;

  volatile const APFFilter *filter;
};

/*----------------------------------------------------------------------------------------------------*/

class StaticConvolver : public Convolver {
public:
  // no public constructor
  virtual ~StaticConvolver();

  // no public members - everything is driven by a ConvolverManager

protected:
  friend class ConvolverManager;
  typedef apf::conv::StaticConvolver APFConvolver;

  /*--------------------------------------------------------------------------------*/
  /** Protected constructor so that only ConvolverManager can create convolvers
   */
  /*--------------------------------------------------------------------------------*/
  StaticConvolver(uint_t _convindex, uint_t _blocksize, APFConvolver *_convolver, float _scale, double _delay);

  /*--------------------------------------------------------------------------------*/
  /** Actually perform convolution on the input and store it in the provided buffer
   *
   * @param dest destination buffer
   *
   */
  /*--------------------------------------------------------------------------------*/
  virtual void Convolve(float *dest);

protected:
  APFConvolver *convolver;
};

BBC_AUDIOTOOLBOX_END

#endif /* CONVOLVER_H_ */
