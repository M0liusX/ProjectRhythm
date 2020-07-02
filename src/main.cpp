#include <soundio/soundio.h>
#include <gainput/gainput.h>
#include <windows.h>

// #include <glm/vec4.hpp>
// #include <glm/mat4x4.hpp>

#include "wave.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
//#include <curses.h>

#define SFW_LOG(...) { char buf[1024]; sprintf(buf, __VA_ARGS__); OutputDebugStringA(buf); }

enum Button
{
    ButtonConfirm
};

void hello_world() {
   printf("Hello World!\n");
}

struct WAVE wave;
struct WAVE_PLAYER wavePlayer = {&wave, 0};

static const float PI = 3.1415926535f;
static float seconds_offset = 0.0f;
static void write_callback(struct SoundIoOutStream *outstream,
        int frame_count_min, int frame_count_max)
{
    const struct SoundIoChannelLayout *layout = &outstream->layout;
    float float_sample_rate = outstream->sample_rate;
    float seconds_per_frame = 1.0f / float_sample_rate;
    struct SoundIoChannelArea *areas;
    int frames_left = frame_count_max;
	if (frame_count_max > wave.num_samples) {
		frames_left = wave.num_samples;
	}
    int err;

    while (frames_left > 0) {
        int frame_count = frames_left;
        int pos = play(&wavePlayer, frame_count);
        if (pos == -1){
            return;
        }

        if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
            fprintf(stderr, "%s\n", soundio_strerror(err));
            exit(1);
        }

        if (!frame_count)
            break;
            
        for (int frame = 0; frame < frame_count; frame += 1) {
            for (int channel = 0; channel < layout->channel_count; channel += 1) {
				if (frame + pos >= wave.num_samples) { continue; }
                float *ptr = (float*)(areas[channel].ptr + areas[channel].step * frame);
                *ptr = wave.buffer[(frame+pos)*wave.header.channels + channel%wave.header.channels];
            }
        }

        seconds_offset = fmodf(seconds_offset +
            seconds_per_frame * frame_count, 1.0f);

        if ((err = soundio_outstream_end_write(outstream))) {
            fprintf(stderr, "%s\n", soundio_strerror(err));
            exit(1);
        }

        frames_left -= frame_count;
    }
}

int loop(HWND hWnd) {
    int err;
    struct SoundIo *soundio = soundio_create();

    //Obtain Wave
	char* file = "fast_snare.wav";
    parse(file, &wave);

    if (!soundio) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    if ((err = soundio_connect(soundio))) {
        fprintf(stderr, "error connecting: %s", soundio_strerror(err));
        return 1;
    }

    soundio_flush_events(soundio);

    int default_out_device_index = soundio_default_output_device_index(soundio);
    if (default_out_device_index < 0) {
        fprintf(stderr, "no output device found");
        return 1;
    }

    struct SoundIoDevice *device = soundio_get_output_device(soundio, default_out_device_index);
    if (!device) {
        fprintf(stderr, "out of memory");
        return 1;
    }

    fprintf(stderr, "Output device: %s\n", device->name);
    
	//Input Stuff
	gainput::InputManager manager;
	manager.SetDisplaySize(1024, 1024);
	const gainput::DeviceId keyboardId = manager.CreateDevice<gainput::InputDeviceKeyboard>();
	const gainput::DeviceId mouseId = manager.CreateDevice<gainput::InputDeviceMouse>();
	const gainput::DeviceId padId = manager.CreateDevice<gainput::InputDevicePad>();
	const gainput::DeviceId touchId = manager.CreateDevice<gainput::InputDeviceTouch>();

	gainput::InputMap map(manager);
	map.MapBool(ButtonConfirm, keyboardId, gainput::KeyReturn);
	map.MapBool(ButtonConfirm, keyboardId, gainput::KeyUp);
	map.MapBool(ButtonConfirm, keyboardId, gainput::KeyLeft);
	map.MapBool(ButtonConfirm, keyboardId, gainput::KeyRight);
	map.MapBool(ButtonConfirm, mouseId, gainput::MouseButtonLeft);
	map.MapBool(ButtonConfirm, padId, gainput::PadButtonA);
	map.MapBool(ButtonConfirm, touchId, gainput::Touch0Down);

	MSG msg;

	SFW_LOG("BS.\n");

   
    struct SoundIoOutStream *outstream = soundio_outstream_create(device);
    outstream->format = SoundIoFormatFloat32NE;
    outstream->write_callback = write_callback;
    outstream->sample_rate = (float)wave.header.sample_rate;

    if ((err = soundio_outstream_open(outstream))) {
        fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
        return 1;
    }

    if (outstream->layout_error)
        fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));


	//for (;;) {}

    if ((err = soundio_outstream_start(outstream))) {
        fprintf(stderr, "unable to start device: %s", soundio_strerror(err));
        return 1;
    }

	for (;;) {
	while (true) {
		manager.Update();
		while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			manager.HandleMessage(msg);
		}

		if (map.GetBool(ButtonConfirm)) {
			break;
		}
	}

	//for (;;) {}

    
	soundio_outstream_clear_buffer(outstream);
    wavePlayer.pos = 0;
	while (true) {
		manager.Update();
		while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			manager.HandleMessage(msg);
		}

		if (map.GetBoolWasDown(ButtonConfirm)) {
			break;
		}
	}
    }

	//soundio_outstream_destroy(outstream);
    soundio_device_unref(device);
    soundio_destroy(soundio);

    hello_world();
    return 0;
}



int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	const auto pClassName = "ProjectR";
	// register window class
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = DefWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = pClassName;
	wc.hIconSm = nullptr;
	RegisterClassEx(&wc);
	// create window instance
	HWND hWnd = CreateWindowEx(
		0, pClassName,
		"Happy Hard Menu",
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		200, 200, 640, 480,
		nullptr, nullptr, hInstance, nullptr
	);
	// show the damn window
	ShowWindow(hWnd, SW_SHOW);
	loop(hWnd);
	//while (true);
	return 0;
}