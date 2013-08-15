/*
 * ConvolverFFT.hpp
 *
 *  Fast frequency-domain convolution using FFTW. No partitioning is performed in this convolver.
 *  It should be used when accurate convolution is required, generally not for real-time processing.
 *  This class is based on the SoundScapeRenderer Convolver class (for real-time partitioned convolution).
 *  That code is under GPLv3 and therefore so is this.
 *
 *  Copyright (c) 2006-2012 Quality & Usability Lab                            *
 *                          Deutsche Telekom Laboratories, TU Berlin           *
 *                          Ernst-Reuter-Platz 7, 10587 Berlin, Germany        *
 *  http://tu-berlin.de/?id=ssr                  SoundScapeRenderer@telekom.de *
 *
 *  Copyright (c) 2013- 	BBC Research & Development
 *
 *  Created on: Aug 14, 2013
 *      Author: Chris Pike
 */

#ifndef CONVOLVERFFT_H_
#define CONVOLVERFFT_H_

#include <fftw3.h>
#include <vector>
#include <stdexcept>
#include <memory> // for auto_ptr
#include <inttypes.h> // for uint32_t

class ConvolverFFT {
public:

    typedef uint32_t nframes_t; // same as jack_nframes_t!

    typedef float sample_t;

    typedef std::vector<sample_t> data_t;

	typedef std::auto_ptr<ConvolverFFT> ptr_t; ///< auto_ptr to ConvolverFFT

	static ptr_t create(const nframes_t fft_size = 1024);
	ConvolverFFT(const nframes_t fft_size)
		throw (std::bad_alloc, std::runtime_error);
	virtual ~ConvolverFFT();

	void set_filter_t(const data_t& filter_t);
	void set_filter_f(data_t& filter_f);
	void set_neutral_filter(const nframes_t length);

	static void prepare_impulse_response(const data_t& filter_t, data_t& filter_f, const unsigned int fft_size);
	static void sort_coefficients(data_t& coefficients, const unsigned int fft_size);

	void convolve_signal(data_t& signal, data_t& output, sample_t weighting_factor = 1.0f);
private:
	nframes_t _fft_length;
	nframes_t _filter_length;

	data_t  _fft_buffer; ///< one partition
	data_t _ifft_buffer; ///< one partition

	/// vector holding time-domain filter coefficients to process
	data_t _filter_coefficients_t;
	/// vector holding frequency-domain filter coefficients to process
	data_t _filter_coefficients_f;
	/// vector holding frequency-domain signal coefficients to process
	data_t _signal_coefficients_f;

	data_t _zeros; ///< vector holding zeros

	fftwf_plan  _fft_plan;
	fftwf_plan _ifft_plan;

	void _update_filter_partitions();
	void _multiply_spectra();
	void _multiply_spectra_cpp(const float* signal, const float* filter);
#ifdef __SSE__
	void _multiply_spectra_simd(const float* signal, const float* filter);
#endif

	void _sort_coefficients();
	void _unsort_coefficients();

	void _fft();
	void _ifft();
};

#endif /* CONVOLVERFFT_H_ */
