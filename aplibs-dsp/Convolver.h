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

// APF convolver header files cannot be included in header files because of function implementations in the header files
// which lead to multiple definitions when linking!
// So this acts as a prototype for the required objects from convolver.h
namespace apf {
  namespace conv {
    struct Convolver;
    struct Filter;
  }
}

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** ConvolverManager acts as manager for thr individual convolvers, it holds all the
 * impulse responses (as frequency filters) and creates and destroys convolver objects
 *
 */
/*--------------------------------------------------------------------------------*/
class Convolver;
class ConvolverManager {
public:
  /*--------------------------------------------------------------------------------*/
  /** Constructor for convolver manager
   *
   * @param irfile WAV file containing IRs
   * @param irdelayfile text file containing the required delays (in SAMPLES) of each IR in irfile
   * @param partitionsize the convolution partition size - essentially the block size of the processing
   *
   */
  /*--------------------------------------------------------------------------------*/
  ConvolverManager(const char *irfile, const char *irdelayfile, uint_t partitionsize);
  virtual ~ConvolverManager();

  /*--------------------------------------------------------------------------------*/
  /** Load IR files from WAV file
   */
  /*--------------------------------------------------------------------------------*/
  void LoadIRs(const char *filename);

  /*--------------------------------------------------------------------------------*/
  /** Load IR delays from text file
   */
  /*--------------------------------------------------------------------------------*/
  void LoadIRDelays(const char *filename);

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

protected:
  typedef apf::conv::Convolver APFConvolver;
  typedef apf::conv::Filter    APFFilter;

  /*--------------------------------------------------------------------------------*/
  /** Create a convolver with the correct parameters for inclusion in this manager
   */
  /*--------------------------------------------------------------------------------*/
  APFConvolver *CreateConvolver() const;

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
  virtual ~Convolver();

  // no public members - everything is driven by a ConvolverManager

protected:
  friend class ConvolverManager;

  typedef apf::conv::Convolver APFConvolver;
  typedef apf::conv::Filter    APFFilter;

  /*--------------------------------------------------------------------------------*/
  /** Protected constructor so that only ConvolverManager can create convolvers
   */
  /*--------------------------------------------------------------------------------*/
  Convolver(uint_t _convindex, uint_t _blocksize, APFConvolver *_convolver, float _scale);

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
   * @param newfilter new IR filter from ConvolverManager
   * @param level audio output level
   * @param delay audio delay (due to ITD and source delay) (in SAMPLES)
   * @param hqproc true for high-quality and CPU hungry processing
   */
  /*--------------------------------------------------------------------------------*/
  virtual void SetParameters(const APFFilter& newfilter, double level, double delay, bool hqproc);

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
  static void *__Process(void *arg) {
    Convolver& convolver = *(Convolver *)arg;
    return convolver.Process();
  }

  virtual void *Process();

  std::string DebugHeader() const;

protected:
  ThreadBoolSignalObject   startsignal;
  ThreadBoolSignalObject   donesignal;
  pthread_t                thread;
  APFConvolver             *convolver;
  uint_t                   blocksize;
  uint_t                   convindex;
  float                    scale;

  volatile const APFFilter *filter;
  volatile float           *input;
  volatile float           *output;
  volatile double          outputdelay;
  volatile double          outputlevel;
  volatile uint_t          maxadditionaldelay;
  volatile bool            hqproc;
  volatile bool            quitthread;
};

BBC_AUDIOTOOLBOX_END

#endif /* CONVOLVER_H_ */
