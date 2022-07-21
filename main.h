// Source: http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
typedef struct WAV_HEADER {
	// NOTE: Bytes are stored in little-endian and need to be converted to big-endian before use
	unsigned char riff[4];			// "RIFF"
	uint32_t total_size;			// Total size of the wav file
	unsigned char type[4];			// File type = "WAVE"
	unsigned char fmt_marker[4];	// Marks the start of the format = "fmt\0"
	uint32_t int fmt_length;		// Length of format data
	uint16_t fmt_type;				// Format type = 1 (PCM)
	uint16_t num_channels;			// Number of channels, 1 = Mono, 2 = Stereo
	uint32_t sample_rate;			// Sample rate in Hz (44100 commonly)
	uint32_t byte_rate;				// (Sample Rate * BitsPerSample * Channels) / 8
	uint16_t block_align;			// (BitsPerSample * Channels) / 8 [1 = 8 bit mono, 2 = 8 bit stereo/16 bit mono, 4 = 16 bit stereo]
	uint16_t bits_per_sample;		// Bits per sample
	unsigned char data_marker[4];	// Marks the start of the data = "data"
	uint32_t data_length;			// Length of the data
} Header;

uint16_t convert_to_big_endian(unsigned char* little_endian);
uint32_t convert_to_big_endian(unsigned char* little_endian);

void print_wav_info(FILE* fp);