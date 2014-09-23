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

#define DEBUG_LEVEL 2
#include "Convolver.h"
#include "SoundFormatConversions.h"
#include "FractionalSample.h"
#include "PerformanceMonitor.h"

// set at 1 to output whether each convolver is processing every 2s
#define DEBUG_CONVOLVER_STATES   0
#define MEASURE_MAX_FILTER_LEVEL 0

BBC_AUDIOTOOLBOX_START

const ConvolverManager::FILTER_FADE ConvolverManager::defaultfade = {0.0, 0.0, 0.0, 0.0};

/*--------------------------------------------------------------------------------*/
/** Constructor for convolver manager
 *
 * @param partitionsize the convolution partition size - essentially the block size of the processing
 *
 */
/*--------------------------------------------------------------------------------*/
ConvolverManager::ConvolverManager(uint_t partitionsize) :
  blocksize(partitionsize),
  partitions(0),
  delayscale(1.0),
  audioscale(1.0),
  hqproc(true),
  updateparameters(true)
{
  reporttick = GetTickCount();
}

/*--------------------------------------------------------------------------------*/
/** Constructor for convolver manager
 *
 * @param irfile file containing IRs (either WAV or SOFA) - SOFA file can also contain delays
 * @param partitionsize the convolution partition size - essentially the block size of the processing
 * @param fade fade information to allow a subset of the data to be used
 *
 */
/*--------------------------------------------------------------------------------*/
ConvolverManager::ConvolverManager(const char *irfile, uint_t partitionsize, const FILTER_FADE& fade) :
  blocksize(partitionsize),
  partitions(0),
  delayscale(1.0),
  audioscale(1.f),
  hqproc(true),
  updateparameters(true)
{
  reporttick = GetTickCount();
  LoadIRs(irfile, fade);
}

/*--------------------------------------------------------------------------------*/
/** Constructor for convolver manager
 *  The delays in irdelayfile will overwrite any specified within irfile if
 *  irfile is a SOFA file.
 *
 * @param irfile file containing IRs (either WAV or SOFA if enabled)
 * @param irdelayfile text file containing the required delays (in SAMPLES) of each IR in irfile
 * @param partitionsize the convolution partition size - essentially the block size of the processing
 * @param fade fade information to allow a subset of the data to be used
 *
 */
/*--------------------------------------------------------------------------------*/
ConvolverManager::ConvolverManager(const char *irfile, const char *irdelayfile, uint_t partitionsize, const FILTER_FADE& fade) :
  blocksize(partitionsize),
  partitions(0),
  delayscale(1.0),
  audioscale(1.f),
  hqproc(true),
  updateparameters(true)
{
  reporttick = GetTickCount();
  LoadIRs(irfile, fade);
  LoadIRDelays(irdelayfile);
}

ConvolverManager::~ConvolverManager()
{
  // delete all convolvers
  while (convolvers.size())
  {
    Convolver *conv = convolvers.back();
    delete conv;
    convolvers.pop_back();
  }
}

/*--------------------------------------------------------------------------------*/
/** Calculate number of partitions required for specified filter length and fade parameters
 */
/*--------------------------------------------------------------------------------*/
uint_t ConvolverManager::CalcPartitions(const FILTER_FADE& fade, double samplerate, uint_t filterlen, uint_t blocksize, uint_t& start, uint_t& len)
{
  // work out start sample and number of samples required for filter given the supplied fade information
  start = (uint_t)floor(MAX(fade.fade_in_start, 0.0) * samplerate);

  if ((fade.fade_out_start + fade.fade_out_length) == 0.0) len = filterlen - start;
  else
  {
    len = (uint_t)ceil(MAX(fade.fade_out_start + fade.fade_out_length - fade.fade_in_start, 0.0) * samplerate);
    len = MIN(len, filterlen - start);
  }

  DEBUG2(("From fade structure (start %0.3lfs fade-in %0.3lfs end %0.3lfs fade-out %0.3lfs), filter length %u samples and sample rate of %0.0lfHz, filter start is %u samples, length %u samples",
          fade.fade_in_start, fade.fade_in_length,
          fade.fade_out_start, fade.fade_out_length,
          filterlen, samplerate,
          start, len));

  // calculate the number of partitions required for the calculated filter length
  return (len + blocksize - 1) / blocksize;
}

/*--------------------------------------------------------------------------------*/
/** Create fades from fade profile
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::CreateFades(const FILTER_FADE& fade, double samplerate, std::vector<float>& fadein, std::vector<float>& fadeout)
{
  uint_t i, len = (uint_t)ceil(fade.fade_in_length * samplerate);

  fadein.resize(len);

  DEBUG2(("Filter fade in is %u samples", len));

  if (len) {
    double scale = 1.0 / (fade.fade_in_length * samplerate);

    for (i = 0; i < len; i++)
    {
      double v = MIN((double)i * scale, 1.0);           // ramp
      fadein[i] = (float)(.5 - .5 * cos(v * M_PI));     // raised cosine profile
    }
  }

  len = (uint_t)ceil(fade.fade_out_length * samplerate);

  fadeout.resize(len);

  DEBUG2(("Filter fade out is %u samples", len));

  if (len) {
    double scale = 1.0 / (fade.fade_out_length * samplerate);

    for (i = 0; i < len; i++)
    {
      double v = MIN((double)i * scale, 1.0);           // ramp
      // NOTE: fade out is stored backwards!
      fadeout[i] = (float)(.5 - .5 * cos(v * M_PI));    // raised cosine profile
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Apply fade in and fade out to data
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::ApplyFades(float *data, uint_t len, const std::vector<float>& fadein, const std::vector<float>& fadeout)
{
  uint_t i;

  // apply fade in
  DEBUG3(("Applying fade-in of %u samples", (uint_t)fadein.size()));
  for (i = 0; i < fadein.size(); i++)  data[i]           *= fadein[i];

  // apply fade out at end of filter (note that the fade MUST be stored backwards for this to work)
  DEBUG3(("Applying fade-out of %u samples", (uint_t)fadeout.size()));
  for (i = 0; i < fadeout.size(); i++) data[len - 1 - i] *= fadeout[i];
}

/*--------------------------------------------------------------------------------*/
/** Create impulse responses (IRs) from sample data.
 *  IRs are sequential and data is contiguous.
 *
 * @param irdata pointer to impulse response data
 * @param numirs the number of impulse responses
 * @param irlength the length of each impulse response
 * @param fade fade information to allow a subset of the data to be used
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::CreateIRs(const float *irdata, uint_t numirs, const uint_t irlength, const FILTER_FADE& fade)
{
  uint_t i;

  filters.clear();

  if (numirs && irlength)
  {
    std::vector<float> fadein, fadeout;
    std::vector<float> buffer;
    double samplerate = 48000.0;
    uint_t filterstart, filterlen;

    partitions = CalcPartitions(fade, samplerate, irlength, blocksize, filterstart, filterlen);
    CreateFades(fade, samplerate, fadein, fadeout);

    buffer.resize(filterlen);

    DEBUG2(("IRs are %u samples, therefore %u partitions are needed", filterlen, partitions));

    apf::conv::Convolver convolver(blocksize, partitions);
    ulong_t tick = GetTickCount();

#if MEASURE_MAX_FILTER_LEVEL
    float   maxlevel = 0.f;
#endif

    // create array to take an IR (which won't be full length because we've rounded up to a whole number of partitions)
    DEBUG2(("Creating %u filters...", numirs));

    for (i = 0; i < numirs; i++)
    {
      const float *irsrc = irdata + i * irlength + filterstart;
      float *irdata1 = &buffer[0];

      DEBUG5(("Creating filter for IR %u", i));

      filters.push_back(APFFilter(blocksize, partitions));
      memcpy(irdata1, irsrc, filterlen * sizeof(*irsrc));
      ApplyFades(irdata1, filterlen, fadein, fadeout);
      convolver.prepare_filter(irdata1, irdata1 + filterlen, filters[i]);

#if MEASURE_MAX_FILTER_LEVEL
      float filterlevel = CalculateLevel(irdata1, filterlen);
      DEBUG4(("Level of filter %u is %0.3lfdB", i, 20.0 * log10(filterlevel)));
      maxlevel = MAX(maxlevel, filterlevel);
#endif
    }

    DEBUG2(("Finished creating filters (took %lums)", GetTickCount() - tick));
#if DEBUG_LEVEL < 2
    UNUSED_PARAMETER(tick);
#endif

#if MEASURE_MAX_FILTER_LEVEL
    SetAudioScale(maxlevel);
#endif

    // force update of parameters
    updateparameters = true;
  }
}

/*--------------------------------------------------------------------------------*/
/** Load IRs from a file (either WAV or SOFA if enabled).
 *
 * @param filename file containing IRs
 * @param fade fade information to allow a subset of the data to be used
 *
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::LoadIRs(const char *filename, const FILTER_FADE& fade)
{
  if (filename)
  {
#if ENABLE_SOFA
    // only attempt to load file as SOFA if suffix is .sofa (otherwise app bombs out on non-SOFA files)
    if ((strlen(filename) >= 5) && (strcasecmp(filename + strlen(filename) - 5, ".sofa") == 0) && LoadSOFA(filename, fade))
    {
      DEBUG3(("Loaded IRs from SOFA file (%s).", filename));
    }
    else
#endif
      if (LoadIRsSndFile(filename, fade))
      {
        DEBUG3(("Loaded IRs from WAV file (%s).", filename));
      } else
      {
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
 * @param fade fade information to allow a subset of the data to be used
 *
 * @note Receiver measurements are interleaved.
 */
/*--------------------------------------------------------------------------------*/
bool ConvolverManager::LoadSOFA(const char *filename, const FILTER_FADE& fade)
{
  filters.clear();

  if (filename)
  {
    SOFA file(filename);

    if (file)
    {
      DEBUG3(("Opened '%s' okay, %u measurements from %u sources at %luHz", filename, (uint_t)file.get_num_measurements(), (uint_t)file.get_num_emitters(), (uint_t)file.get_samplerate()));
      LoadIRsSOFA(file, fade);
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
 * @param fade fade information to allow a subset of the data to be used
 *
 */
/*--------------------------------------------------------------------------------*/
bool ConvolverManager::LoadIRsSndFile(const char *filename, const FILTER_FADE& fade)
{
  filters.clear();

  if (filename)
  {
    SndfileHandle file(filename);

    if (file && file.frames() && file.channels())
    {
      uint_t filelen = file.frames();
      uint_t i, n = file.channels();
      uint_t filterstart, filterlen;
      std::vector<float> fadein, fadeout;

      DEBUG3(("Opened '%s' okay, %u channels at %uHz", filename, n, (uint_t)file.samplerate()));

      partitions = CalcPartitions(fade, file.samplerate(), filelen, blocksize, filterstart, filterlen);

      CreateFades(fade, file.samplerate(), fadein, fadeout);

      DEBUG2(("File '%s' is %u samples long, therefore %u partitions are needed", filename, filelen, partitions));

      apf::conv::Convolver convolver(blocksize, partitions);
      float *sampledata = new float[filelen * n];
      float *response   = new float[filterlen];
      slong_t res;

      memset(sampledata, 0, filelen * n * sizeof(*sampledata));

      DEBUG2(("Reading sample data..."));

      if ((res = file.read(sampledata, filelen * n)) < 0)
      {
        ERROR("Read of %u frames result: %ld", filelen, res);
      }

      DEBUG2(("Creating %u filters...", n));
      ulong_t tick     = GetTickCount();
#if MEASURE_MAX_FILTER_LEVEL
      float   maxlevel = 0.f;
#endif
      for (i = 0; i < n; i++)
      {
        DEBUG5(("Creating filter for IR %u", i));

        filters.push_back(APFFilter(blocksize, partitions));

        TransferSamples(sampledata + filterstart * n, i, n, response, 0, 1, 1, filterlen);
        ApplyFades(response, filterlen, fadein, fadeout);
        convolver.prepare_filter(response, response + filterlen, filters[i]);

#if MEASURE_MAX_FILTER_LEVEL
        float filterlevel = CalculateLevel(response, filterlen);
        DEBUG4(("Level of filter %u is %0.3lfdB", i, 20.0 * log10(filterlevel)));
        maxlevel = MAX(maxlevel, filterlevel);
#endif
      }
      DEBUG2(("Finished creating filters (took %lums)", GetTickCount() - tick));
#if DEBUG_LEVEL < 2
      UNUSED_PARAMETER(tick);
#endif

      delete[] sampledata;
      delete[] response;

#if MEASURE_MAX_FILTER_LEVEL
      SetAudioScale(maxlevel);
#endif

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
  maxdelay = 0.0;

  if (filename)
  {
    FILE *fp;

    DEBUG2(("Reading IR delays from '%s'", filename));

    if ((fp = fopen(filename, "r")) != NULL)
    {
      double mindelay = 0.0;  // used to reduce delays to minimum
      double delay = 0.0;
      uint_t i;
      bool   initialreading = true;

      while (fscanf(fp, "%lf", &delay) > 0)
      {
        irdelays.push_back(delay);

        // update minimum delay if necessary
        if (initialreading || (delay < mindelay)) mindelay = delay;

        initialreading = false;
      }

      // reduce each delay by mindelay to reduce overall delay
      // TODO: this isn't always acceptable behaviour
      for (i = 0; i < irdelays.size(); i++)
      {
        irdelays[i] -= mindelay;
        
        maxdelay = MAX(maxdelay, irdelays[i]);
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
  maxdelay = 0.0;

  if (num_delays)
  {
    uint_t i;

    irdelays.resize(num_delays);

    // reduce each delay by mindelay to reduce overall delay
    for (i = 0; i < irdelays.size(); i++)
    {
      irdelays[i] = delays[i];
      maxdelay = MAX(maxdelay, irdelays[i]);
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Prepare data for static convolvers
 *
 * @param convolverdata data structure to be populated with fade information
 * @param irlength length of IR data
 * @param samplerate sample rate for convert fade times to samples
 * @param fade fade profile
 *
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::PrepareStaticConvolvers(STATIC_CONVOLVER_DATA& convolverdata, uint_t irlength, double samplerate, const FILTER_FADE& fade)
{
  if (convolvers.size())
  {
    DEBUG1(("Warning: removing existing static convolvers"));

    while (convolvers.size())
    {
      Convolver *conv = convolvers.back();
      delete conv;
      convolvers.pop_back();
    }
  }

  parameters.clear();

  // populate convolverdata
  convolverdata.samplerate = samplerate;

  // calculate number of partitions and filter start/length
  partitions = CalcPartitions(fade, samplerate, irlength, blocksize, convolverdata.filterstart, convolverdata.filterlen);
  
  // create fades
  CreateFades(fade, samplerate, convolverdata.fadein, convolverdata.fadeout);
}

/*--------------------------------------------------------------------------------*/
/** Create a static convolver with the correct parameters for inclusion in this manager
 *
 * @param irdata pointer to impulse response data buffer
 * @param delay a delay associated with the static convolver
 * @param convolverdata previously populated (by the above) data structure
 *
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::CreateStaticConvolver(const float *irdata, double delay, const STATIC_CONVOLVER_DATA& convolverdata)
{
  std::vector<float> data;
  Convolver *conv;

  // take copy of IR data in order to apply fades
  data.resize(convolverdata.filterlen);

  float *fadedirdata = &data[0];

  // copy data
  memcpy(fadedirdata, irdata + convolverdata.filterstart, data.size() * sizeof(*irdata));

  // apply fades
  ApplyFades(fadedirdata, data.size(), convolverdata.fadein, convolverdata.fadeout);

  delay *= convolverdata.samplerate;

  // set up new convolver
  if ((conv = new StaticConvolver(convolvers.size(), blocksize, partitions, new apf::conv::StaticConvolver(blocksize, fadedirdata, fadedirdata + data.size()), delay)) != NULL)
  {
    PARAMETERS params;

    memset(&params, 0, sizeof(params));
    params.delay = delay;
    params.level = 1.0;
    parameters.push_back(params);

    conv->SetParameters(params.level, params.delay, hqproc);
    convolvers.push_back(conv);
  }
}

/*--------------------------------------------------------------------------------*/
/** Set the number of convolvers to run - creates and destroys convolvers as necessary
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::SetConvolverCount(uint_t nconvolvers)
{
  DEBUG3(("ConvolverManager<%016lx>: setting up for %u convolvers (from %u convolvers)", (ulong_t)this, nconvolvers, (uint_t)convolvers.size()));
 
  // update parameters array size
  parameters.resize(nconvolvers);

  // create convolvers if necessary
  while (convolvers.size() < nconvolvers)
  {
    Convolver *conv;

    // set up new convolver
    if ((conv = new DynamicConvolver(convolvers.size(), blocksize, partitions, new apf::conv::Convolver(blocksize, partitions))) != NULL)
    {
      convolvers.push_back(conv);

      // set default IR
      SelectIR(convolvers.size() - 1, 0);
    }
    else
    {
      ERROR("Failed to create new convolver!");
      break;
    }
  }

  // delete excessive convolvers
  while (convolvers.size() > nconvolvers)
  {
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

      DEBUG3(("Convolver[%03u]: Selecting IR %03u and delay %10.3lf samples", convolver, ir, delay));

      // pass parameters to convolver, add additional delay to scaled delay due to IR's
      DynamicConvolver *dynconv;
      if ((dynconv = dynamic_cast<DynamicConvolver *>(convolvers[convolver])) != NULL)
      {
        // Dynamic Convolver -> set IR filter
        dynconv->SetFilter(filters[ir]);
      }

      // set other parameters
      convolvers[convolver]->SetParameters(params.level, delay + params.delay, hqproc);
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
  PERFMON("Convolve");
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

#if DEBUG_CONVOLVER_STATES
  // report state of convolvers every two seconds
  if ((GetTickCount() - reporttick) >= 2000)
  {
    std::vector<char> states;

    // create string of indicators (add one for terminator)
    states.resize(convolvers.size() + 1);
    // set terminator
    states[convolvers.size()] = 0;

    for (i = 0; i < convolvers.size(); i++)
    {
      states[i] = convolvers[i]->IsProcessing() ? '*' : '.';
    }

    DEBUG1(("Convolvers: %s", &states[0]));

    reporttick = GetTickCount();
  }
#endif

  // now process outputs
  for (i = 0; i < convolvers.size(); i++)
  {
    DEBUG5(("Waiting on convolver %u/%u to complete...", i + 1, (uint_t)convolvers.size()));
    convolvers[i]->EndConvolution(output + (i % outputchannels), outputchannels, audioscale);
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
/** Get offset into raw data supplied by SOFA file 
 */
/*--------------------------------------------------------------------------------*/
uint_t ConvolverManager::GetSOFAOffset(const SOFA& file, uint_t emitter, uint_t measurement, uint_t receiver) const
{
  uint_t nr = file.get_num_receivers();
  uint_t ne = file.get_num_emitters();
  
  // This is VERY MUCH reliant on the format of the SOFA file!
  // ASSUMES data is stored in a 3-dimensional array [measurement][receiver][emitter]
  return measurement * nr * ne + receiver * ne + emitter;
}

/*--------------------------------------------------------------------------------*/
/** Load impulse reponse data from a SOFA file.
 *
 * @param file SOFA file object, opened for reading
 * @param fade fade information to allow a subset of the data to be used
 *
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::LoadIRsSOFA(SOFA& file, const FILTER_FADE& fade)
{
  // load impulse responses
  uint_t irlength = file.get_ir_length();
  uint_t ne = file.get_num_emitters(), ie;
  uint_t nm = file.get_num_measurements(), im;
  uint_t nr = file.get_num_receivers(), ir;
  std::vector<float> fadein, fadeout;
  double samplerate = file.get_samplerate();
  uint_t filterstart, filterlen;

  partitions = CalcPartitions(fade, samplerate, irlength, blocksize, filterstart, filterlen);
  CreateFades(fade, samplerate, fadein, fadeout);

  DEBUG2(("File is %u samples long, therefore %u partitions are needed", filterlen, partitions));

  apf::conv::Convolver convolver(blocksize, partitions);
  SOFA::audio_buffer_t irdata;
  ulong_t tick     = GetTickCount();
#if MEASURE_MAX_FILTER_LEVEL
  float   maxlevel = 0.f;
#endif

  DEBUG2(("Creating %u filters...", ne * nm * nr));

  file.get_all_irs(irdata);

  // loops MUST be done in this order to maintain the correct layout
  for (im = 0; im < nm; im++)         // measurements
  {
    for (ir = 0; ir < nr; ir++)       // receivers
    {
      for (ie = 0; ie < ne; ie++)     // emitters
      {
        SOFA::audio_sample_t *irdata1 = &irdata[0] + GetSOFAOffset(file, ie, im, ir) * irlength + filterstart;
        uint_t fn = filters.size(); // index of filter about to be created

        filters.push_back(APFFilter(blocksize, partitions));
        ApplyFades(irdata1, filterlen, fadein, fadeout);
        convolver.prepare_filter(irdata1, irdata1 + filterlen, filters[fn]);

#if MEASURE_MAX_FILTER_LEVEL
        float filterlevel = CalculateLevel(irdata1, filterlen);
        DEBUG4(("Level of filter %u/%u/%u is %0.3lfdB", ie, im, ir, 20.0 * log10(filterlevel)));
        maxlevel = MAX(maxlevel, filterlevel);
#endif
      }
    }
  }
  DEBUG2(("Finished creating filters (took %lums)", GetTickCount() - tick));
#if DEBUG_LEVEL < 2
  UNUSED_PARAMETER(tick);
#endif

#if MEASURE_MAX_FILTER_LEVEL
  SetAudioScale(maxlevel);
#endif

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
  maxdelay = 0.0;

  // get number of measurements and receivers
  float   sr = file.get_samplerate();
  uint_t  ne = file.get_num_emitters(), ie;
  uint_t  nm = file.get_num_measurements(), im, ndm = file.get_num_delay_measurements();        // number of delay measurements MAY be different fron number of measurements
  uint_t  nr = file.get_num_receivers(), ir;
  SOFA::delay_buffer_t sofadelays;

  // read delays for each receiver and insert into irdelays interleaved
  DEBUG2(("Loading %u delays from SOFA file", ne * nm * nr));

  file.get_all_delays(sofadelays);

  // loops MUST be done in this order to maintain the correct layout
  for (im = 0; im < nm; im++)         // measurements
  {
    for (ir = 0; ir < nr; ir++)       // receivers
    {
      for (ie = 0; ie < ne; ie++)     // emitters
      {
        double delay = sofadelays[GetSOFAOffset(file, ie, im % ndm, ir)] * sr;

        DEBUG3(("Delay for %u:%u:%u is %0.1lf samples", im, ir, ie, delay));

        irdelays.push_back(delay);
        maxdelay = MAX(maxdelay, delay);
      }
    }
  }

  // force update of parameters
  updateparameters = true;
}
#endif

/*--------------------------------------------------------------------------------*/
/** Return approximate number of seconds worth of audio this renderer 'holds'
 */
/*--------------------------------------------------------------------------------*/
uint_t ConvolverManager::SamplesBuffered() const
{
  return blocksize * partitions + Convolver::GetMaxAdditionalDelay();
}

/*--------------------------------------------------------------------------------*/
/** Calculate reasonnable level value for filter
 */
/*--------------------------------------------------------------------------------*/
float ConvolverManager::CalculateLevel(const float *data, uint_t n)
{
  const uint_t sumlen = 480;
  float  sum = 0.f;
  float  max = 0.f;
  uint_t i;
  
  for (i = 0; i < n; i++)
  {
    sum += data[i] * data[i];
    if (i >= sumlen) sum -= data[i - sumlen] * data[i - sumlen];
    max = MAX(max, sum);
  }

  return sqrtf(max / (float)MIN(sumlen, n));
}

/*--------------------------------------------------------------------------------*/
/** Set audio scaling value based on maxlevel
 *
 * (Not currently used)
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::SetAudioScale(float maxlevel)
{
  UNUSED_PARAMETER(maxlevel);

  if (maxlevel > 0.f)
  {
    audioscale = 1.f / maxlevel;
    DEBUG1(("Max level = %0.1lfdB, scale = %0.1lfdB", 20.0 * log10(maxlevel), 20.0 * log10(audioscale)));
  }
}

/*----------------------------------------------------------------------------------------------------*/

uint_t Convolver::maxadditionaldelay = 2400;

/*--------------------------------------------------------------------------------*/
/** Protected constructor so that only ConvolverManager can create convolvers
 */
/*--------------------------------------------------------------------------------*/
Convolver::Convolver(uint_t _convindex, uint_t _blocksize, uint_t _partitions, double _delay) :
  thread(0),
  blocksize(_blocksize),
  partitions(_partitions),
  zeroblocks(0),
  maxzeroblocks(0),
  convindex(_convindex),
  input(new float[blocksize]),
  output(new float[blocksize]),
  outputdelay(_delay),
  outputlevel(1.0),
  quitthread(false)
{
  // calculate number of blocks of silence after which there's no need to do any processing
  maxzeroblocks = partitions + (maxadditionaldelay / blocksize) + 1;
  if (convindex == 0) DEBUG2(("Max zero blocks = %u", maxzeroblocks));

  // create thread
  if (pthread_create(&thread, NULL, &__Process, (void *)this) != 0)
  {
    ERROR("Failed to create thread (%s)", strerror(errno));
    thread = 0;
  }
}

Convolver::~Convolver()
{
  StopThread();

  if (input)  delete[] input;
  if (output) delete[] output;
}

std::string Convolver::DebugHeader() const
{
  static const std::string column = "                    ";
  static ulong_t tick0 = GetTickCount();
  std::string res = "";
  uint_t i;

  Printf(res, "%06lu (%02u): ", GetTickCount() - tick0, convindex);

  for (i = 0; i < convindex; i++) res += column;

  return res;
}

/*--------------------------------------------------------------------------------*/
/** Stop processing thread
 */
/*--------------------------------------------------------------------------------*/
void Convolver::StopThread()
{
  if (thread)
  {
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
  bool   nonzero = false;

  // copy input (de-interleaving)
  for (i = 0; i < blocksize; i++)
  {
    input[i] = _input[i * inputchannels];
    nonzero |= (input[i] != 0.0);       // detect silence
  }

  // count up silent blocks
  if      (nonzero)                    zeroblocks = 0;
  else if (zeroblocks < maxzeroblocks) zeroblocks++;

  // only start thread if there is non-zero blocks somewhere in the chain
  if (zeroblocks < maxzeroblocks)
  {
    DEBUG4(("%smain signal", DebugHeader().c_str()));

    // release thread
    startsignal.Signal();
  }
}

/*--------------------------------------------------------------------------------*/
/** Wait for end of convolution and mix output
 *
 * @param _output buffer to mix locally generated output to (assumed to by blocksize * outputchannels long)
 * @param outputchannels number of channels in _output
 * @param level mix level
 *
 */
/*--------------------------------------------------------------------------------*/
void Convolver::EndConvolution(float *_output, uint_t outputchannels, float level)
{
  // only wait if processing has been triggered
  if (zeroblocks < maxzeroblocks)
  {
    DEBUG4(("%smain wait", DebugHeader().c_str()));

    // wait for completion
    donesignal.Wait();

    DEBUG4(("%smain done", DebugHeader().c_str()));

    // mix locally generated output into supplied output buffer
    uint_t i;
    for (i = 0; i < blocksize; i++)
    {
      _output[i * outputchannels] += output[i] * level;
    }
  }
}

/*--------------------------------------------------------------------------------*/
/** Processing thread
 */
/*--------------------------------------------------------------------------------*/
void *Convolver::Process()
{
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

    // decide whether there's any need to perform convolution
    if (zeroblocks < partitions)
    {
      // call DynamicConvolver or StaticConvolver
      Convolve(delay + delaypos);
    }
    else
    {
      // no audio in -> simply zero input
      memset(delay + delaypos, 0, blocksize * sizeof(*delay));
    }

    // process delay memory using specified delay
    uint_t pos1   = delaypos + delaylen;
    double level2 = outputlevel;
    double delay2 = MIN(outputdelay, (double)maxdelay);

    // decide whether there's any need to perform mixing
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
 * @param level audio output level
 * @param delay audio delay (due to ITD and source delay) (in SAMPLES)
 * @param hqproc true for high-quality and CPU hungry processing
 */
/*--------------------------------------------------------------------------------*/
void Convolver::SetParameters(double level, double delay, bool hqproc)
{
  // update processing parameters
  outputlevel  = level;
  outputdelay  = delay;
  this->hqproc = hqproc;
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Protected constructor so that only ConvolverManager can create convolvers
 */
/*--------------------------------------------------------------------------------*/
DynamicConvolver::DynamicConvolver(uint_t _convindex, uint_t _blocksize, uint_t _partitions, APFConvolver *_convolver) : Convolver(_convindex, _blocksize, _partitions),
                                                                                                                         convolver(_convolver),
                                                                                                                         convfilter(NULL),
                                                                                                                         filter(NULL)
{
}

DynamicConvolver::~DynamicConvolver()
{
  if (convolver) delete convolver;
}

/*--------------------------------------------------------------------------------*/
/** Set IR filter for convolution
 *
 * @param newfilter new IR filter from ConvolverManager
 */
/*--------------------------------------------------------------------------------*/
void DynamicConvolver::SetFilter(const APFFilter& newfilter)
{
  if (&newfilter != filter)
  {
    DEBUG3(("[%010lu]: Selecting new filter for convolver %3u", GetTickCount(), convindex));
    // set convolver filter
    filter = &newfilter;
  }
}
  
/*--------------------------------------------------------------------------------*/
/** Actually perform convolution on the input and store it in the provided buffer
 *
 * @param dest destination buffer
 *
 */
/*--------------------------------------------------------------------------------*/
void DynamicConvolver::Convolve(float *dest)
{
  bool crossfade = false;

  // add input to convolver
  convolver->add_block(input);

  // do convolution
  const float *result = convolver->convolve(1.f);
            
  // copy data into delay memory
  memcpy(dest, result, blocksize * sizeof(*dest));

  // if filter needs updating, update it now
  if (filter && (filter != convfilter))
  {
    convfilter = (const APFFilter *)filter;
    convolver->set_filter(*convfilter);
    crossfade = true;   // force crossfade after filter update
    DEBUG3(("[%010lu]: Selected new filter for convolver %3u", GetTickCount(), convindex));
  }

  // if queues_empty() returns false, must cross fade between convolutions
  if (crossfade || !convolver->queues_empty())
  {
    uint_t i;

    DEBUG3(("Crossfading convolver %u", convindex));

    // rotate queues within convolver
    convolver->rotate_queues();

    // do convolution a second time
    result = convolver->convolve(1.f);

    // crossfade new convolution result into buffer
    for (i = 0; i < blocksize; i++)
    {
      double b = (double)i / (double)blocksize, a = 1.0 - b;

      dest[i] = dest[i] * a + b * result[i];
    }
  }
}

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Protected constructor so that only ConvolverManager can create convolvers
 */
/*--------------------------------------------------------------------------------*/
StaticConvolver::StaticConvolver(uint_t _convindex, uint_t _blocksize, uint_t _partitions, APFConvolver *_convolver, double _delay) : Convolver(_convindex, _blocksize, _partitions, _delay),
                                                                                                                                      convolver(_convolver)
{
}

StaticConvolver::~StaticConvolver()
{
  if (convolver) delete convolver;
}

/*--------------------------------------------------------------------------------*/
/** Actually perform convolution on the input and store it in the provided buffer
 *
 * @param dest destination buffer
 *
 */
/*--------------------------------------------------------------------------------*/
void StaticConvolver::Convolve(float *dest)
{
  // add input to convolver
  convolver->add_block(input);

  // do convolution
  const float *result = convolver->convolve(1.f);
            
  // copy data into delay memory
  memcpy(dest, result, blocksize * sizeof(*dest));
}

BBC_AUDIOTOOLBOX_END
