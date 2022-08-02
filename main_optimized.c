#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#if defined __has_include
#  if __has_include (<direct.h>)
#    include <direct.h>
#  endif
#else
#  include <unistd.h>
#  define _getcwd getcwd
#endif
#include "main_optimized.h"


/*
	Only supports signed 16-bit PCM .WAV files
*/

// Reads a .wav file into the global Wave struct
uint32_t read_wav(Wave* wave_ptr, FILE* input_file) {

	uint32_t num_samples;
	unsigned char header_buffer[44];
	
	fread(header_buffer, sizeof(unsigned char), 44, input_file);

	// Read header
	wave_ptr->header.riff[0] = header_buffer[0];
	wave_ptr->header.riff[1] = header_buffer[1];
	wave_ptr->header.riff[2] = header_buffer[2];
	wave_ptr->header.riff[3] = header_buffer[3];
	
	wave_ptr->header.total_size = header_buffer[4] | (header_buffer[5] << 8) | (header_buffer[6] << 16) | (header_buffer[7] << 24);

	wave_ptr->header.type[0] = header_buffer[8];
	wave_ptr->header.type[1] = header_buffer[9];
	wave_ptr->header.type[2] = header_buffer[10];
	wave_ptr->header.type[3] = header_buffer[11];

	wave_ptr->header.fmt_marker[0] = header_buffer[12];
	wave_ptr->header.fmt_marker[1] = header_buffer[13];
	wave_ptr->header.fmt_marker[2] = header_buffer[14];
	wave_ptr->header.fmt_marker[3] = header_buffer[15];

	wave_ptr->header.fmt_length = header_buffer[16] | (header_buffer[17] << 8) | (header_buffer[18] << 16) | (header_buffer[19] << 24);

	wave_ptr->header.fmt_type = header_buffer[20] | (header_buffer[21] << 8);

	wave_ptr->header.num_channels = header_buffer[22] | (header_buffer[23] << 8);

	wave_ptr->header.sample_rate = header_buffer[24] | (header_buffer[25] << 8) | (header_buffer[26] << 16) | (header_buffer[27] << 24);

	wave_ptr->header.byte_rate = header_buffer[28] | (header_buffer[29] << 8) | (header_buffer[30] << 16) | (header_buffer[31] << 24);

	wave_ptr->header.block_align = header_buffer[32] | (header_buffer[33] << 8);

	wave_ptr->header.bits_per_sample = header_buffer[34] | (header_buffer[35] << 8);

	wave_ptr->header.data_marker[0] = header_buffer[36];
	wave_ptr->header.data_marker[1] = header_buffer[37];
	wave_ptr->header.data_marker[2] = header_buffer[38];
	wave_ptr->header.data_marker[3] = header_buffer[39];

	wave_ptr->header.data_length = header_buffer[40] | (header_buffer[41] << 8) | (header_buffer[42] << 16) | (header_buffer[43] << 24);

	// Calculate some useful numbers
	uint16_t bytes_per_sample = (wave_ptr->header.bits_per_sample * wave_ptr->header.num_channels) / 8;
	num_samples = wave_ptr->header.data_length / (bytes_per_sample * wave_ptr->header.num_channels);

	// Allocate memory for the sample data
	wave_ptr->samples = calloc(num_samples, bytes_per_sample);
	if (wave_ptr->samples == NULL) {
		printf("Could not allocate memory for samples.\n");
		exit(1);
	}

	// Read the sample data from the .wav file into the Wave struct
	fread(wave_ptr->samples, bytes_per_sample, num_samples, input_file);

	return num_samples;
}

void write_wav(Wave* wave_ptr, uint32_t num_samples, FILE* output_file) {
	// Write header
	fwrite(&wave_ptr->header, sizeof(unsigned char), 44, output_file);

	// Write data
	fwrite(wave_ptr->samples, sizeof(uint16_t), num_samples, output_file);
}

uint8_t* compress_data(Wave* wave_ptr, uint32_t num_samples) {

	uint8_t* compressed_samples = calloc(num_samples, sizeof(uint8_t));
	if (compressed_samples == NULL) {
		printf("Could not allocate memory for compressed samples.\n");
		exit(1);
	}

	int16_t sample;
	uint16_t mag;
	uint8_t sign;
	uint8_t codeword;

	int i;
	for (i = 0; i < num_samples; i++) {
		// printf("Full Sample: %i\n", wave_ptr->samples[i]);
		sample = (wave_ptr->samples[i] >> 2); // Only 14 bits are needed for mu-Law
		// printf("Sample[14]: %i\n", sample);

		// Convert the sample into sign-magnitude representation
		sign = signum(sample);
		mag = magnitude(sample) + 33; // Bias of 33 added so that each threshold is a power of 2

		// Find the codeword according to the mu-law encoding table
		codeword = get_codeword(sign, mag);
		// printf("Codeword: %i\n", codeword);

		// Perform bit-wise inversion of the codeword
		codeword = ~codeword;

		compressed_samples[i] = codeword;
	}

	return compressed_samples;
}

// Returns the proper codeword from the mu-law encoding table
uint8_t get_codeword(uint8_t sign, uint16_t mag) {
	uint8_t chord, step;

	// Find the chord by calculating the location of the most signifigant 1
	if (mag & (1 << 12)) {
		chord = 0x7;

		// Extract the 4 step bits through masking
		step = (mag >> 8) & 0xF;
	}
	else if (mag & (1 << 11)) {
		chord = 0x6;
		step = (mag >> 7) & 0xF;
	}
	else if (mag & (1 << 10)) {
		chord = 0x5;
		step = (mag >> 6) & 0xF;
	}
	else if (mag & (1 << 9)) {
		chord = 0x4;
		step = (mag >> 5) & 0xF;
	}
	else if (mag & (1 << 8)) {
		chord = 0x3;
		step = (mag >> 4) & 0xF;
	}
	else if (mag & (1 << 7)) {
		chord = 0x2;
		step = (mag >> 3) & 0xF;
	}
	else if (mag & (1 << 6)) {
		chord = 0x1;
		step = (mag >> 2) & 0xF;
	}
	else if (mag & (1 << 5)) {
		chord = 0x0;
		step = (mag >> 1) & 0xF;
	}
	else {
		printf("Problem generating codeword\n");
		exit(1);
	}

	// printf("New Sign: %i\n", sign);
	// printf("Magnitude: %i\n", mag);
	// printf("Chord: %i\n", chord);
	// printf("Step: %i\n", step);
	// Assemble the sign, chord, and step bits into a compressed codeword
	return (sign << 7) | (chord << 4) | step;
}

// Returns opposite of codeword's 16th bit
uint8_t signum(int16_t sample) {
	return sample & (1 << 15) ? 0 : 1;
}

// Returns opposite of codeword's 8th bit
uint8_t compressed_signum(uint8_t codeword) {
	return codeword & (1 << 7) ? 0 : 1;
}

// Returns the absolute value of the sample value
uint16_t magnitude(int16_t sample) {
	if (sample < 0) {
		sample = -sample;
	}

	return (uint16_t)sample;
}

uint16_t compressed_magnitude(uint8_t codeword) {
	uint8_t chord = (codeword >> 4) & 0x7;
	// printf("Chord: %i\n", chord);
	uint8_t step = codeword & 0xF;
	// printf("Step: %i\n", step);
	uint16_t res;

	if (chord == 0x7) {
		return (1 << 12) | (step << 8) | (1 << 7);
	}
	else if (chord == 0x6) {
		return (1 << 11) | (step << 7) | (1 << 6);
	}
	else if (chord == 0x5) {
		return (1 << 10) | (step << 6) | (1 << 5);
	}
	else if (chord == 0x4) {
		return (1 << 9) | (step << 5) | (1 << 4);
	}
	else if (chord == 0x3) {
		return (1 << 8) | (step << 4) | (1 << 3);
	}
	else if (chord == 0x2) {
		return (1 << 7) | (step << 3) | (1 << 2);
	}
	else if (chord == 0x1) {
		return (1 << 6) | (step << 2) | (1 << 1);
	}
	else if (chord == 0x0) {
		return (1 << 5) | (step << 1) | 1;
	}
	else {
		printf("Problem retrieving magnitude from codeword\n");
		exit(1);
	}
}

// Performs the compression steps in reverse according to the mu-law decoding table to get back a full 16 bit sample
void decompress_data(Wave* wave_ptr, uint32_t num_samples, uint8_t* compressed_samples) {
	int16_t sample;
	uint16_t mag;
	uint8_t sign;
	uint8_t codeword;

	int i;
	for (i = 0; i < num_samples; i++) {
		codeword = compressed_samples[i];
		codeword = ~codeword;
		// printf("Decompressed Codeword: %i\n", codeword);
		sign = compressed_signum(codeword);
		mag = compressed_magnitude(codeword);
		// printf("Decompressed Sign: %i\n", sign);
		// printf("Biased Magnitude: %i\n", mag);
		mag -= 33;
		sample = (int16_t)(sign ? -mag : mag);
		// printf("Decompressed Sample [14]: %i\n", sample);
		wave_ptr->samples[i] = (sample << 2);
		// printf("Full Decompressed Sample: %i\n\n\n", wave_ptr->samples[i]);
	}
}

// Prints the header values of a .wav file
void print_header(Wave* wave_ptr, uint32_t num_samples) {
	printf("================== HEADER ====================\n");
	printf("(1-4):\t\t %.4s\n", wave_ptr->header.riff);
	printf("(5-8):\t\t Total Size: %u bytes, %ukb\n", wave_ptr->header.total_size, wave_ptr->header.total_size / 1024);
	printf("(9-12):\t\t %.4s\n", wave_ptr->header.type);
	printf("(13-16):\t %.3s\n", wave_ptr->header.fmt_marker);
	printf("(17-20):\t Format Length: %u bytes\n", wave_ptr->header.fmt_length);
	printf("(21-22):\t Format Type: %u\n", wave_ptr->header.fmt_type);
	printf("(23-24):\t Channels: %u\n", wave_ptr->header.num_channels);
	printf("(25-28):\t Sample Rate: %u Hz\n", wave_ptr->header.sample_rate);
	printf("(29-32):\t Byte Rate: %u bytes/s\n", wave_ptr->header.byte_rate);
	printf("(33-34):\t Block Align: %u\n", wave_ptr->header.block_align);
	printf("(35-36):\t Bits Per Sample: %u\n", wave_ptr->header.bits_per_sample);
	printf("(37-40):\t %.4s\n", wave_ptr->header.data_marker);
	printf("(40-44):\t Data Length: %u bytes, %ukb\n", wave_ptr->header.data_length, wave_ptr->header.data_length / 1024);
	printf("==============================================\n\n");
	printf("Number of Samples: %u\n", num_samples);
	printf("Bytes per Sample: %u\n\n", (wave_ptr->header.bits_per_sample * wave_ptr->header.num_channels) / 8);
}

// Prints the samples in big-endian format
void print_samples(Wave* wave_ptr, uint32_t num_samples) {
	int i;
	for (i = 0; i < num_samples; i++) {
		printf("[%i]: %x\n", i, wave_ptr->samples[i]);
	}
}

int main(int argc, char* argv[]) {

	FILE *input_file; 
	FILE* output_file;
	Wave* wave_ptr, wave;
	wave_ptr = &wave;
	time_t start, end;

	// Check args length
	if (argc < 3) {
		printf("Please provide both an input and output filepath\n");
		exit(1);
	}

	// Build input filepath
	char cwd[1024];
	char input_filepath[1024];
	if (_getcwd(cwd, sizeof(cwd)) == NULL) {
		printf("Error getting working directory\n");
		exit(1);
	}
	strcpy(input_filepath, cwd);
	strcat(input_filepath, "\\");
	strcat(input_filepath, argv[1]);
	printf("\nUsing file: %s\n\n", input_filepath);

	// Open file
	input_file = fopen(input_filepath, "rb");
	if (input_file == NULL) {
		printf("Error opening file\n");
		exit(1);
	}

	// Read file contents
	uint32_t num_samples = read_wav(wave_ptr, input_file);

	// Close input file
	fclose(input_file);
	
	// Print wave_ptr->header info
	print_header(wave_ptr, num_samples);

	// Print wave_ptr->samples data
	// print_samples(wave_ptr, num_samples);

	// Compress its contents
	start = clock();
	uint8_t* compressed_samples = compress_data(wave_ptr, num_samples);
	end = clock();
	printf("Compressed %u samples in %us\n", num_samples, (uint32_t)((end - start) / CLOCKS_PER_SEC));

	// Decompress its contents
	start = clock();
	decompress_data(wave_ptr, num_samples, compressed_samples);
	end = clock();
	printf("Decompressed %u samples in %us\n\n", num_samples, (uint32_t)((end - start) / CLOCKS_PER_SEC));
	
	// Build output filepath
	char output_filepath[1024];
	strcpy(output_filepath, cwd);
	strcat(output_filepath, "/");
	strcat(output_filepath, argv[2]);
	printf("\nSaving to file: %s\n\n", output_filepath);
	
	// Create file
	output_file = fopen(output_filepath, "wb");
	if (output_file == NULL) {
		printf("Error creating output file\n");
		exit(1);
	}

	// Write to file
	write_wav(wave_ptr, num_samples, output_file);

	// Close and exit
	fclose(output_file);
	free(wave_ptr->samples);
	free(compressed_samples);
	
	exit(0);
}