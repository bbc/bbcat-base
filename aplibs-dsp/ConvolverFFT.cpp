/*
 * ConvolverFFT.cpp
 *
 *  Created on: Aug 14, 2013
 *      Author: Chris Pike
 */

#include "ConvolverFFT.hpp"
#include <iostream>
#include <cmath>
#include <algorithm> // for std::copy, std::transform

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

/** Initialize the convolver.
 * It also sets the filter to be a Dirac. Thus, if no filter is specified
 * the audio data are not affected. However, quite some computational
 * load for nothing...
 *
 * You should use create() instead of directly using this constructor.
 * @throw std::bad_alloc if not enough memory could be allocated
 * @throw std::runtime_error if sizeof(float) != 4
 **/
ConvolverFFT::ConvolverFFT(const nframes_t fft_size)
throw (std::bad_alloc, std::runtime_error)
: _fft_length(fft_size)
, _fft_buffer(_fft_length,0.f)
, _ifft_buffer(_fft_length,0.f)
, _filter_coefficients_t(_fft_length,0.f)
, _filter_coefficients_f(_fft_length,0.f)
, _signal_coefficients_f(_fft_length,0.f)
, _zeros(_fft_length,0.f)
{
	// make sure that SIMD instructions can be used properly
	if (sizeof(float) != 4)
	{
		throw(std::runtime_error("sizeof(float) on your computer is not 4! "
				" The convolution can not take place properly."));
	}


	_fft_plan  = fftwf_plan_r2r_1d(_fft_length, &_fft_buffer[0],
			&_fft_buffer[0], FFTW_R2HC, FFTW_PATIENT);
	_ifft_plan = fftwf_plan_r2r_1d(_fft_length, &_ifft_buffer[0],
			&_ifft_buffer[0], FFTW_HC2R, FFTW_PATIENT);

	set_neutral_filter(fft_size);
}

ConvolverFFT::~ConvolverFFT()
{
	fftwf_destroy_plan (_fft_plan);
	fftwf_destroy_plan (_ifft_plan);
}

ConvolverFFT::ptr_t ConvolverFFT::create(const nframes_t fft_size)
{
	ptr_t temp;
	try
	{
		temp.reset(new ConvolverFFT(fft_size));
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
void ConvolverFFT::set_neutral_filter(const nframes_t length)
{
	data_t dirac(length,0.f);
	dirac[0] = 1.f;
	set_filter_t(dirac);
}

/** Sets the filter in the time-domain.
 * @param filter impulse response of the filter
 */
void ConvolverFFT::set_filter_t(const data_t& filter_t)
{

	if (filter_t.empty())
	{
//		WARNING("You are trying to use an empty filter.");
		std::cerr << "WARNING: " << "You are trying to use an empty filter." << std::endl;
		return;
	}

	_filter_coefficients_t = filter_t;
	_filter_length = filter_t.size();
	_filter_coefficients_f.clear(); // clear freq. domain version, to be set on next convolution
}

/** Sets a new filter by explicitly specifying frequency domain data.
 * If you use this method, it is your responsibility to ensure that you have performed adequate zero-padding.
 * @param filter vector holding the transfer function of the zero padded
 * filter in half-complex format (see fftw3 documentation).
 */
void ConvolverFFT::set_filter_f(data_t& filter_f)
{
	if (filter_f.empty()) return;
	if (filter_f.size() != _fft_length)
	{
//		WARNING("Frequency domain filter data is provided with incorrect size. Ignoring. Try using set_filter_t instead.");
		std::cerr << "WARNING: " << "Frequency domain filter data is provided with incorrect size. Ignoring. Try using set_filter_t instead." << std::endl;
		return;
	}

	std::copy(filter_f.begin(),filter_f.end(),_filter_coefficients_f.begin());
}

/** This is a static function to pre-calculate the frequency
 * domain filter that the \b Convolver needs. It does
 * not require an instantiation of a \b Convolver. An FFT plan is created
 * with every call so it is not very efficient, but if you will be using
 * a number of different filters of the same size (and input signals of
 * the same size) then it will be more efficient to use this with
 * \b ConvolverFFT::set_filter_f.
 * @param filter_t time-domain filter coefficients (impulse response).
 * @param filter_f frequency-domain filter coefficients
 * @param fft_size size of the fft that will be used
 */
void ConvolverFFT::prepare_impulse_response(const data_t& filter_t, data_t& filter_f, const unsigned int fft_size)
{
	data_t fft_buffer(fft_size,0.f);
	data_t zeros(fft_size,0.f);

	// create fft plans for half-complex data format
	fftwf_plan fft_plan = fftwf_plan_r2r_1d(fft_size, &fft_buffer[0],
			&fft_buffer[0], FFTW_R2HC, FFTW_PATIENT);

	// copy time-domain data into fft buffer
	std::copy(filter_t.begin(), filter_t.end(), fft_buffer.begin());

	// fft
	fftwf_execute(fft_plan);
	sort_coefficients(fft_buffer, fft_size);

	filter_f.clear();
	filter_f.resize(fft_size,0.f);

	// copy frequency domain data to filter container
	std::copy(fft_buffer.begin(),fft_buffer.end(),filter_f.begin());

	// clean up
	fftwf_destroy_plan(fft_plan);
}

/** static version */
void
ConvolverFFT::sort_coefficients(data_t& coefficients,
		const unsigned int fft_size)
{
	const unsigned int buffer_size = fft_size;
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

void ConvolverFFT::convolve_signal(data_t& signal, data_t& output, sample_t weighting_factor)
{
	nframes_t signal_length = signal.size();
	//nframes_t fft_length = (nframes_t) mathtools::next_power_of_2(signal_length+_filter_length-1);
	nframes_t fft_length = (nframes_t) pow(2.,ceil(log2((float)(signal_length+_filter_length-1))));
	output.resize(fft_length,0.f);

	// check that we have the correct fft length
	if (fft_length != _fft_length)
	{
		// need to re-plan FFTs
		fftwf_destroy_plan (_fft_plan);
		fftwf_destroy_plan (_ifft_plan);
		_fft_length = fft_length;
		_fft_buffer.resize(_fft_length,0.f);
		_ifft_buffer.resize(_fft_length,0.f);
		_signal_coefficients_f.resize(_fft_length,0.f);
		_zeros.resize(_fft_length,0.f);

		std::cout << "Calculating FFT plans. May take a little while." << std::endl;
		_fft_plan  = fftwf_plan_r2r_1d(_fft_length, &_fft_buffer[0],
				&_fft_buffer[0], FFTW_R2HC, FFTW_PATIENT);
		_ifft_plan = fftwf_plan_r2r_1d(_fft_length, &_ifft_buffer[0],
				&_ifft_buffer[0], FFTW_HC2R, FFTW_PATIENT);

		// clear frequency-domain filter data - will need to be recalculated
		_filter_coefficients_f.clear();
	}

	// calculate and store filter data if necessary
	if (_filter_coefficients_f.size() != _fft_length)
	{
		_filter_coefficients_f.resize(_fft_length,0.f);
		// fill fft buffer with zeros
		std::copy(_zeros.begin(),_zeros.end(),_fft_buffer.begin());
		// copy time-domain filter coefficients in.
		std::copy(_filter_coefficients_t.begin(),_filter_coefficients_t.end(),_fft_buffer.begin());

		_fft();

		std::copy(_fft_buffer.begin(),_fft_buffer.end(),_filter_coefficients_f.begin());
	}

	// convert signal to frequency-domain
	std::copy(_zeros.begin(),_zeros.end(),_fft_buffer.begin());
	std::copy(signal.begin(),signal.end(),_fft_buffer.begin());

	_fft();

	std::copy(_fft_buffer.begin(),_fft_buffer.end(),_signal_coefficients_f.begin());

//	// checking magnitude stuff
//	size_t real_ind = _fft_length/4;
//	size_t imag_ind = _fft_length - _fft_length/4;
//	double sweep_mag = mathtools::linear2dB(sqrt(mathtools::square(_signal_coefficients_f[real_ind]) + mathtools::square(_signal_coefficients_f[imag_ind])));
//	VERBOSE2("sig mag: " << sweep_mag);
//	double inv_sweep_mag = mathtools::linear2dB(sqrt(mathtools::square(_filter_coefficients_f[real_ind]) + mathtools::square(_filter_coefficients_f[imag_ind])));
//	VERBOSE2("filt mag: " << inv_sweep_mag);

	_multiply_spectra();

//	double out_mag = mathtools::linear2dB(sqrt(mathtools::square(_signal_coefficients_f[real_ind]) + mathtools::square(_signal_coefficients_f[imag_ind])));
//	VERBOSE2("out mag: " << out_mag);

	_ifft();

	// normalize with respect to the fft size (as well as specified weighting)
	weighting_factor *= 1.0f / _fft_length;

	// store data in output buffer and apply weighting factor
//	std::copy(_ifft_buffer.begin(),	_ifft_buffer.end(), output.begin());
	std::transform(_ifft_buffer.begin(), _ifft_buffer.end(), output.begin(),
	               std::bind1st(std::multiplies<sample_t>(),weighting_factor));
}

void ConvolverFFT::_multiply_spectra()
{
	// initialize _ifft_buffer
	std::copy(_zeros.begin(), _zeros.end(), _ifft_buffer.begin());

#ifdef __SSE__
		_multiply_spectra_simd( &(_signal_coefficients_f[0]),
				&(_filter_coefficients_f[0]) );
#else
		_multiply_spectra_cpp( &(_signal_coefficients_f[0]),
				&(_filter_coefficients_f[0]) );
#endif

}

void ConvolverFFT::_multiply_spectra_cpp(const float* signal, const float* filter)
{
	float d1s = _ifft_buffer[0] + signal[0] * filter[0];
	float d2s = _ifft_buffer[4] + signal[4] * filter[4];

	for (unsigned int nn = 0u; nn < _fft_length; nn += 8u)
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
void ConvolverFFT::_multiply_spectra_simd(const float* signal,
		const float* filter)
{
  // 16 byte alignment is needed for _mm_load_ps()!
  // This should be the case anyway because fftwf_malloc() is used.

  float dc = _ifft_buffer[0] + signal[0] * filter[0];
  float ny = _ifft_buffer[4] + signal[4] * filter[4];

  for(size_t i = 0; i < _fft_length; i += 8)
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

void ConvolverFFT::_sort_coefficients()
{
	const unsigned int buffer_size = _fft_length;
	data_t buffer(buffer_size);

	int base = 8;

	unsigned int i,ii;

	buffer[0] = _fft_buffer[0];
	buffer[1] = _fft_buffer[1];
	buffer[2] = _fft_buffer[2];
	buffer[3] = _fft_buffer[3];
	buffer[4] = _fft_buffer[_fft_length>>1];
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

void ConvolverFFT::_unsort_coefficients()
{
	const unsigned int buffer_size = _fft_length;
	data_t buffer(buffer_size);

	int base = 8;

	unsigned int i,ii;

	buffer[0]             = _ifft_buffer[0];
	buffer[1]             = _ifft_buffer[1];
	buffer[2]             = _ifft_buffer[2];
	buffer[3]             = _ifft_buffer[3];
	buffer[_fft_length>>1]   = _ifft_buffer[4];
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

void ConvolverFFT::_fft()
{
	fftwf_execute(_fft_plan);
	_sort_coefficients();
}

void ConvolverFFT::_ifft()
{
	_unsort_coefficients();
	fftwf_execute(_ifft_plan);
}
