// WAVE file header format
struct HEADER {
	unsigned char riff[4];						// RIFF string
	unsigned int overall_size	;				// overall size of file in bytes
	unsigned char wave[4];						// WAVE string
	unsigned char fmt_chunk_marker[4];			// fmt string with trailing null char
	unsigned int length_of_fmt;					// length of the format data
	unsigned int format_type;					// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	unsigned int channels;						// no.of channels
	unsigned int sample_rate;					// sampling rate (blocks per second)
	unsigned int byterate;						// SampleRate * NumChannels * BitsPerSample/8
	unsigned int block_align;					// NumChannels * BitsPerSample/8
	unsigned int bits_per_sample;				// bits per sample, 8- 8bits, 16- 16 bits etc
	unsigned char data_chunk_header [4];		// DATA string or FLLR string
	unsigned int data_size;						// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};

struct WAVE {
    struct HEADER header;                       // Header file with all related information
	int num_samples;							// num of samples per channel
    float* buffer;                              // Sample data
};

struct WAVE_PLAYER {
	struct WAVE* wave;							// Reference to wave file
	int pos;									// Position in the buffer
};

char* seconds_to_time(float seconds);
int parse(char* filename, struct WAVE* wave);
int play(struct WAVE_PLAYER* wavePlayer, int frame_count);	// Returns int in wave buffer [-1: finished reading]