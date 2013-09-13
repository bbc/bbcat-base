/*
 * FractionalDelay.cpp
 *
 *  Created on: 15 Aug 2013
 *      Author: chrisp
 */

#include "FractionalDelay.hpp"
#include "Logging.hpp"

FractionalDelay::FractionalDelay(float sample_rate, nframes_t buffer_size, bool smooth_delay_adjustment)
: _sample_rate(sample_rate)
, _buffer_size(buffer_size)
, _smoothing(smooth_delay_adjustment)
, _output_buffer(buffer_size,0.0f)
{
	int src_error;
	_src = src_new(SRC_SINC_BEST_QUALITY, 1, &src_error); // 1 channel

	if (_src == NULL)
	{
		APLIBS_DSP_WARNING("Initialization of sample rate converter failed! " + std::string(src_strerror(src_error)));
		return;
	}

	if (smooth_delay_adjustment)
		APLIBS_DSP_WARNING("With smoothing on, the delay achieved cannot be guaranteed (especially when large), but it might sound better if delays are changing alot.");

	// do an initial conversion to get transport delay
	std::vector<float> silence_in(_buffer_size,0.0f);

	// setup src conversion
	_src_data.input_frames = _buffer_size;
	_src_data.output_frames = _buffer_size;
	_src_data.data_in = &silence_in[0];
	_src_data.data_out = &_output_buffer[0];
	_src_data.end_of_input = 0; // indicates that more audio will come (in next buffer)

	// do conversion and calculate transport delay
	_src_data.src_ratio = 1.0f;
	_set_ratio();
	_do_src();

	APLIBS_DSP_INFO("\tInput frames used (set-up): " << _src_data.input_frames_used);
	APLIBS_DSP_INFO("\tOutput frames gen (set-up): " << _src_data.output_frames_gen);


	_transport_delay = _src_data.input_frames - _src_data.output_frames_gen;
}

FractionalDelay::~FractionalDelay() {
	SRC_STATE* src_return = src_delete (_src);
	if (src_return != NULL)
	{
		APLIBS_DSP_ERROR("Problem deleting SRC instance.");
	}
}

float* FractionalDelay::apply_delay(float* input_buffer, const nframes_t nframes_in,
		float target_delay, nframes_t& nframes_generated)
{
	// TODO: when the delay change is too large and smoothing is on, the correct delay is not reached
	if (nframes_in > _buffer_size)
	{
		APLIBS_DSP_ERROR("Incorrect number of input frames. Could cause a segmentation fault.");
	}
	else if (nframes_in < _buffer_size)
	{
		APLIBS_DSP_WARNING("Fewer than expected input frames.");
	}

	if (target_delay < 0.0f)
	{
		APLIBS_DSP_WARNING("It isn't possible to create a negative delay. "
				<< "Fewer samples will be generated than have been supplied however.");
	}

	// setup src conversion
	_src_data.input_frames = nframes_in;
	_src_data.data_in = input_buffer;
	_src_data.src_ratio = _buffer_size / (_buffer_size - (target_delay - _current_delay)*_sample_rate);

	if (!_smoothing)
		_set_ratio();

	_do_src();

	APLIBS_DSP_INFO("\tSRC ratio: " << _src_data.src_ratio);
	APLIBS_DSP_INFO("\tInput frames used: " << _src_data.input_frames_used);
	APLIBS_DSP_INFO("\tOutput frames gen: " << _src_data.output_frames_gen);

	_current_delay = target_delay;

	nframes_generated = _src_data.output_frames_gen;

	if (_src_data.input_frames_used < _buffer_size)
	{
		APLIBS_DSP_WARNING("Fewer frames used than provided. (This doesn't seem to happen ever - if it does this class won't work)");
	}

	return &_output_buffer[0];
}

const FractionalDelay::nframes_t FractionalDelay::get_transport_delay() {
	return _transport_delay;
}

void FractionalDelay::_set_ratio() {
	int src_error = src_set_ratio(_src, _src_data.src_ratio);
	if (src_error) {
		APLIBS_DSP_WARNING("Sample rate conversion error: " + std::string(src_strerror (src_error)));
	}
}

void FractionalDelay::_do_src()
{
	int src_error = src_process(_src, &_src_data);
	if (src_error) {
		APLIBS_DSP_WARNING("Sample rate conversion error: " + std::string(src_strerror (src_error)));
	}
}
