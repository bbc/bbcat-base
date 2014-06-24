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

using namespace std;

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Constructor for convolver manager
 *
 * @param irfile WAV file containing IRs
 * @param irdelayfile text file containing the required delays (in SAMPLES) of each IR in irfile
 * @param partitionsize the convolution partition size - essentially the block size of the processing
 *
 */
/*--------------------------------------------------------------------------------*/
ConvolverManager::ConvolverManager(const char *irfile, const char *irdelayfile, uint_t partitionsize) : blocksize(partitionsize),
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
/** Create a convolver with the correct parameters for inclusion in this manager
 */
/*--------------------------------------------------------------------------------*/
ConvolverManager::APFConvolver *ConvolverManager::CreateConvolver() const
{
	return new APFConvolver(blocksize, partitions);
}

/*--------------------------------------------------------------------------------*/
/** Load IR files from WAV file
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::LoadIRs(const char *filename)
{
	filters.clear();

	if (filename) {
		SndfileHandle file(filename);

		if (file) {
			APFConvolver *convolver;
			ulong_t len = file.frames();
			uint_t i, n = file.channels();

			DEBUG3(("Opened '%s' okay, %u channels at %luHz", filename, n, (ulong_t)file.samplerate()));

			partitions = (uint_t)((len + blocksize - 1) / blocksize);

			DEBUG2(("File '%s' is %lu samples long, therefore %u partitions are needed", filename, len, partitions));

			audioscale = .125f;

			if ((convolver = CreateConvolver()) != NULL) {
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

					convolver->prepare_filter(response, response + blocksize * partitions, filters[i]);
				}
				DEBUG2(("Finished creating filters (took %lums)", (ulong_t)(GetTickCount() - tick)));

				delete[] sampledata;
				delete[] response;
				delete   convolver;

				// force update of parameters
				updateparameters = true;
			}
		}
		else ERROR("Failed to open IR file ('%s') for reading", filename);
	}
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
			double mindelay = 0.0;	// used to reduce delays to minimum
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
/** Set the number of convolvers to run - creates and destroys convolvers as necessary
 */
/*--------------------------------------------------------------------------------*/
void ConvolverManager::SetConvolverCount(uint_t nconvolvers)
{
	// update irindexes vector size
	irindexes.resize(nconvolvers);

	// update delays vector size
	delays.resize(nconvolvers);

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
 * @param delay additional delay to be applied to the convolver
 *
 * @return true if IR selected
 */
/*--------------------------------------------------------------------------------*/
bool ConvolverManager::SelectIR(uint_t convolver, uint_t ir, double delay)
{
	bool success = false;

	if (convolver < convolvers.size())
	{
		if (ir < filters.size())
		{
			DEBUG3(("[%010lu]: Selecting IR %03u for convolver %3u", (ulong_t)GetTickCount(), ir, convolver));

			// store IR number in index
			irindexes[convolver] = ir;

			// store IR number in index
			delays[convolver] = delay;

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
		uint_t ir = irindexes[convolver];

		if (ir < filters.size())
		{
			// if a delay is available for this IR, subtract minimum delay and scale it by delayscale
			// to compensate for ITD
			double delay = (ir < irdelays.size()) ? irdelays[ir] * delayscale : 0.0;

			// pass parameters to convolver, add additional delay to scaled delay due to IR's
			convolvers[convolver]->SetParameters(filters[ir], delay + delays[convolver], hqproc);
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

/*----------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
/** Protected constructor so that only ConvolverManager can create convolvers
 */
/*--------------------------------------------------------------------------------*/
Convolver::Convolver(uint_t _convindex, uint_t _blocksize, APFConvolver *_convolver, float _scale) :
	thread(0),
	convolver(_convolver),
	blocksize(_blocksize),
	convindex(_convindex),
	scale(_scale),
	filter(NULL),
	input(new float[blocksize]),
	output(new float[blocksize]),
	outputdelay(0.0),
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

	if (convolver) delete convolver;
	if (input)     delete[] input;
	if (output)    delete[] output;
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
	uint_t delaypos = 0, delaylen = blocksize * 2;
	float  *delay = new float[delaylen];		// delay memory
	double delay1 = 0.0;

	// clear delay memory
	memset(delay, 0, delaylen * sizeof(*delay));

	while (!quitthread) {
		uint_t i;

		DEBUG4(("%sproc wait", DebugHeader().c_str()));

		// wait to be released
		startsignal.Wait();

		DEBUG4(("%sproc start", DebugHeader().c_str()));
	
		// detect quit request
		if (quitthread) {
			break;
		}

		// add input to convolver
		convolver->add_block(input);

		// do convolution
		const float *result = convolver->convolve(scale);
		float       *dest   = delay + delaypos;
			
		// copy data into delay memory
		memcpy(dest, result, blocksize * sizeof(*delay));

		// if filter needs updating, update it now
		if (filter && (filter != convfilter)) {
			convfilter = (const APFFilter *)filter;
			convolver->set_filter(*convfilter);
			DEBUG3(("[%010lu]: Selected new filter for convolver %3u", (ulong_t)GetTickCount(), convindex));
		}

		// if queues_empty() returns false, must cross fade between convolutions
		if (!convolver->queues_empty()) {
			// rotate queues within convolver
			convolver->rotate_queues();

			// do convolution a second time
			result = convolver->convolve(scale);

			// crossfade new convolution result into buffer
			for (i = 0; i < blocksize; i++) {
				double b = (double)i / (double)blocksize, a = 1.0 - b;

				dest[i] = dest[i] * a + b * result[i];
			}
		}

		// process delay memory using specified delay
		uint_t pos1   = delaypos + delaylen;
		double delay2 = outputdelay;
		double fpos1  = (double)pos1               - delay1;
		double fpos2  = (double)(pos1 + blocksize) - delay2;

		if (hqproc) {
			// high quality processing - use SRC filter to generate samples with fractional delays
			for (i = 0; i < blocksize; i++) {
				double b    = (double)i / (double)blocksize, a = 1.0 - b;
				double fpos = a * fpos1 + b * fpos2;

				output[i] = FractionalSample(delay, 0, 1, delaylen, fpos);
			}
		}
		else {
			// low quality processing - just use integer delays without SRC filter
			for (i = 0; i < blocksize; i++) {
				double b    = (double)i / (double)blocksize, a = 1.0 - b;
				double fpos = a * fpos1 + b * fpos2;

				output[i] = delay[(uint_t)fpos % delaylen];
			}
		}

		// advance delay position by a block
		delaypos = (delaypos + blocksize) % delaylen;
		delay1   = delay2;

		DEBUG4(("%sproc done", DebugHeader().c_str()));

		// signal done
		donesignal.Signal();
	}

	pthread_exit(NULL);

	return NULL;
}

/*--------------------------------------------------------------------------------*/
/** Set filter, delay and high-quality processing options for convolution
 */
/*--------------------------------------------------------------------------------*/
void Convolver::SetParameters(const APFFilter& newfilter, double delay, bool hqproc)
{
	if (&newfilter != filter) {
		DEBUG3(("[%010lu]: Selecting new filter for convolver %3u", (ulong_t)GetTickCount(), convindex));

		// set convolver filter
		filter = &newfilter;
	}

	// update processing parameters
	this->hqproc = hqproc;
	outputdelay  = delay;
}

BBC_AUDIOTOOLBOX_END
