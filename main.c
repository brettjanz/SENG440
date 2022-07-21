#include <stdio.h>
#include <stdint.h>
#include "main.h"

// Converts an array of two-bytes in 
uint16_t convert_16_to_big_endian(unsigned char* little_endian) {
	uint16_t big_endian = little_endian[0] | (little_endian[1] << 8);
	return big_endian;
}

uint32_t convert_32_to_big_endian(unsigned char* little_endian) {
	uint32_t big_endian = little_endian[0] | (little_endian[1] << 8) | (little_endian[2] << 16) | (little_endian[3] << 24);
	return big_endian;
}

void print_wav_info(FILE* fp) {
	Header header;
	unsigned char two_byte_buffer[2];
	unsigned char four_byte_buffer[4];

	fread(header.riff, sizeof(header.riff), 1, fp);
	printf("(1-4): %s\n", header.riff);

	fread(four_byte_buffer, sizeof(four_byte_buffer), 1, fp);
	printf("(5-8)[BE]: %u%u%u%u\n", four_byte_buffer[0], four_byte_buffer[1], four_byte_buffer[2], four_byte_buffer[3]);

	header.total_size = convert_32_to_big_endian(four_byte_buffer);
	printf("(5-8)[BE]: %u\n", header.total_size);
}

int main(int argc, char* argv[]) {
	// Read the wav file
	if (argc < 2) {
		printf("First argument must be a .wav file in the current directory\n");
		return 0;
	}

	char* filename = argv[1];
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL) {
		printf("Error opening file\n");
		return 0;
	}

	print_wav_info(fp);
	return 0;
	// Compress its contents

	// Decompress its contents

	// Save the wav file
}