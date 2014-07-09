/*
 * Convolver.cpp
 *
 *  Created on: May 13, 2014
 *      Author: chrisp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/time.h>
#include <unistd.h>

#include <sndfile.hh>
#include <apf/convolver.h>

#define DEBUG_LEVEL 1
#include "Convolver.h"
#include "SoundFormatConversions.h"
#include "FractionalSample.h"

using namespace std;

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Constructor for convolver manager
 *
 * @param irfile file containing IRs (either WAV or SOFA) - SOFA file can also contain delays
 * @param partitionsize the convolution partition size - essentially the block size of the processing
 *
 */
/*--------------------------------------------------------------------------------*/
ConvolverManager::ConvolverManager(uint_t partitionsize) :
  blocksize(partitionsize),
  partitions(0),
  delayscale(1.0),
  audioscale(1.f),
  hqproc(true),
  updateparameters(true)
{
}

/*--------------------------------------------------------------------------------*/
/** Constructor for convolver manager
 *
 * @param irfile file containing IRs (either WAV or SOFA) - SOFA file can also contain delays
 * @param partitionsize the convolution partition size - essentially the block size of the processing
 *
 */
/*--------------------------------------------------------------------------------*/
ConvolverManager::ConvolverManager(const char *irfile, uint_t partitionsize) :
  blocksize(partitionsize),
  partitions(0),
  delayscale(1.0),
  audioscale(1.f),
  hqproc(true),
  updateparameters(true)
{
  LoadIRs(irfile);
}

/*--------------------------------------------------------------------------------*/
/** Constructor for convolver manager
 *
 * @param irfile file containing IRs (either WAV or SOFA if enabled)
 * @param irdelayfile text file containing the required delays (in SAMPLES) of each IR in irfile
 * @param partitionsize the convolution partition size - essentially the block size of the processing
 *
 */
/*--------------------------------------------------------------------------------*/
ConvolverManager::ConvolverManager(const char *irfile, const char *irdelayfile, uint_t partitionsize) :
  blocksize(partitionsize),
  partitions(0),
  delayscale(1.0),
  audioscale(1.f),
  hqproc(true),
  updateparameters(true)
{
  LoadIRs(irfile);
  LoadIRDelays(irdelayfile);
}

ConvolverManager::~ConvolverManager()
{
  // delete all convolvers
  while (convolvers.size()) {
    Convolver *conv = convolvers.back();
    delete conv;
    convolvers.pop_back();
  }
}

/*--------------------------------------------------------------------------------*/
/** Create a dynamic convolver with the correct parameters for inclusion in this manager
 */
/*--------------------------------------------------------------------------------*/
ConvolverManager::APFConvolver ConvolverManager::CreateConvolver() const
{
  APFConvolver convolver;

  memset(&convolver, 0, sizeof(convolver));
  convolver.dynamicconvolver = new apf::conv::Convolver(blocksize, partitions);

  return convolver;
}

/*--------------------------------------------------------------------------------*/
/** Create a static convolver with the correct parameters for inclusion in this manager
 *
 * @param irdate pointer to impulse response data buffer
 * @param irlength length of the buffer in samples
 */
/*--------------------------------------------------------------------------------*/
ConvolverManager::APFConvolver ConvolverManager::CreateConvolver(const float *irdata, const ulong_t irlength)
{
  APFConvolver convolver;

  memset(&convolver, 0, sizeof(convolver));
  convolver.staticconvolver = new apf::conv::StaticConvolver(blocksize, irdata, irdata + irlength);

  return convolver;
}

/*--------------------------------------------------------------------------------*/
/** Sets the expected impulse response length (for static convolvers ONLY)
 *  Should be called before creating convolvers, will clear all initialised ones
 *  if existing.
 *
 * @param irlength the length of the irs to be added
 *
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::SetIRLength(ulong_t irlength)
{
  partitions = (uint_t)((irlength + blocksize - 1) / blocksize);

  if (convolvers.size()) DEBUG1(("Warning: removing existing static convolvers."));

  while (convolvers.size()) {
    Convolver *conv = convolvers.back();
    delete conv;
    convolvers.pop_back();
  }

  parameters.clear();
}

/*--------------------------------------------------------------------------------*/
/** Create impulse responses (IRs) from sample data.
 *  IRs are sequential and data is contiguous.
 *
 * @param irdata pointer to impulse response data
 * @param numirs the number of impulse responses
 * @param irlength the length of each impulse response
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::CreateIRs(const float *irdata, uint_t numirs, const ulong_t irlength)
{
  uint_t i;

  filters.clear();

  if (numirs && irlength)
  {
    partitions = (uint_t)((irlength + blocksize - 1) / blocksize);

    DEBUG2(("File '%s' is %lu samples long, therefore %u partitions are needed", filename, irlength, partitions));

    apf::conv::Convolver convolver(blocksize, partitions);
    uint32_t tick = GetTickCount();

    DEBUG2(("Creating %u filters...", n));

    // create array to take an IR (which won't be full length because we've rounded up to a whole number of partitions)
    float *ir;
    if ((ir = new float[blocksize * partitions]) != NULL)
    {
      memset(ir, 0, blocksize * partitions * sizeof(*ir));

      for (i = 0; i < numirs; i++)
      {
        DEBUG5(("Creating filter for IR %u", i));

        filters.push_back(APFFilter(blocksize, partitions));
        memcpy(ir, (irdata + i * irlength), irlength * sizeof(*ir));
        convolver.prepare_filter(ir, (ir+irlength), filters[i]);
      }

      DEBUG2(("Finished creating filters (took %lums)", (ulong_t)(GetTickCount() - tick)));
#if DEBUG_LEVEL < 2
      UNUSED_PARAMETER(tick);
#endif

      delete[] ir;
    }

    // force update of parameters
    updateparameters = true;
  }
}

/*--------------------------------------------------------------------------------*/
/** Load IRs from a file (either WAV or SOFA if enabled).
 *
 * @param filename file containing IRs
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::LoadIRs(const char *filename)
{
  if (filename) {
#if ENABLE_SOFA
    // only attempt to load file as SOFA if suffix is .sofa (otherwise app bombs out on none SOFA files)
    if ((strlen(filename) >= 5) && (strcasecmp(filename + strlen(filename) - 5, ".sofa") == 0) && LoadSOFA(filename))
    {
      DEBUG3(("Loaded IRs from SOFA file (%s).", filename));
    }
    else
#endif
      if (LoadIRsSndFile(filename)) {
        DEBUG3(("Loaded IRs from WAV file (%s).", filename));
      } else {
        ERROR("Failed to load IRs from file (%s).", filename);
      }
  }
  else
  {
    ERROR("Invalid filename for IR file ('%s')", filename);
  }
}

#if ENABLE_SOFA
/*--------------------------------------------------------------------------------*/
/** Load impulse reponse data from a SOFA file.
 *
 * @param file SOFA file object, opened for reading
 *
 * @note Receiver measurements are interleaved.
 */
/*--------------------------------------------------------------------------------*/
bool ConvolverManager::LoadSOFA(const char *filename)
{
  filters.clear();

  if (filename) {
    SOFA file(filename);

    if (file) {
      DEBUG3(("Opened '%s' okay, %u measurements from %u sources at %luHz", filename, file.get_num_measurements(), file.get_num_emitters(), (ulong_t)file.get_samplerate()));
      LoadIRsSOFA(file);
      LoadDelaysSOFA(file);
      return true;
    }
    else
    {
      ERROR("SOFA file is invalid");
    }
  }
  else
  {
    ERROR("Invalid filename for IR file ('%s')", filename);
  }
  return false;
}
#endif

/*--------------------------------------------------------------------------------*/
/** Load IR files from a WAV file
 *
 * @param filename name of a WAV file containing IRs
 */
/*--------------------------------------------------------------------------------*/
bool ConvolverManager::LoadIRsSndFile(const char *filename)
{
  filters.clear();

  if (filename) {
    SndfileHandle file(filename);

    if (file && file.frames() && file.channels()) {
      ulong_t len = file.frames();
      uint_t i, n = file.channels();

      DEBUG3(("Opened '%s' okay, %u channels at %luHz", filename, n, (ulong_t)file.samplerate()));

      partitions = (uint_t)((len + blocksize - 1) / blocksize);

      DEBUG2(("File '%s' is %lu samples long, therefore %u partitions are needed", filename, len, partitions));

      apf::conv::Convolver convolver(blocksize, partitions);
      float *sampledata = new float[blocksize * partitions * n]; 
      float *response   = new float[blocksize * partitions];
      slong_t res;

      memset(sampledata, 0, blocksize * partitions * n * sizeof(*sampledata));

      DEBUG2(("Reading sample data..."));

      if ((res = file.read(sampledata, blocksize * partitions * n)) < 0) {
        ERROR("Read of %u frames result: %ld", blocksize * partitions, res);
      }

      DEBUG2(("Creating %u filters...", n));
      uint32_t tick = GetTickCount();
      for (i = 0; i < n; i++) {
        DEBUG5(("Creating filter for IR %u", i));

        filters.push_back(APFFilter(blocksize, partitions));

        TransferSamples(sampledata, i, n, response, 0, 1, 1, blocksize * partitions);

        convolver.prepare_filter(response, response + blocksize * partitions, filters[i]);
      }
      DEBUG2(("Finished creating filters (took %lums)", (ulong_t)(GetTickCount() - tick)));
#if DEBUG_LEVEL < 2
      UNUSED_PARAMETER(tick);
#endif

      delete[] sampledata;
      delete[] response;

      // force update of parameters
      updateparameters = true;
      return true;
    }
    else
    {
      ERROR("Failed to open IR file ('%s') for reading", filename);
    }
  }
  else
  {
    ERROR("Invalid filename for IR file ('%s')", filename);
  }
  return false;
}

/*--------------------------------------------------------------------------------*/
/** Load IR delays from text file
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::LoadIRDelays(const char *filename)
{
  irdelays.clear();

  if (filename) {
    FILE *fp;

    DEBUG2(("Reading IR delays from '%s'", filename));

    if ((fp = fopen(filename, "r")) != NULL) {
      double mindelay = 0.0;  // used to reduce delays to minimum
      double delay = 0.0;
      uint_t i;
      bool   initialreading = true;

      while (fscanf(fp, "%lf", &delay) > 0) {
        irdelays.push_back(delay);

        // update minimum delay if necessary
        if (initialreading || (delay < mindelay)) mindelay = delay;

        initialreading = false;
      }

      // reduce each delay by mindelay to reduce overall delay
      // TODO: this isn't always acceptable behaviour
      for (i = 0; i < irdelays.size(); i++) {
        irdelays[i] -= mindelay;
      }

      fclose(fp);

      // force update of parameters
      updateparameters = true;
    }
    else DEBUG1(("Failed to open IR delays file ('%s') for reading, zeroing delays", filename));
  }
}

/*--------------------------------------------------------------------------------*/
/** Set IR delays
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::SetIRDelays(const double *delays, const uint_t num_delays)
{
  irdelays.clear();

  if (num_delays)
  {
    uint_t i;

    irdelays.resize(num_delays);

    // reduce each delay by mindelay to reduce overall delay
    for (i = 0; i < irdelays.size(); i++) {
      irdelays[i] = delays[i];
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Create a static convolver with the correct parameters for inclusion in this manager
 *
 * @param irdate pointer to impulse response data buffer
 * @param irlength length of the buffer in samples
 * @param delay a delay associated with the static convolver
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::CreateStaticConvolver(const float *irdata, const ulong_t irlength, double delay)
{
  PARAMETERS params;
  Convolver *conv;

  memset(&params, 0, sizeof(params));
  params.delay = delay;
  parameters.push_back(params);

  // set up new convolver
  if ((conv = new Convolver(convolvers.size(), blocksize, CreateConvolver(irdata, irlength), audioscale, delay)) != NULL) {
    convolvers.push_back(conv);
  }
}

/*--------------------------------------------------------------------------------*/
/** Set the number of convolvers to run - creates and destroys convolvers as necessary
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::SetConvolverCount(uint_t nconvolvers)
{
  // update parameters array size
  parameters.resize(nconvolvers);

  // create convolvers if necessary
  while (convolvers.size() < nconvolvers) {
    Convolver *conv;

    // set up new convolver
    if ((conv = new Convolver(convolvers.size(), blocksize, CreateConvolver(), audioscale)) != NULL) {
      convolvers.push_back(conv);

      // set default IR
      SelectIR(convolvers.size() - 1, 0);
    }
    else {
      ERROR("Failed to create new convolver!");
      break;
    }
  }

  // delete excessive convolvers
  while (convolvers.size() > nconvolvers) {
    Convolver *conv = convolvers.back();
    delete conv;
    convolvers.pop_back();
  }
}

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
bool ConvolverManager::SelectIR(uint_t convolver, uint_t ir, double level, double delay)
{
  bool success = false;

  if (convolver < convolvers.size())
  {
    if (ir < filters.size())
    {
      DEBUG4(("[%010lu]: Selecting IR %03u for convolver %3u", (ulong_t)GetTickCount(), ir, convolver));

      // store parameters for convolver
      parameters[convolver].irindex = ir;
      parameters[convolver].level   = level;
      parameters[convolver].delay   = delay;

      // update the parameters of the convolver
      UpdateConvolverParameters(convolver);

      success = true;
    }
    else ERROR("Out-of-bounds IR %u requested", ir);
  }
  else ERROR("Out-of-bounds convolver %u requested", convolver);

  return success;
}

/*--------------------------------------------------------------------------------*/
/** Update the parameters of an individual convolver
 *
 * @param convolver convolver number
 *
 * @note this function updates the filter, delay and HQ processing flag
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::UpdateConvolverParameters(uint_t convolver)
{
  if (convolver < convolvers.size())
  {
    const PARAMETERS& params = parameters[convolver];
    uint_t ir = params.irindex;

    if (ir < filters.size())
    {
      // if a delay is available for this IR, subtract minimum delay and scale it by delayscale
      // to compensate for ITD
      double delay = (ir < irdelays.size()) ? irdelays[ir] * delayscale : 0.0;

      // pass parameters to convolver, add additional delay to scaled delay due to IR's
      convolvers[convolver]->SetParameters(filters[ir], params.level, delay + params.delay, hqproc);
    }
  }
}

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
void ConvolverManager::Convolve(const float *input, float *output, uint_t inputchannels, uint_t outputchannels)
{
  uint_t i;

  // ASSUMES output is clear before being called

  // start parallel convolving on all channels
  for (i = 0; i < convolvers.size(); i++)
  {
    // if requested to, update the parameters of this convolver
    if (updateparameters) UpdateConvolverParameters(i);

    DEBUG5(("Starting convolver %u/%u...", i + 1, (uint_t)convolvers.size()));
    convolvers[i]->StartConvolution(input + i / outputchannels, inputchannels);
    DEBUG5(("Convolver %u/%u started", i + 1, (uint_t)convolvers.size()));
  }

  // clear flag
  updateparameters = false;

  // now process outputs
  for (i = 0; i < convolvers.size(); i++)
  {
    DEBUG5(("Waiting on convolver %u/%u to complete...", i + 1, (uint_t)convolvers.size()));
    convolvers[i]->EndConvolution(output + (i % outputchannels), outputchannels);
    DEBUG5(("Convolver %u/%u completed", i + 1, (uint_t)convolvers.size()));
  }
}

/*--------------------------------------------------------------------------------*/
/** Get the number of IRs that have been loaded.
 *
 * @return number of IRs loaded
 */
/*--------------------------------------------------------------------------------*/
uint_t ConvolverManager::NumIRs() const
{
  return filters.size();
}

#if ENABLE_SOFA
/*--------------------------------------------------------------------------------*/
/** Load impulse reponse data from a SOFA file.
 *
 * @param file SOFA file object, opened for reading
 *
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::LoadIRsSOFA(SOFA& file)
{
  // load impulse responses
  ulong_t len = file.get_ir_length();
  uint_t i, j, k, l, m = file.get_num_measurements(), r = file.get_num_receivers(), e = file.get_num_emitters();

  partitions = (uint_t)((len + blocksize - 1) / blocksize);

  DEBUG2(("File is %lu samples long, therefore %u partitions are needed", len, partitions));

  apf::conv::Convolver convolver(blocksize, partitions);
  float *response   = new float[blocksize * partitions];
  float *ir         = new float[len];

  DEBUG2(("Creating %u filters...", n));
  uint32_t tick = GetTickCount();

  for (k = l = 0; k < e; k++)
  {
    for (i = 0; i < m; i++)
    {
      for (j = 0; j < r; j++, l++)
      {
        DEBUG3(("Creating filter for IR %u", l));
        filters.push_back(APFFilter(blocksize, partitions));

        if (!file.get_ir(ir, i, j, k)) ERROR("Error reading IR measurement %u for receiver %u and emitter %u from SOFA file", i, j, k);
        TransferSamples(ir, 0, 1, response, 0, 1, 1, blocksize * partitions);
        convolver.prepare_filter(response, response + blocksize * partitions, filters[l]);
      }
    }
  }
  DEBUG2(("Finished creating filters (took %lums)", (ulong_t)(GetTickCount() - tick)));
#if DEBUG_LEVEL < 2
  UNUSED_PARAMETER(tick);
#endif

  delete[] response;
  delete[] ir;

  // force update of parameters
  updateparameters = true;
}

/*--------------------------------------------------------------------------------*/
/** Load delay data from a SOFA file.
 *
 * @param file SOFA file object, opened for reading
 *
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::LoadDelaysSOFA(SOFA& file)
{
  irdelays.clear();

  // get number of measurements and receivers
  uint_t i, j, k, m = file.get_num_measurements(), r = file.get_num_receivers(), e = file.get_num_emitters(), n = m*r*e;

  // read delays for each receiver and insert into irdelays interleaved
  irdelays.resize(n);
  DEBUG2(("Loading %u delays from SOFA file", n));
  SOFA::delay_buffer_t delays_recv;
  float sr = file.get_samplerate();
  for (k = 0; k < e; k++)
  {
    for (j=0; j<r; j++)
    {
      file.get_delays(delays_recv, j, k);
      for (i=0; i<m; i++)
      {
        if (delays_recv.size() == 1) irdelays[k*m*r + i*r + j] = delays_recv[0]*sr;
        else                         irdelays[k*m*r + i*r + j] = delays_recv[i]*sr;
      }
    }
  }

  // force update of parameters
  updateparameters = true;
}
#endif

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Protected constructor so that only ConvolverManager can create convolvers
 */
/*--------------------------------------------------------------------------------*/
Convolver::Convolver(uint_t _convindex, uint_t _blocksize, const APFConvolver& _convolver, float _scale, double _delay) :
  thread(0),
  convolver(_convolver),
  blocksize(_blocksize),
  convindex(_convindex),
  scale(_scale),
  filter(NULL),
  input(new float[blocksize]),
  output(new float[blocksize]),
  outputdelay(_delay),
  outputlevel(1.0),
  maxadditionaldelay(2400),
  quitthread(false)
{
  // create thread
  if (pthread_create(&thread, NULL, &__Process, (void *)this) != 0) {
    ERROR("Failed to create thread (%s)", strerror(errno));
    thread = 0;
  }
}

Convolver::~Convolver()
{
  StopThread();

  if (convolver.dynamicconvolver) delete convolver.dynamicconvolver;
  if (convolver.staticconvolver)  delete convolver.staticconvolver;

  if (input)  delete[] input;
  if (output) delete[] output;
}

string Convolver::DebugHeader() const
{
  static const string column = "                    ";
  static uint32_t tick0 = GetTickCount();
  string res = "";
  uint_t i;

  Printf(res, "%06lu (%02u): ", (ulong_t)(GetTickCount() - tick0), convindex);

  for (i = 0; i < convindex; i++) res += column;

  return res;
}

/*--------------------------------------------------------------------------------*/
/** Stop processing thread
 */
/*--------------------------------------------------------------------------------*/
void Convolver::StopThread()
{
  if (thread) {
    quitthread = true;

    // release thread
    startsignal.Signal();

    // wait until thread has finished
    pthread_join(thread, NULL);
    thread = 0;
  }
}

/*--------------------------------------------------------------------------------*/
/** Start convolution thread
 *
 * @param _input input buffer (assumed to by blocksize * inputchannels long)
 * @param inputchannels number of channels in _input
 *
 */
/*--------------------------------------------------------------------------------*/
void Convolver::StartConvolution(const float *_input, uint_t inputchannels)
{
  uint_t i;

  // copy input (de-interleaving)
  for (i = 0; i < blocksize; i++) {
    input[i] = _input[i * inputchannels];
  }

  memcpy((float *)output, (const float *)input, blocksize * sizeof(*input));

  DEBUG4(("%smain signal", DebugHeader().c_str()));

  // release thread
  startsignal.Signal();
}

/*--------------------------------------------------------------------------------*/
/** Wait for end of convolution and mix output
 *
 * @param _output buffer to mix locally generated output to (assumed to by blocksize * outputchannels long)
 * @param outputchannels number of channels in _output
 *
 */
/*--------------------------------------------------------------------------------*/
void Convolver::EndConvolution(float *_output, uint_t outputchannels)
{
  DEBUG4(("%smain wait", DebugHeader().c_str()));

  // wait for completion
  donesignal.Wait();

  DEBUG4(("%smain done", DebugHeader().c_str()));

  // mix locally generated output into supplied output buffer
  uint_t i;
  for (i = 0; i < blocksize; i++) {
    _output[i * outputchannels] += output[i];
  }
}

/*--------------------------------------------------------------------------------*/
/** Processing thread
 */
/*--------------------------------------------------------------------------------*/
void *Convolver::Process()
{
  const APFFilter *convfilter = NULL;
  uint_t maxdelay = maxadditionaldelay;       // maximum delay in samples
  uint_t delaypos = 0;
  // delay length is maxdelay plus blocksize samples then rounded up to a whole number of blocksize's
  uint_t delaylen = (1 + ((maxdelay + blocksize - 1) / blocksize)) * blocksize;
  float  *delay   = new float[delaylen];      // delay memory
  double level1   = 1.0;
  double delay1   = 0.0;

  // maxdelay can be extended now because of the rounding up of delaylen
  maxdelay = delaylen - blocksize - 1 - FractionalSampleAdditionalDelayRequired();

  // clear delay memory
  memset(delay, 0, delaylen * sizeof(*delay));

  while (!quitthread)
  {
    uint_t i;

    DEBUG4(("%sproc wait", DebugHeader().c_str()));

    // wait to be released
    startsignal.Wait();

    DEBUG4(("%sproc start", DebugHeader().c_str()));
    
    // detect quit request
    if (quitthread) break;

    if (convolver.dynamicconvolver)
    {
      apf::conv::Convolver *conv = convolver.dynamicconvolver;

      // add input to convolver
      conv->add_block(input);

      // do convolution
      const float *result = conv->convolve(scale);
      float       *dest   = delay + delaypos;
            
      // copy data into delay memory
      memcpy(dest, result, blocksize * sizeof(*delay));

      // if filter needs updating, update it now
      if (filter && (filter != convfilter)) {
        convfilter = (const APFFilter *)filter;
        conv->set_filter(*convfilter);
        DEBUG3(("[%010lu]: Selected new filter for convolver %3u", (ulong_t)GetTickCount(), convindex));
      }

      // if queues_empty() returns false, must cross fade between convolutions
      if (!conv->queues_empty()) {
        // rotate queues within convolver
        conv->rotate_queues();

        // do convolution a second time
        result = conv->convolve(scale);

        // crossfade new convolution result into buffer
        for (i = 0; i < blocksize; i++) {
          double b = (double)i / (double)blocksize, a = 1.0 - b;

          dest[i] = dest[i] * a + b * result[i];
        }
      }
    }
    else if (convolver.staticconvolver)
    {
      apf::conv::StaticConvolver *conv = convolver.staticconvolver;

      // add input to convolver
      conv->add_block(input);

      // do convolution
      const float *result = conv->convolve(scale);
      float       *dest   = delay + delaypos;
            
      // copy data into delay memory
      memcpy(dest, result, blocksize * sizeof(*delay));
    }

    // process delay memory using specified delay
    uint_t pos1   = delaypos + delaylen;
    double level2 = outputlevel;
    double delay2 = MIN(outputdelay, (double)maxdelay);
    double fpos1  = (double)pos1               - delay1;
    double fpos2  = (double)(pos1 + blocksize) - delay2;

    if (hqproc)
    {
      // high quality processing - use SRC filter to generate samples with fractional delays
      for (i = 0; i < blocksize; i++)
      {
        double b     = (double)i / (double)blocksize, a = 1.0 - b;
        double fpos  = a * fpos1  + b * fpos2;
        double level = a * level1 + b * level2;

        output[i] = level * FractionalSample(delay, 0, 1, delaylen, fpos);
      }
    }
    else
    {
      // low quality processing - just use integer delays without SRC filter
      for (i = 0; i < blocksize; i++)
      {
        double b     = (double)i / (double)blocksize, a = 1.0 - b;
        double fpos  = a * fpos1  + b * fpos2;
        double level = a * level1 + b * level2;

        output[i] = level * delay[(uint_t)fpos % delaylen];
      }
    }

    // advance delay position by a block
    delaypos = (delaypos + blocksize) % delaylen;
    delay1   = delay2;
    level1   = level2;

    DEBUG4(("%sproc done", DebugHeader().c_str()));

    // signal done
    donesignal.Signal();
  }

  pthread_exit(NULL);

  return NULL;
}

/*--------------------------------------------------------------------------------*/
/** Set parameters and options for convolution
 *
 * @param newfilter new IR filter from ConvolverManager
 * @param level audio output level
 * @param delay audio delay (due to ITD and source delay) (in SAMPLES)
 * @param hqproc true for high-quality and CPU hungry processing
 */
/*--------------------------------------------------------------------------------*/
void Convolver::SetParameters(const APFFilter& newfilter, double level, double delay, bool hqproc)
{
  if (&newfilter != filter) {
    DEBUG3(("[%010lu]: Selecting new filter for convolver %3u", (ulong_t)GetTickCount(), convindex));
    // set convolver filter
    filter = &newfilter;
  }

  // update processing parameters
  outputlevel  = level;
  outputdelay  = delay;
  this->hqproc = hqproc;
}

/*--------------------------------------------------------------------------------*/
/** Set parameters and options for convolution
 *
 * @param level audio output level
 * @param hqproc true for high-quality and CPU hungry processing
 */
/*--------------------------------------------------------------------------------*/
void Convolver::SetParameters(double level, bool hqproc)
{
  // update processing parameters
  outputlevel  = level;
  this->hqproc = hqproc;
}

BBC_AUDIOTOOLBOX_END
