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
#include "main.h"


/*
	Only supports signed 16-bit PCM .WAV files
*/

Wave wave;
CompressedWave compressed_wave;

FILE* input_file;
FILE* output_file;
uint32_t num_samples;
uint16_t bytes_per_sample;
unsigned char two_byte_buffer[2];
unsigned char four_byte_buffer[4];
time_t start, end;

// Converts an array of two bytes in little-endian form to big-endian form
uint16_t convert_16_to_big_endian(unsigned char* little_endian) {
	uint16_t big_endian = little_endian[0] | (little_endian[1] << 8);
	return big_endian;
}

// Converts a 16-bit unsigned int in big-endian form to an array of two bytes in little-endian form
unsigned char* convert_16_to_little_endian(uint16_t big_endian) {
	two_byte_buffer[0] = big_endian & 0x00FF;
	two_byte_buffer[1] = (big_endian & 0xFF00) >> 8;
	return two_byte_buffer;
}

// Converts an array of four bytes in little-endian form to big-endian form
uint32_t convert_32_to_big_endian(unsigned char* little_endian) {
	uint32_t big_endian = little_endian[0] | (little_endian[1] << 8) | (little_endian[2] << 16) | (little_endian[3] << 24);
	return big_endian;
}

// Converts a 32-bit unsigned int in big-endian form to an array of four bytes in little-endian form
unsigned char* convert_32_to_little_endian(uint32_t big_endian) {
	four_byte_buffer[0] = big_endian & 0x000000FF;
	four_byte_buffer[1] = (big_endian & 0x0000FF00) >> 8;
	four_byte_buffer[2] = (big_endian & 0x00FF0000) >> 16;
	four_byte_buffer[3] = (big_endian & 0xFF000000) >> 24;
	return four_byte_buffer;
}

// Reads a .wav file into the global Wave struct
void read_wav() {

	// Read header
	fread(wave.header.riff, sizeof(wave.header.riff), 1, input_file);

	fread(four_byte_buffer, sizeof(four_byte_buffer), 1, input_file);
	wave.header.total_size = convert_32_to_big_endian(four_byte_buffer);

	fread(wave.header.type, sizeof(wave.header.type), 1, input_file);

	fread(wave.header.fmt_marker, sizeof(wave.header.fmt_marker), 1, input_file);

	fread(four_byte_buffer, sizeof(four_byte_buffer), 1, input_file);
	wave.header.fmt_length = convert_32_to_big_endian(four_byte_buffer);

	fread(two_byte_buffer, sizeof(two_byte_buffer), 1, input_file);
	wave.header.fmt_type = convert_16_to_big_endian(two_byte_buffer);

	fread(two_byte_buffer, sizeof(two_byte_buffer), 1, input_file);
	wave.header.num_channels = convert_16_to_big_endian(two_byte_buffer);

	fread(four_byte_buffer, sizeof(four_byte_buffer), 1, input_file);
	wave.header.sample_rate = convert_32_to_big_endian(four_byte_buffer);

	fread(four_byte_buffer, sizeof(four_byte_buffer), 1, input_file);
	wave.header.byte_rate = convert_32_to_big_endian(four_byte_buffer);

	fread(two_byte_buffer, sizeof(two_byte_buffer), 1, input_file);
	wave.header.block_align = convert_16_to_big_endian(two_byte_buffer);

	fread(two_byte_buffer, sizeof(two_byte_buffer), 1, input_file);
	wave.header.bits_per_sample = convert_16_to_big_endian(two_byte_buffer);

	fread(wave.header.data_marker, sizeof(wave.header.data_marker), 1, input_file);

	fread(four_byte_buffer, sizeof(four_byte_buffer), 1, input_file);
	wave.header.data_length = convert_32_to_big_endian(four_byte_buffer);

	// Calculate some useful numbers
	bytes_per_sample = (wave.header.bits_per_sample * wave.header.num_channels) / 8;
	num_samples = wave.header.data_length / (bytes_per_sample * wave.header.num_channels);

	// Allocate memory for the sample data
	wave.samples = calloc(num_samples, bytes_per_sample);
	if (wave.samples == NULL) {
		printf("Could not allocate memory for samples.\n");
		exit(1);
	}

	// Read the sample data from the .wav file into the Wave struct
	int i;
	for (i = 0; i < num_samples; i++) {
		fread(two_byte_buffer, sizeof(two_byte_buffer), 1, input_file);
		wave.samples[i] = (int16_t)convert_16_to_big_endian(two_byte_buffer);
	}
}

void write_wav() {
	
	// Write to header
	fwrite(wave.header.riff, sizeof(wave.header.riff), 1, output_file);

	convert_32_to_little_endian(wave.header.total_size);
	fwrite(four_byte_buffer, sizeof(four_byte_buffer), 1, output_file);

	fwrite(wave.header.type, sizeof(wave.header.type), 1, output_file);

	fwrite(wave.header.fmt_marker, sizeof(wave.header.fmt_marker), 1, output_file);

	convert_32_to_little_endian(wave.header.fmt_length);
	fwrite(four_byte_buffer, sizeof(four_byte_buffer), 1, output_file);

	convert_16_to_little_endian(wave.header.fmt_type);
	fwrite(two_byte_buffer, sizeof(two_byte_buffer), 1, output_file);

	convert_16_to_little_endian(wave.header.num_channels);
	fwrite(two_byte_buffer, sizeof(two_byte_buffer), 1, output_file);

	convert_32_to_little_endian(wave.header.sample_rate);
	fwrite(four_byte_buffer, sizeof(four_byte_buffer), 1, output_file);

	convert_32_to_little_endian(wave.header.byte_rate);
	fwrite(four_byte_buffer, sizeof(four_byte_buffer), 1, output_file);

	convert_16_to_little_endian(wave.header.block_align);
	fwrite(two_byte_buffer, sizeof(two_byte_buffer), 1, output_file);

	convert_16_to_little_endian(wave.header.bits_per_sample);
	fwrite(two_byte_buffer, sizeof(two_byte_buffer), 1, output_file);

	fwrite(wave.header.data_marker, sizeof(wave.header.data_marker), 1, output_file);

	convert_32_to_little_endian(wave.header.data_length);
	fwrite(four_byte_buffer, sizeof(four_byte_buffer), 1, output_file);

	// Write data
	int i;
	for (i = 0; i < num_samples; i++) {
		convert_16_to_little_endian((uint16_t)wave.samples[i]);
		fwrite(two_byte_buffer, sizeof(two_byte_buffer), 1, output_file);
	}
}

void compress_data() {

	compressed_wave.samples = calloc(num_samples, sizeof(uint8_t));
	if (compressed_wave.samples == NULL) {
		printf("Could not allocate memory for compressed samples.\n");
		exit(1);
	}

	int16_t sample;
	uint16_t mag;
	uint8_t sign;
	uint8_t codeword;

	int i;
	for (i = 0; i < num_samples; i++) {
		// printf("Full Sample: %i\n", wave.samples[i]);
		sample = (wave.samples[i] >> 2); // Only 14 bits are needed for mu-Law
		// printf("Sample[14]: %i\n", sample);

		// Convert the sample into sign-magnitude representation
		sign = signum(sample);
		mag = magnitude(sample) + 33; // Bias of 33 added so that each threshold is a power of 2

		// Find the codeword according to the mu-law encoding table
		codeword = get_codeword(sign, mag);
		// printf("Codeword: %i\n", codeword);

		// Perform bit-wise inversion of the codeword
		codeword = ~codeword;

		compressed_wave.samples[i] = codeword;
	}
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
void decompress_data() {
	int16_t sample;
	uint16_t mag;
	uint8_t sign;
	uint8_t codeword;

	int i;
	for (i = 0; i < num_samples; i++) {
		codeword = compressed_wave.samples[i];
		codeword = ~codeword;
		// printf("Decompressed Codeword: %i\n", codeword);
		sign = compressed_signum(codeword);
		mag = compressed_magnitude(codeword);
		// printf("Decompressed Sign: %i\n", sign);
		// printf("Biased Magnitude: %i\n", mag);
		mag -= 33;
		sample = (int16_t)(sign ? -mag : mag);
		// printf("Decompressed Sample [14]: %i\n", sample);
		wave.samples[i] = (sample << 2);
		// printf("Full Decompressed Sample: %i\n\n\n", wave.samples[i]);
	}
}

// Prints the header values of a .wav file
void print_header() {
	printf("================== HEADER ====================\n");
	printf("(1-4):\t\t %.4s\n", wave.header.riff);
	printf("(5-8):\t\t Total Size: %u bytes, %ukb\n", wave.header.total_size, wave.header.total_size / 1024);
	printf("(9-12):\t\t %.4s\n", wave.header.type);
	printf("(13-16):\t %.3s\n", wave.header.fmt_marker);
	printf("(17-20):\t Format Length: %u bytes\n", wave.header.fmt_length);
	printf("(21-22):\t Format Type: %u\n", wave.header.fmt_type);
	printf("(23-24):\t Channels: %u\n", wave.header.num_channels);
	printf("(25-28):\t Sample Rate: %u Hz\n", wave.header.sample_rate);
	printf("(29-32):\t Byte Rate: %u bytes/s\n", wave.header.byte_rate);
	printf("(33-34):\t Block Align: %u\n", wave.header.block_align);
	printf("(35-36):\t Bits Per Sample: %u\n", wave.header.bits_per_sample);
	printf("(37-40):\t %.4s\n", wave.header.data_marker);
	printf("(40-44):\t Data Length: %u bytes, %ukb\n", wave.header.data_length, wave.header.data_length / 1024);
	printf("==============================================\n\n");
	printf("Number of Samples: %u\n", num_samples);
	printf("Bytes per Sample: %u\n\n", bytes_per_sample);
}

// Prints the samples in big-endian format
void print_samples() {
	int i;
	for (i = 0; i < num_samples; i++) {
		printf("[%i]: %x\n", i, wave.samples[i]);
	}
}

void test() {
	uint16_t input = 10102 << 2;
	printf("Input: %x\n", input);
	int16_t sample = (int16_t)input; // Only 14 bits are needed for mu-Law
	sample = sample >> 2;
	printf("Sample: %i\n", sample);

	// Convert the sample into sign-magnitude representation
	uint8_t sign = signum(sample);
	uint16_t mag = magnitude(sample) + 33; // Bias of 33 added so that each threshold is a power of 2

	// Find the codeword according to the mu-law encoding table
	uint8_t codeword = get_codeword(sign, mag);

	// Perform bit-wise inversion of the codeword
	// codeword = ~codeword;
	printf("Codeword: %x\n", codeword);

	sign = compressed_signum(codeword);
	printf("Sign: %x\n", sign);
	mag = compressed_magnitude(codeword) - 33;
	printf("Mag: %x\n", mag);
	sample = (sign << 13) | mag;
	printf("Sample: %i\n", (int16_t)sample);
}

int main(int argc, char* argv[]) {
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
	strcat(input_filepath, "/");
	strcat(input_filepath, argv[1]);
	printf("\nUsing file: %s\n\n", input_filepath);

	// Open file
	input_file = fopen(input_filepath, "rb");
	if (input_file == NULL) {
		printf("Error opening file\n");
		exit(1);
	}

	// Read file contents
	read_wav();

	// Close input file
	fclose(input_file);
	
	// Print wave.header info
	print_header();

	// Print wave.samples data
	// print_samples();

	// Compress its contents
	start = clock();
	compress_data();
	end = clock();
	printf("Compressed %u samples in %us\n", num_samples, (uint32_t)((end - start) / CLOCKS_PER_SEC));

	// Decompress its contents
	start = clock();
	decompress_data();
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
	write_wav();

	// Close and exit
	fclose(output_file);
	free(wave.samples);
	free(compressed_wave.samples);
	
	//test();
	exit(0);
}