/*
 * FractionalDelay.h
 *
 *	Class to achieve variable fractional delay using band-limited interpolation
 *	thanks to libsamplerate. See <http://www.mega-nerd.com/SRC/> and <https://ccrma.stanford.edu/~jos/resample/>.
 *	The target delay determines the resampling sampling rate for the input_buffer.
 *	By generating more samples from this input buffer, subsequent blocks will be delayed.
 *	With smoothing off, the target delay is reached by the next buffer,
 *	otherwise it will change rate gradually, taking slightly longer but sounding better.
 *
 *	The SRC has internal buffering so the delayed samples will appear in subsequent calls to apply_delay.
 *	This class is intended for use in real-time streaming.
 *
 *	The SRC instance has a transport delay, which is unavoidable. Any specified delay will be added to this,
 *	so a specified delay of 0 seconds will still have a delay of transport_delay samples. If the processed signals
 *	need sychronising with other signals then they will also need delaying by this amount.
 *
 *	To allow negative delay you need to wrap this class with a ring buffer or something.
 *
 *  Created on: 15 Aug 2013
 *      Author: Chris Pike
 */

#ifndef FRACTIONALDELAY_H_
#define FRACTIONALDELAY_H_

#include <samplerate.h>
#include <vector>
#include <inttypes.h> // for uint32_t

class FractionalDelay {
public:

    typedef uint32_t nframes_t;

	FractionalDelay(float sample_rate, nframes_t buffer_size, bool smooth_delay_adjustment = true);
	virtual ~FractionalDelay();

	/// perform a resampling to get towards target delay
	float* apply_delay(float* input_buffer, const nframes_t nframes_in, float target_delay);

	const nframes_t get_transport_delay();

	nframes_t get_buffer_size() const
	{
		return _buffer_size;
	}

	float get_current_delay() const
	{
		return _current_delay;
	}

	bool is_smoothing() const
	{
		return _smoothing;
	}

	void set_smoothing(bool smoothing)
	{
		_smoothing = smoothing;
	}

private:
	void _set_ratio(); ///< used to explicitly set ratio (when not smoothing)
	void _do_src();

	float _sample_rate;
	nframes_t _transport_delay;
	nframes_t _buffer_size;
    float _current_delay;
    bool _smoothing;

    SRC_STATE* _src;
	SRC_DATA _src_data;

	std::vector<float> _output_buffer;
};

#endif /* FRACTIONALDELAY_H_ */
