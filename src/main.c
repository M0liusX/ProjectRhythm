#include <soundio/soundio.h>

// #include <glm/vec4.hpp>
// #include <glm/mat4x4.hpp>

#include "wave.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include <curses.h>

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
    int err;

    while (frames_left > 0) {
        int frame_count = frames_left;
        int pos = play(&wavePlayer, frame_count);
        //printf("%d\n", pos);
        if (pos == -1){
            //soundio_outstream_pause(outstream, true);
            //printf("Called");
            return 1;
        }

        if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
            fprintf(stderr, "%s\n", soundio_strerror(err));
            exit(1);
        }

        if (!frame_count)
            break;
            
        for (int frame = 0; frame < frame_count; frame += 1) {
            for (int channel = 0; channel < layout->channel_count; channel += 1) {
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

int main(int argc, char **argv) {
    int err;
    struct SoundIo *soundio = soundio_create();

    //Obtain Wave
    char* filename = (char*) malloc(sizeof(char) * 1024);
    if (filename == NULL) {
    printf("Error in malloc\n");
    exit(1);
    }

    // get file path
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
   
        strcpy(filename, cwd);

        // get filename from command line
        if (argc < 2) {
            printf("No wave file specified\n");
            return -1;
        }
	
        strcat(filename, "/");
        strcat(filename, argv[1]);
        printf("%s\n", filename);
    }
    
    //struct WAVE wave_file;
    parse(filename, &wave);

    //Init Input Code
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    
    printf("HI\n");
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
    
    for (;;) {
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

    if ((err = soundio_outstream_start(outstream))) {
        fprintf(stderr, "unable to start device: %s", soundio_strerror(err));
        return 1;
    }


    char ch;
    while( (ch = getch()) == ERR) {}
    soundio_outstream_destroy(outstream);
    wavePlayer.pos = 0;
    }

    soundio_device_unref(device);
    soundio_destroy(soundio);

    hello_world();
    return 0;
}
