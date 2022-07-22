#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#if defined __has_include
#  if __has_include (<direct.h>)
#    include <direct.h>
#  endif
#else
#  include <unistd.h>
#endif
#include "main.h"


/*
	Only supports signed 16-bit PCM .WAV files
*/

Wave wave;

FILE* fp;
uint32_t num_samples;
uint16_t bytes_per_sample;
unsigned char two_byte_buffer[2];
unsigned char four_byte_buffer[4];

// Converts an array of two bytes in little-endian form to big-endian form
uint16_t convert_16_to_big_endian(unsigned char* little_endian) {
	uint16_t big_endian = little_endian[0] | (little_endian[1] << 8);
	return big_endian;
}

// Converts an array of four bytes in little-endian form to big-endian form
uint32_t convert_32_to_big_endian(unsigned char* little_endian) {
	uint32_t big_endian = little_endian[0] | (little_endian[1] << 8) | (little_endian[2] << 16) | (little_endian[3] << 24);
	return big_endian;
}

// Reads a .wav file into the global Wave struct
void read_wav() {

	fread(wave.header.riff, sizeof(wave.header.riff), 1, fp);

	fread(four_byte_buffer, sizeof(four_byte_buffer), 1, fp);
	wave.header.total_size = convert_32_to_big_endian(four_byte_buffer);

	fread(wave.header.type, sizeof(wave.header.type), 1, fp);

	fread(wave.header.fmt_marker, sizeof(wave.header.fmt_marker), 1, fp);

	fread(four_byte_buffer, sizeof(four_byte_buffer), 1, fp);
	wave.header.fmt_length = convert_32_to_big_endian(four_byte_buffer);

	fread(two_byte_buffer, sizeof(two_byte_buffer), 1, fp);
	wave.header.fmt_type = convert_16_to_big_endian(two_byte_buffer);

	fread(two_byte_buffer, sizeof(two_byte_buffer), 1, fp);
	wave.header.num_channels = convert_16_to_big_endian(two_byte_buffer);

	fread(four_byte_buffer, sizeof(four_byte_buffer), 1, fp);
	wave.header.sample_rate = convert_32_to_big_endian(four_byte_buffer);

	fread(four_byte_buffer, sizeof(four_byte_buffer), 1, fp);
	wave.header.byte_rate = convert_32_to_big_endian(four_byte_buffer);

	fread(two_byte_buffer, sizeof(two_byte_buffer), 1, fp);
	wave.header.block_align = convert_16_to_big_endian(two_byte_buffer);

	fread(two_byte_buffer, sizeof(two_byte_buffer), 1, fp);
	wave.header.bits_per_sample = convert_16_to_big_endian(two_byte_buffer);

	fread(wave.header.data_marker, sizeof(wave.header.data_marker), 1, fp);

	fread(four_byte_buffer, sizeof(four_byte_buffer), 1, fp);
	wave.header.data_length = convert_32_to_big_endian(four_byte_buffer);

	num_samples = wave.header.data_length / (wave.header.bits_per_sample * wave.header.num_channels);

	bytes_per_sample = (wave.header.bits_per_sample * wave.header.num_channels) / 8;

	// Allocate memory for the sample data
	wave.samples = calloc(num_samples, bytes_per_sample);
	if (wave.samples == NULL) {
		printf("Could not assign memory for samples.\n");
		exit(1);
	}

	// Read the sample data from the .wav file into the Wave struct
	int i;
	for (i = 0; i < num_samples; i++) {
		fread(two_byte_buffer, sizeof(two_byte_buffer), 1, fp);
		wave.samples[i] = convert_16_to_big_endian(two_byte_buffer);
	}
}

// Prints the header values of a .wav file
void print_header() {
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
	printf("        \t Number of Samples: %u\n", num_samples);
	printf("        \t Bytes per Sample: %u\n", bytes_per_sample);
}

// Prints the samples in big-endian format
void print_samples() {
	int i;
	for (i = 0; i < num_samples; i++) {
		printf("[%i]: %x\n", i, wave.samples[i]);
	}
}

int main(int argc, char* argv[]) {
	// Check args length
	if (argc < 2) {
		printf("First argument must be a .wav file in the current directory\n");
		exit(1);
	}

	// Build filepath
	char filepath[1024];
	if (_getcwd(filepath, sizeof(filepath)) == NULL) {
		printf("Error getting working directory\n");
		exit(1);
	}
	strcat(filepath, "\\");
	strcat(filepath, argv[1]);
	printf("Filepath: %s\n", filepath);

	// Open file
	fp = fopen(filepath, "rb");
	if (fp == NULL) {
		printf("Error opening file\n");
		exit(1);
	}

	// Read file contents
	read_wav();
	
	// Print wave.header info
	print_header();

	// Print wave.samples data
	// print_samples();

	// Compress its contents

	// Decompress its contents

	// Save the wav file

	// Close and exit
	fclose(fp);
	free(wave.samples);
	exit(0);
}