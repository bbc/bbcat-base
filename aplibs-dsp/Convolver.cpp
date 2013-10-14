/******************************************************************************
 * Copyright (c) 2006-2012 Quality & Usability Lab                            *
 *                         Deutsche Telekom Laboratories, TU Berlin           *
 *                         Ernst-Reuter-Platz 7, 10587 Berlin, Germany        *
 *                                                                            *
 * This file is part of the SoundScape Renderer (SSR).                        *
 *                                                                            *
 * The SSR is free software:  you can redistribute it and/or modify it  under *
 * the terms of the  GNU  General  Public  License  as published by the  Free *
 * Software Foundation, either version 3 of the License,  or (at your option) *
 * any later version.                                                         *
 *                                                                            *
 * The SSR is distributed in the hope that it will be useful, but WITHOUT ANY *
 * WARRANTY;  without even the implied warranty of MERCHANTABILITY or FITNESS *
 * FOR A PARTICULAR PURPOSE.                                                  *
 * See the GNU General Public License for more details.                       *
 *                                                                            *
 * You should  have received a copy  of the GNU General Public License  along *
 * with this program.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * The SSR is a tool  for  real-time  spatial audio reproduction  providing a *
 * variety of rendering algorithms.                                           *
 *                                                                            *
 * http://tu-berlin.de/?id=ssr                  SoundScapeRenderer@telekom.de *
 ******************************************************************************/

/** @file
 * Convolution engine (implementation).
 *
 * $LastChangedDate: 2012-02-01 11:43:25 +0100 (Wed, 01 Feb 2012) $
 * $LastChangedRevision: 1659 $
 * $LastChangedBy: geier.matthias $
 **/

#include <cmath>
#include <fstream>
#include <sys/types.h>
#include <cstring>
#include <iostream>
#include "Convolver.hpp"
#include "Logging.hpp"

#ifdef __SSE__
#include <xmmintrin.h>  // for SSE instrinsics
#endif

namespace // anonymous
{
const float pi_float = 3.14159265f;

// typedef needed by SIMD instructions
typedef float v4sf __attribute__ ((vector_size(16)));

union f4vector
{
	v4sf *v;
	float *f;
};

union f4vector2
{
	v4sf v;
	float f[4];
};
}

//#include "complex_mul.h"

//extern "C" {
//#include "asmprot.h"
//}

/** Initialize the convolver. 
 * It also sets the filter to be a Dirac. Thus, if no filter is specified
 * the audio data are not affected. However, quite some computational 
 * load for nothing...
 *
 * You should use create() instead of directly using this constructor.
 * @throw std::bad_alloc if not enough memory could be allocated
 * @throw std::runtime_error if sizeof(float) != 4
 **/
Convolver::Convolver(const nframes_t nframes, const crossfade_t crossfade_type)
throw (std::bad_alloc, std::runtime_error)
: _frame_size(nframes)
, _partition_size(nframes+nframes)
, _crossfade_type(crossfade_type)
, _no_of_partitions_to_process(0)
, _old_weighting_factor(0)
{
	// make sure that SIMD instructions can be used properly
	if (sizeof(float) != 4)
	{
		throw(std::runtime_error("sizeof(float) on your computer is not 4! "
				" The convolution can not take place properly."));
	}

	_signal.clear();
	_waiting_queue.clear();

	// allocate memory and initialize to 0
	_fft_buffer.resize  (_partition_size, 0.0f);
	_ifft_buffer.resize (_partition_size, 0.0f);

	// create first partition
	//_filter_coefficients.resize(_partition_size, 0.0f);

	_zeros.resize (_partition_size, 0.0f);

	_output_buffer.resize(_frame_size, 0.0f);

	// create fades if required
	if (_crossfade_type != none)
	{
		// init memory
		_fade_in.resize (_frame_size, 0.0f);
		_fade_out.resize(_frame_size, 0.0f);

		// this is the ifft normalization factor (fftw3 does not normalize)
		const float norm = 1.0f / _partition_size;

		// raised cosine fade
		if(_crossfade_type == raised_cosine)
		{
			// create fades
			for(unsigned int n = 0u; n < _frame_size; n++)
			{
				_fade_in[n]  = norm * (0.5f
						+ 0.5f * cos(static_cast<float>(_frame_size-n) / _frame_size
								* pi_float));
				_fade_out[n] = norm * (0.5f
						+ 0.5f * cos(static_cast<float>(n) / _frame_size * pi_float));
			} // for
		} // if

		// linear fade
		else if (_crossfade_type == linear)
		{
			// create fades
			for(unsigned int n = 0u; n < _frame_size; n++)
			{
				_fade_in[n]  = norm * (static_cast<float>(n)/_frame_size);
				_fade_out[n] = norm * (static_cast<float>(_frame_size-n)/_frame_size);
			} // for
		} // else if

	} // if

	/*
  // open file
  std::ifstream fft_wisdom("fftw3_plans/fft_plan.txt");

  if (!fft_wisdom.is_open())
  {
    APLIBS_DSP_WARNING("Cannot open file.");
  }
  else if (!fftw_import_wisdom_from_file(fft_wisdom))
  {
    APLIBS_DSP_WARNING("Cannot import fft wisdom.");
  }

  fft_wisdom.close();  
	 */

	// create fft plans for halfcomplex data format
	_fft_plan  = fftwf_plan_r2r_1d(_partition_size, &_fft_buffer[0],
			&_fft_buffer[0], FFTW_R2HC, FFTW_PATIENT);
	_ifft_plan = fftwf_plan_r2r_1d(_partition_size, &_ifft_buffer[0],
			&_ifft_buffer[0], FFTW_HC2R, FFTW_PATIENT);

	// calculate transfer function of a dirac
	std::copy(_zeros.begin(), _zeros.end(), _fft_buffer.begin());

	_fft_buffer[0] = 1.0f;
	_fft();

	// store dirac
	_neutral_filter = _fft_buffer;

	// clear _fft_buffer
	std::copy(_zeros.begin(), _zeros.end(), _fft_buffer.begin());

	// set dirac as default filter
	set_neutral_filter();
}

Convolver::~Convolver()
{
	fftwf_destroy_plan(_fft_plan);
	fftwf_destroy_plan(_ifft_plan);
}

/** static factory function for Convolver objects
 * @param nframes length of audio frame
 * @param crossfade_type type of the employed crossfade. Most efficient is 
 * of course \b none.
 * @return std::auto_ptr to the new Convolver object.
 **/
Convolver::ptr_t
Convolver::create(const nframes_t nframes, const crossfade_t crossfade_type)
{
	ptr_t temp;
	try
	{
		temp.reset(new Convolver(nframes, crossfade_type));
	}
	catch (const std::bad_alloc &)
	{
		temp.reset(); // set to NULL
	}
	// we deliberately don't catch std::runtime_error!
	return temp;
}

/** Sets a filter that does not influence the audio data 
 * (i.e. a dirac in time domain).
 * Note that this is done at initialization stage.
 */
void 
Convolver::set_neutral_filter()
{
	set_filter_f(_neutral_filter);
}

/** Sets the filter. 
 * The length of the impulse response is arbitrary. It automatically
 * performs zero padding if necessary and creates the required number of 
 * partitions. It is not very efficient since it calls 
 * \b Convolver::prepare_impulse_response(). It is provided for convenience.
 * If you have the filter's transfer function in halfcomplex format use
 * \b Convolver::set_filter_f() instead. 
 * @param filter impulse response of the filter
 */
void 
Convolver::set_filter_t(const data_t& filter)
{

	if (filter.empty())
	{
		APLIBS_DSP_WARNING("You are trying to use an empty filter.");
		return;
	}

	data_t buffer;

	// TODO: It would be more efficient to use _fft_plan and _fft_buffer etc.
	prepare_impulse_response(buffer, &filter[0], filter.size(), _frame_size);

	set_filter_f(buffer);
}

/** Sets a new filter. However, the filter partitions are updated
 * cycle by cycle one after the other. Note: With the current implementation
 * the number of partitions of the new filter does not matter as long as it is 
 * equal or higher than the number of partitions of the current filter! If you 
 * set a filter which has less partitions than the current one, some of the 
 * partitions of the old filter will remain in the chain. So don't do that.
 * @param filter vector holding the transfer functions of the zero padded 
 * filter partitions in halfcomplex format (see also fftw3 documentation).
 * First element of \b filter is the first partition etc.
 */
void 
Convolver::set_filter_f(data_t& filter)
{
	if (filter.empty()) return;

	// if more filter updates than convolutions happen
	if (!_waiting_queue.empty() && _waiting_queue.back().second == 0u)
	{
		_waiting_queue.pop_back();
	}

	_waiting_queue.push_back(std::pair<data_t, unsigned int> (filter, 0u));

}

/** This function assures that the filter partitions are not 
 * exchanged all at the same time when a new filter is set.
 * It assures the correct order and timing of the update of
 * the individual filter partitions.
 */
void 
Convolver::_update_filter_partitions()
{
	// if nothing to update
	if (_waiting_queue.empty()) return;

	unsigned int no_of_partitions = _filter_coefficients.size()/_partition_size;


	// go through all filters that are waiting to check
	// for how long they have waited
	for (waiting_queue_t::iterator i = _waiting_queue.begin();
			i != _waiting_queue.end(); )
	{
		// exchange filter partition
		if (i->second < no_of_partitions)
		{
			std::copy(i->first.begin() + i->second * _partition_size,
					i->first.begin() + (i->second+1) * _partition_size,
					_filter_coefficients.begin() + i->second * _partition_size);
		}

		// append partition to the filter
		else if (i->second == no_of_partitions &&
				i->first.size() > no_of_partitions*_partition_size)
		{

			for (unsigned int n = 0u; n < _partition_size; n++)
			{
				_filter_coefficients.push_back(
						i->first[i->second * _partition_size + n]);
			}

			no_of_partitions++;
			APLIBS_DSP_INFO("Adding filter partition number " << no_of_partitions << ".");
		}

		// this should not happen
		else if (i->second > no_of_partitions)
		{
			APLIBS_DSP_ERROR("Something's wrong concerning the partition scheduling.");

			// get rid of the problem
			i = _waiting_queue.erase(i);
		}

		// if all partitions of the filter are already in use
		if (i->first.size() == (i->second+1)*_partition_size)
		{
			// erase filter from waiting queue
			i = _waiting_queue.erase(i);
		}
		else // if not
		{
			// increase the age of the filter by one cycle
			(i->second)++;
			// step one filter further
			i++;
		}
	}

	///////////////////////////////////////////
	///////////////////////////////////////////
	///////////////////////////////////////////
	//  TODO: Handle filters with different  //
	//  numbers of partitions                //
	///////////////////////////////////////////
	///////////////////////////////////////////
	///////////////////////////////////////////

}

/** Fast convolution of audio signal frame.
 * TODO: This function might provides some (slight) potential to optimize the 
 * performance regarding the buffer management. 
 * @param input_signal pointer to the first audio sample in the frame to be
 * convolved.
 * @param weighting_factor amplitude weighting factor for current signal frame
 * The filter has to be set via \b Convolver::set_filter_t or 
 * \b Convolver::set_filter_f. The filter is stored. If you want 
 * to keep the previous filter you don't need to update it.
 * @return pointer to the first sample of the convolved and weighted signal
 */
float*
Convolver::convolve_signal(float *input_signal, float weighting_factor)
{ 
	/////////////////////////////////////////////////
	////// check if processing has to be done ///////
	/////////////////////////////////////////////////
	if ( !weighting_factor || !_contains_data(input_signal, _frame_size) )
	{
		if ( !_no_of_partitions_to_process )
		{
			// no processing has to be done

			// make sure that no previous signal frames are reused
			_signal.clear();

			// make sure that output buffer is empty
			std::copy(_zeros.begin(),
					_zeros.begin() + _frame_size,
					_output_buffer.begin());

			// set current filter in order to assure smooth re-fade-in
			_update_filter_partitions();

			return &_output_buffer[0];
		}
		else
			// if there are still partitions to be convolved
		{
			_no_of_partitions_to_process--;
		}
	}
	// if there is data in input signal
	else _no_of_partitions_to_process = _waiting_queue.size();

	//////////////////////////////////////////////
	/////// processing has to be done ////////////
	//////////////////////////////////////////////

	// add current signal frame to _fft_buffer
	// _fft_buffer holds two signal frames
	std::copy(input_signal,
			input_signal + _frame_size,
			_fft_buffer.begin() + _frame_size);

	// signal fft
	_fft();

	// save signal partition in frequency domain
	_signal.push_back(_fft_buffer);

	// add signal to fft buffer (for the upcoming cycle)
	std::copy(input_signal, input_signal + _frame_size, _fft_buffer.begin());

	// if we crossfade, then convolve current audio frame
	// with previous filter
	if (_crossfade_type != none)
	{
		// erase most ancient audio signal frame
		if (_signal.size() > _filter_coefficients.size()/_partition_size)
			_signal.erase(_signal.begin());

		// multiplication of spectra
		_multiply_spectra();

		// signal ifft
		_ifft();

		// store data in output buffer
		std::copy(_ifft_buffer.begin() + _frame_size,
				_ifft_buffer.end(),
				_output_buffer.begin());
	}

	// set current filter
	_update_filter_partitions();

	// this loops when a long filter has been replaced by a short one
	while (_signal.size() > _filter_coefficients.size()/_partition_size)
	{
		_signal.erase(_signal.begin());
	}

	// multiplication of spectra
	_multiply_spectra();

	// signal ifft
	_ifft();

	// create proper output signal depending on crossfade
	if (_crossfade_type == none)
	{
		// normalize buffer (fftw3 does not do this)
		_normalize_buffer(&_ifft_buffer[_frame_size], weighting_factor);

		// return second half of _ifft_buffer
		return &_ifft_buffer[_frame_size];
	}
	else
	{
		// here, FFT normalization is included in the crossfades
		_crossfade_into_buffer(_output_buffer, weighting_factor);

		return &_output_buffer[0];
	}
}

/** Checks if a buffer contains data.
 * @param buffer buffer to be checked
 * @param buffer_size this allows for checking only part of a buffer.
 * @return \b true if data exists, \false if only zeros
 **/
bool 
Convolver::_contains_data(const float* buffer,
		const unsigned buffer_size) const
{
	for (unsigned int n = 0u; n < buffer_size; n++)
	{
		if (buffer[n] != 0.0) return true;
	}

	return false;
}

void 
Convolver::_multiply_partition_cpp(const float *signal, const float* filter)
{
	float d1s = _ifft_buffer[0] + signal[0] * filter[0];
	float d2s = _ifft_buffer[4] + signal[4] * filter[4];

	for (unsigned int nn = 0u; nn < _partition_size; nn += 8u)
	{

		// real parts
		_ifft_buffer[nn+0] += signal[nn+0] * filter[nn + 0] -
				signal[nn+4] * filter[nn + 4];
		_ifft_buffer[nn+1] += signal[nn+1] * filter[nn + 1] -
				signal[nn+5] * filter[nn + 5];
		_ifft_buffer[nn+2] += signal[nn+2] * filter[nn + 2] -
				signal[nn+6] * filter[nn + 6];
		_ifft_buffer[nn+3] += signal[nn+3] * filter[nn + 3] -
				signal[nn+7] * filter[nn + 7];

		// imaginary parts
		_ifft_buffer[nn+4] += signal[nn+0] * filter[nn + 4] +
				signal[nn+4] * filter[nn + 0];
		_ifft_buffer[nn+5] += signal[nn+1] * filter[nn + 5] +
				signal[nn+5] * filter[nn + 1];
		_ifft_buffer[nn+6] += signal[nn+2] * filter[nn + 6] +
				signal[nn+6] * filter[nn + 2];
		_ifft_buffer[nn+7] += signal[nn+3] * filter[nn + 7] +
				signal[nn+7] * filter[nn + 3];

	} // for

	_ifft_buffer[0] = d1s;
	_ifft_buffer[4] = d2s;

}

#ifdef __SSE__
void
Convolver::_multiply_partition_simd(const float *signal, const float* filter)
{
  // 16 byte alignment is needed for _mm_load_ps()!
  // This should be the case anyway because fftwf_malloc() is used.

  float dc = _ifft_buffer[0] + signal[0] * filter[0];
  float ny = _ifft_buffer[4] + signal[4] * filter[4];

  for(size_t i = 0; i < _partition_size; i += 8)
  {
    // load real and imaginary parts of signal and filter
    __m128 sigr = _mm_load_ps(signal + i);
    __m128 sigi = _mm_load_ps(signal + i + 4);
    __m128 filtr = _mm_load_ps(filter + i);
    __m128 filti = _mm_load_ps(filter + i + 4);

    // multiply and subtract
    __m128 res1 = _mm_sub_ps(_mm_mul_ps(sigr, filtr), _mm_mul_ps(sigi, filti));

    // multiply and add
    __m128 res2 = _mm_add_ps(_mm_mul_ps(sigr, filti), _mm_mul_ps(sigi, filtr));

    // load output data for accumulation
    __m128 acc1 = _mm_load_ps(&_ifft_buffer[i]);
    __m128 acc2 = _mm_load_ps(&_ifft_buffer[i + 4]);

    // accumulate
    acc1 = _mm_add_ps(acc1, res1);
    acc2 = _mm_add_ps(acc2, res2);

    // store output data
    _mm_store_ps(&_ifft_buffer[i], acc1);
    _mm_store_ps(&_ifft_buffer[i + 4], acc2);
  }

  _ifft_buffer[0] = dc;
  _ifft_buffer[4] = ny;
}
#endif

/** This function performs the actual fast convolution.*/
void 
Convolver::_multiply_spectra()
{
	// determine how many partitions have to be processed
	const unsigned int no_of_partitions =
			static_cast<unsigned int>(_signal.size());

	// initialize _ifft_buffer
	std::copy(_zeros.begin(), _zeros.end(), _ifft_buffer.begin());

	std::list<data_t>::reverse_iterator signal_partition = _signal.rbegin();

	// loop over partitions
	for( unsigned int partition = 0; partition < no_of_partitions; partition++ )
	{

#ifdef __SSE__
		_multiply_partition_simd( &(*signal_partition)[0],
				&(_filter_coefficients[partition * _partition_size]) );
#else
		_multiply_partition_cpp( &(*signal_partition)[0],
				&(_filter_coefficients[partition * _partition_size]) );
#endif

		signal_partition++;
	} // loop over partitions

}

/** fftw3 does not do this. 
 * It is expected that the size of the buffer is \b _frame_size 
 * (i.e. 0.5 * \b _parition_size).
 **/
void 
Convolver::_normalize_buffer(data_t& buffer, float weighting_factor)
{
	float* output_sample = &buffer[0];

	// normalize with respect to the fft size (2*_frame_size)
	const float norm = 1.0f / _partition_size;

	for(unsigned int n = 0; n < _frame_size; n++)
	{
		// we multiply instead of dividing to save computation power
		*output_sample *= norm;
		*output_sample *= weighting_factor;
		output_sample++;
	}
}

/** fftw3 does not do this. 
 * Overloaded function to normalize a part of a buffer
 **/
void 
Convolver::_normalize_buffer(float* sample, float weighting_factor)
{
	// normalize with respect to the fft size (2*_frame_size)
	const float norm = 1.0f / _partition_size;

	for(unsigned int n = 0; n < _frame_size; n++)
	{
		// we multiply instead of dividing to save computation power
		*sample *= norm;
		*sample *= weighting_factor;
		sample++;
	}
}

/** This function performs the actual crossfade if a crossfade
 * is applied.
 */
void 
Convolver::_crossfade_into_buffer(data_t& buffer, float weighting_factor)
{
	float *output_sample = &buffer[0];

	float *ifft_sample = &_ifft_buffer[_frame_size];

	float *fade_in_sample  = &_fade_in[0];
	float *fade_out_sample = &_fade_out[0];

	// crossfade between current ifft output and whatever
	// resides in the current output buffer
	for(unsigned int n = 0; n < _frame_size; n++)
	{
		// C-style to be efficient
		*output_sample = *output_sample * *fade_out_sample * _old_weighting_factor
				+ *ifft_sample * *fade_in_sample * weighting_factor;

		// step further
		output_sample++; ifft_sample++;
		fade_in_sample++; fade_out_sample++;
	}
	_old_weighting_factor = weighting_factor;
}

/** This is an autarc function to precalculate the frequency 
 * domain filter partitions that a \b Convolver needs. It does 
 * not require an instantiation of a \b Convolver. However, it is 
 * not very efficient since an FFT plan is created with every call.
 * @param container place to store the partitions.
 * @param filter impulse response of the filter
 * @param filter_size size of the impulse response
 * @param partition_size size of the partitions (this is the 
 * partition size that the outside world sees, internally it is twice as long)
 */
void 
Convolver::prepare_impulse_response(data_t& container, const float *filter,
		const unsigned int filter_size,
		const unsigned int partition_size)
{

	// find out how many complete partitions we have
	unsigned int no_of_partitions = filter_size/partition_size;

	// if there is even one more
	if (filter_size%partition_size) no_of_partitions++;

	// empty container
	container.clear();

	// allocate memory
	container.resize(2*no_of_partitions*partition_size, 0.0f);

	// define temporary buffers
	data_t fft_buffer;
	data_t zeros;

	// allocate memory and initialize to 0
	fft_buffer.resize(2*partition_size, 0.0f);
	zeros.resize(2*partition_size, 0.0f);

	// create fft plans for halfcomplex data format
	fftwf_plan fft_plan = fftwf_plan_r2r_1d(2*partition_size, &fft_buffer[0],
			&fft_buffer[0], FFTW_R2HC, FFTW_ESTIMATE);

	// convert filter partitionwise to frequency domain

	/////// process complete partitions //////////////

	for (unsigned int partition = 0u; partition < no_of_partitions-1; partition++)
	{
		std::copy(filter + partition*partition_size,
				filter + (partition+1)*partition_size,
				fft_buffer.begin());

		// zero pad
		std::copy(zeros.begin(), zeros.begin() + partition_size, fft_buffer.begin() + partition_size);

		// fft
		fftwf_execute(fft_plan);
		sort_coefficients(fft_buffer, 2*partition_size);

		// add the partition to the filter
		std::copy(fft_buffer.begin(), fft_buffer.begin() + 2*partition_size,
				container.begin() + 2*partition*partition_size);

	}

	////// end process complete partitions

	//// process potentially incomplete last partition ////////////

	// zeros
	std::copy(zeros.begin(),
			zeros.end(),
			fft_buffer.begin());

	// add filter coefficients
	std::copy(filter + (no_of_partitions-1)*partition_size,
			filter + filter_size,
			fft_buffer.begin());

	// fft
	fftwf_execute(fft_plan);
	sort_coefficients(fft_buffer, 2*partition_size);

	// add the partition to the filter
	std::copy(fft_buffer.begin(), fft_buffer.end(),
			container.begin() + 2*(no_of_partitions-1)*partition_size);

	///// end process potentially incomplete partition ////////

	// clean up
	fftwf_destroy_plan(fft_plan);
}


/** Sort the FFT coefficients to be in proper place for the efficient 
 * multiplication of the spectra.
 */
void 
Convolver::_sort_coefficients()
{
	const unsigned int buffer_size = 2*_frame_size;
	data_t buffer(buffer_size);

	int base = 8;

	unsigned int i,ii;

	buffer[0] = _fft_buffer[0];
	buffer[1] = _fft_buffer[1];
	buffer[2] = _fft_buffer[2];
	buffer[3] = _fft_buffer[3];
	buffer[4] = _fft_buffer[_frame_size];
	buffer[5] = _fft_buffer[buffer_size - 1];
	buffer[6] = _fft_buffer[buffer_size - 2];
	buffer[7] = _fft_buffer[buffer_size - 3];

	for (i=0; i<(buffer_size/8-1); i++)
	{
		for (ii = 0; ii < 4; ii++)
		{
			buffer[base+ii] = _fft_buffer[base/2+ii];
		}

		for (ii = 0; ii < 4; ii++)
		{
			buffer[base+4+ii] = _fft_buffer[buffer_size-base/2-ii];
		}

		base += 8;
	}

	std::copy(buffer.begin(), buffer.end(), _fft_buffer.begin());

}

/** Inverse function of \b Convolver::_sort_coefficients() .
 */
void
Convolver::_unsort_coefficients()
{
	const unsigned int buffer_size = 2*_frame_size;
	data_t buffer(buffer_size);

	int base = 8;

	unsigned int i,ii;

	buffer[0]             = _ifft_buffer[0];
	buffer[1]             = _ifft_buffer[1];
	buffer[2]             = _ifft_buffer[2];
	buffer[3]             = _ifft_buffer[3];
	buffer[_frame_size]   = _ifft_buffer[4];
	buffer[buffer_size-1] = _ifft_buffer[5];
	buffer[buffer_size-2] = _ifft_buffer[6];
	buffer[buffer_size-3] = _ifft_buffer[7];

	for (i=0; i<(buffer_size/8-1); i++)
	{
		for (ii = 0; ii < 4; ii++)
		{
			buffer[base/2+ii] = _ifft_buffer[base+ii];
		}

		for (ii = 0; ii < 4; ii++)
		{
			buffer[buffer_size-base/2-ii] = _ifft_buffer[base+4+ii];
		}

		base += 8;
	}

	std::copy(buffer.begin(), buffer.end(), _ifft_buffer.begin());

}

/** static version */
void 
Convolver::sort_coefficients(data_t& coefficients,
		const unsigned int partition_size)
{
	const unsigned int buffer_size = partition_size;
	data_t buffer(buffer_size);

	int base = 8;

	unsigned int i,ii;

	buffer[0] = coefficients[0];
	buffer[1] = coefficients[1];
	buffer[2] = coefficients[2];
	buffer[3] = coefficients[3];
	buffer[4] = coefficients[buffer_size/2];
	buffer[5] = coefficients[buffer_size - 1];
	buffer[6] = coefficients[buffer_size - 2];
	buffer[7] = coefficients[buffer_size - 3];

	for (i=0; i<(buffer_size/8-1); i++)
	{
		for (ii = 0; ii < 4; ii++)
		{
			buffer[base+ii] = coefficients[base/2+ii];
		}

		for (ii = 0; ii < 4; ii++)
		{
			buffer[base+4+ii] = coefficients[buffer_size-base/2-ii];
		}

		base += 8;
	}

	std::copy(buffer.begin(), buffer.end(), coefficients.begin());

}


void 
Convolver::_fft()
{
	fftwf_execute(_fft_plan);
	_sort_coefficients();
}

void 
Convolver::_ifft()
{
	_unsort_coefficients();
	fftwf_execute(_ifft_plan);
}

// Settings for Vim (http://www.vim.org/), please do not remove:
// vim:softtabstop=2:shiftwidth=2:expandtab:textwidth=80:cindent
