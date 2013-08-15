/*
 * Convolver.hpp
 *
 *  Fast frequency-domain overlap-add convolution using uniform partition sizes and FFTW.
 *  Allows dynamic switching of impulse responses, such as required in head-tracked binaural synthesis.
 *  This class is copied from the SoundScapeRenderer Convolver class.
 *  That code is under GPLv3 and therefore so is this.
 *
 *  Copyright (c) 2006-2012 Quality & Usability Lab                            *
 *                          Deutsche Telekom Laboratories, TU Berlin           *
 *                          Ernst-Reuter-Platz 7, 10587 Berlin, Germany        *
 *                                                                             *
 *                                                                             *
 *  http://tu-berlin.de/?id=ssr                  SoundScapeRenderer@telekom.de *
 */

#ifndef CONVOLVER_H_
#define CONVOLVER_H_

#include <memory>
#include <vector>
#include <list>
#include <stdexcept>
#include <fftw3.h>
#include <utility>    // for std::pair
#include <inttypes.h> // for uint32_t

/** Convolution engine.
 * Uses (uniformly) partitioned convolution.
 **/
class Convolver
{
  public:
    /// type of crossfade to be used to fade between to adjacent filters 
    typedef enum
    {
      none,           ///< no crossfade
      raised_cosine,  ///< dito.
      linear          ///< linear slope
    } crossfade_t;

    typedef uint32_t nframes_t; // same as jack_nframes_t!

    typedef std::vector<float> data_t;

    typedef std::auto_ptr<Convolver> ptr_t; ///< auto_ptr to Convolver

    static ptr_t create(const nframes_t nframes
        , const crossfade_t crossfade_type = raised_cosine);

    Convolver(const nframes_t nframes, const crossfade_t crossfade_type)
      throw (std::bad_alloc, std::runtime_error);

    virtual ~Convolver();

    void set_filter_t(const data_t& filter);
    void set_filter_f(data_t& filter);
    void set_neutral_filter();

    static void prepare_impulse_response(data_t& container, const float *filter
        , const unsigned int filter_size, const unsigned int partition_size);

    static void sort_coefficients(data_t& coefficients
        , const unsigned int partition_size);

    float* convolve_signal(float *signal, float weighting_factor = 1.0f);

  private:
    typedef std::list< std::pair<data_t, unsigned int> > waiting_queue_t;

    const nframes_t _frame_size;
    const unsigned int _partition_size;
    const crossfade_t _crossfade_type;

    /// This is used to ensure proper fade-in and fade-out in conjunction
    /// with power saving functionality
    unsigned int _no_of_partitions_to_process;

    float _old_weighting_factor;

    /// this is a list holding the filters that are supposed 
    /// to be used for the convolution; it also holds the number of cycles
    /// that they have already waited 
    waiting_queue_t _waiting_queue;

    /// list holding the spectrum of the different double-frames 
    /// of input signal to be convolved
    /// the last element is the most recent signal chunk
    std::list<data_t> _signal;

    /// vector holding frequency domain filter coefficients to process 
    data_t _filter_coefficients;

    data_t _neutral_filter; ///< flat transfer function

    data_t _zeros; ///< two frames containing only zeros

    data_t _output_buffer;

    std::vector<float> _fade_in;
    std::vector<float> _fade_out;

    data_t  _fft_buffer; ///< one partition
    data_t _ifft_buffer; ///< one partition

    fftwf_plan  _fft_plan;
    fftwf_plan _ifft_plan;

    void _update_filter_partitions();
    void _multiply_spectra();
    void _multiply_partition_cpp(const float* signal, const float* filter);
#ifdef __SSE__
    void _multiply_partition_simd(const float* signal, const float* filter);
#endif
    void _normalize_buffer(data_t& buffer, float weighting_factor);
    void _normalize_buffer(float* sample, float weighting_factor);
    void _crossfade_into_buffer(data_t& buffer, float weighting_factor);

    void _sort_coefficients();
    void _unsort_coefficients();

    void _fft();
    void _ifft();

    /// check if buffer contains data
    bool _contains_data(const float* buffer
        , const unsigned buffer_size = 0) const;
};

#endif /* CONVOLVER_H_ */
