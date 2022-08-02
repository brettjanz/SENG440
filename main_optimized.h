// Source: http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
typedef struct HEADER {
	// NOTE: Bytes are stored in little-endian and need to be converted to big-endian before use
	unsigned char riff[4];			// "RIFF"
	uint32_t total_size;			// Total size of the wav file
	unsigned char type[4];			// File type = "WAVE"
	unsigned char fmt_marker[4];	// Marks the start of the format = "fmt\0"
	uint32_t fmt_length;			// Length of format data
	uint16_t fmt_type;				// Format type = 1 (PCM)
	uint16_t num_channels;			// Number of channels, 1 = Mono, 2 = Stereo
	uint32_t sample_rate;			// Sample rate in Hz (44100 commonly)
	uint32_t byte_rate;				// (Sample Rate * BitsPerSample * Channels) / 8
	uint16_t block_align;			// (BitsPerSample * Channels) / 8 [1 = 8 bit mono, 2 = 8 bit stereo/16 bit mono, 4 = 16 bit stereo]
	uint16_t bits_per_sample;		// Bits per sample = 16
	unsigned char data_marker[4];	// Marks the start of the data = "data"
	uint32_t data_length;			// Length of the data
} Header;

typedef struct WAVE {
	Header header;
	int16_t* samples;
}Wave;

typedef struct COMPRESSED_WAVE {
	Header header;
	uint8_t* samples;
}CompressedWave;

uint16_t convert_16_to_big_endian(unsigned char* little_endian);
unsigned char* convert_16_to_little_endian(uint16_t big_endian);
uint32_t convert_32_to_big_endian(unsigned char* little_endian);
unsigned char* convert_32_to_little_endian(uint32_t big_endian);

uint32_t read_wav(Wave* wave_ptr, FILE* input_file);
void write_wav(Wave* wave_ptr, uint32_t num_samples, FILE* output_file);
void print_header(Wave* wave_ptr, uint32_t num_samples);
void print_samples(Wave* wave_ptr, uint32_t num_samples);
uint8_t* compress_data(Wave* wave_ptr, uint32_t num_samples);
void decompress_data(Wave* wave_ptr, uint32_t num_samples, uint8_t* compressed_samples);
uint8_t signum(int16_t sample);
uint8_t compressed_signum(uint8_t codeword);
uint16_t magnitude(int16_t sample);
uint16_t compressed_magnitude(uint8_t codeword);
uint8_t get_codeword(uint8_t sign, uint16_t mag);