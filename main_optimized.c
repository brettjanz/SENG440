#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
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

	int16_t sample1, sample2, sample3, sample4, sample5, sample6, sample7, sample8;
	uint16_t mag1, mag2, mag3, mag4, mag5, mag6, mag7, mag8;
	uint8_t sign1, sign2, sign3, sign4, sign5, sign6, sign7, sign8;
	uint8_t codeword1, codeword2, codeword3, codeword4, codeword5, codeword6, codeword7, codeword8;

	int i = 0;
	while (num_samples >= 8){
		sample1 = (wave_ptr->samples[i] >> 2);
		sample2 = (wave_ptr->samples[i + 1] >> 2);
		sample3 = (wave_ptr->samples[i + 2] >> 2);
		sample4 = (wave_ptr->samples[i + 3] >> 2);
		sample5 = (wave_ptr->samples[i + 4] >> 2);
		sample6 = (wave_ptr->samples[i + 5] >> 2);
		sample7 = (wave_ptr->samples[i + 6] >> 2);
		sample8 = (wave_ptr->samples[i + 7] >> 2);

		sign1 = signum(sample1);
		sign2 = signum(sample2);
		sign3 = signum(sample3);
		sign4 = signum(sample4);
		sign5 = signum(sample5);
		sign6 = signum(sample6);
		sign7 = signum(sample7);
		sign8 = signum(sample8);

		mag1 = magnitude(sample1) + 33;
		mag2 = magnitude(sample2) + 33;
		mag3 = magnitude(sample3) + 33;
		mag4 = magnitude(sample4) + 33;
		mag5 = magnitude(sample5) + 33;
		mag6 = magnitude(sample6) + 33;
		mag7 = magnitude(sample7) + 33;
		mag8 = magnitude(sample8) + 33;

		codeword1 = get_codeword(sign1, mag1);
		codeword2 = get_codeword(sign2, mag2);
		codeword3 = get_codeword(sign3, mag3);
		codeword4 = get_codeword(sign4, mag4);
		codeword5 = get_codeword(sign5, mag5);
		codeword6 = get_codeword(sign6, mag6);
		codeword7 = get_codeword(sign7, mag7);
		codeword8 = get_codeword(sign8, mag8);

		codeword1 = ~codeword1;
		codeword2 = ~codeword2;
		codeword3 = ~codeword3;
		codeword4 = ~codeword4;
		codeword5 = ~codeword5;
		codeword6 = ~codeword6;
		codeword7 = ~codeword7;
		codeword8 = ~codeword8;

		compressed_samples[i] = codeword1;
		compressed_samples[i + 1] = codeword2;
		compressed_samples[i + 2] = codeword3;
		compressed_samples[i + 3] = codeword4;
		compressed_samples[i + 4] = codeword5;
		compressed_samples[i + 5] = codeword6;
		compressed_samples[i + 6] = codeword7;
		compressed_samples[i + 7] = codeword8;

		num_samples -= 8;
		i += 8;
	}

	switch (num_samples){
		case 7:
			sample1 = (wave_ptr->samples[i] >> 2);
			sample2 = (wave_ptr->samples[i + 1] >> 2);
			sample3 = (wave_ptr->samples[i + 2] >> 2);
			sample4 = (wave_ptr->samples[i + 3] >> 2);
			sample5 = (wave_ptr->samples[i + 4] >> 2);
			sample6 = (wave_ptr->samples[i + 5] >> 2);
			sample7 = (wave_ptr->samples[i + 6] >> 2);

			sign1 = signum(sample1);
			sign2 = signum(sample2);
			sign3 = signum(sample3);
			sign4 = signum(sample4);
			sign5 = signum(sample5);
			sign6 = signum(sample6);
			sign7 = signum(sample7);

			mag1 = magnitude(sample1) + 33;
			mag2 = magnitude(sample2) + 33;
			mag3 = magnitude(sample3) + 33;
			mag4 = magnitude(sample4) + 33;
			mag5 = magnitude(sample5) + 33;
			mag6 = magnitude(sample6) + 33;
			mag7 = magnitude(sample7) + 33;

			codeword1 = get_codeword(sign1, mag1);
			codeword2 = get_codeword(sign2, mag2);
			codeword3 = get_codeword(sign3, mag3);
			codeword4 = get_codeword(sign4, mag4);
			codeword5 = get_codeword(sign5, mag5);
			codeword6 = get_codeword(sign6, mag6);
			codeword7 = get_codeword(sign7, mag7);

			codeword1 = ~codeword1;
			codeword2 = ~codeword2;
			codeword3 = ~codeword3;
			codeword4 = ~codeword4;
			codeword5 = ~codeword5;
			codeword6 = ~codeword6;
			codeword7 = ~codeword7;

			compressed_samples[i] = codeword1;
			compressed_samples[i + 1] = codeword2;
			compressed_samples[i + 2] = codeword3;
			compressed_samples[i + 3] = codeword4;
			compressed_samples[i + 4] = codeword5;
			compressed_samples[i + 5] = codeword6;
			compressed_samples[i + 6] = codeword7;
			break;
		
		case 6:
			sample1 = (wave_ptr->samples[i] >> 2);
			sample2 = (wave_ptr->samples[i + 1] >> 2);
			sample3 = (wave_ptr->samples[i + 2] >> 2);
			sample4 = (wave_ptr->samples[i + 3] >> 2);
			sample5 = (wave_ptr->samples[i + 4] >> 2);
			sample6 = (wave_ptr->samples[i + 5] >> 2);

			sign1 = signum(sample1);
			sign2 = signum(sample2);
			sign3 = signum(sample3);
			sign4 = signum(sample4);
			sign5 = signum(sample5);
			sign6 = signum(sample6);

			mag1 = magnitude(sample1) + 33;
			mag2 = magnitude(sample2) + 33;
			mag3 = magnitude(sample3) + 33;
			mag4 = magnitude(sample4) + 33;
			mag5 = magnitude(sample5) + 33;
			mag6 = magnitude(sample6) + 33;

			codeword1 = get_codeword(sign1, mag1);
			codeword2 = get_codeword(sign2, mag2);
			codeword3 = get_codeword(sign3, mag3);
			codeword4 = get_codeword(sign4, mag4);
			codeword5 = get_codeword(sign5, mag5);
			codeword6 = get_codeword(sign6, mag6);

			codeword1 = ~codeword1;
			codeword2 = ~codeword2;
			codeword3 = ~codeword3;
			codeword4 = ~codeword4;
			codeword5 = ~codeword5;
			codeword6 = ~codeword6;

			compressed_samples[i] = codeword1;
			compressed_samples[i + 1] = codeword2;
			compressed_samples[i + 2] = codeword3;
			compressed_samples[i + 3] = codeword4;
			compressed_samples[i + 4] = codeword5;
			compressed_samples[i + 5] = codeword6;
			break;

		case 5:
			sample1 = (wave_ptr->samples[i] >> 2);
			sample2 = (wave_ptr->samples[i + 1] >> 2);
			sample3 = (wave_ptr->samples[i + 2] >> 2);
			sample4 = (wave_ptr->samples[i + 3] >> 2);
			sample5 = (wave_ptr->samples[i + 4] >> 2);

			sign1 = signum(sample1);
			sign2 = signum(sample2);
			sign3 = signum(sample3);
			sign4 = signum(sample4);
			sign5 = signum(sample5);

			mag1 = magnitude(sample1) + 33;
			mag2 = magnitude(sample2) + 33;
			mag3 = magnitude(sample3) + 33;
			mag4 = magnitude(sample4) + 33;
			mag5 = magnitude(sample5) + 33;

			codeword1 = get_codeword(sign1, mag1);
			codeword2 = get_codeword(sign2, mag2);
			codeword3 = get_codeword(sign3, mag3);
			codeword4 = get_codeword(sign4, mag4);
			codeword5 = get_codeword(sign5, mag5);

			codeword1 = ~codeword1;
			codeword2 = ~codeword2;
			codeword3 = ~codeword3;
			codeword4 = ~codeword4;
			codeword5 = ~codeword5;

			compressed_samples[i] = codeword1;
			compressed_samples[i + 1] = codeword2;
			compressed_samples[i + 2] = codeword3;
			compressed_samples[i + 3] = codeword4;
			compressed_samples[i + 4] = codeword5;

			break;

		case 4:
			sample1 = (wave_ptr->samples[i] >> 2);
			sample2 = (wave_ptr->samples[i + 1] >> 2);
			sample3 = (wave_ptr->samples[i + 2] >> 2);
			sample4 = (wave_ptr->samples[i + 3] >> 2);

			sign1 = signum(sample1);
			sign2 = signum(sample2);
			sign3 = signum(sample3);
			sign4 = signum(sample4);

			mag1 = magnitude(sample1) + 33;
			mag2 = magnitude(sample2) + 33;
			mag3 = magnitude(sample3) + 33;
			mag4 = magnitude(sample4) + 33;

			codeword1 = get_codeword(sign1, mag1);
			codeword2 = get_codeword(sign2, mag2);
			codeword3 = get_codeword(sign3, mag3);
			codeword4 = get_codeword(sign4, mag4);

			codeword1 = ~codeword1;
			codeword2 = ~codeword2;
			codeword3 = ~codeword3;
			codeword4 = ~codeword4;

			compressed_samples[i] = codeword1;
			compressed_samples[i + 1] = codeword2;
			compressed_samples[i + 2] = codeword3;
			compressed_samples[i + 3] = codeword4;
			break;

		case 3:
			sample1 = (wave_ptr->samples[i] >> 2);
			sample2 = (wave_ptr->samples[i + 1] >> 2);
			sample3 = (wave_ptr->samples[i + 2] >> 2);

			sign1 = signum(sample1);
			sign2 = signum(sample2);
			sign3 = signum(sample3);

			mag1 = magnitude(sample1) + 33;
			mag2 = magnitude(sample2) + 33;
			mag3 = magnitude(sample3) + 33;

			codeword1 = get_codeword(sign1, mag1);
			codeword2 = get_codeword(sign2, mag2);
			codeword3 = get_codeword(sign3, mag3);

			codeword1 = ~codeword1;
			codeword2 = ~codeword2;
			codeword3 = ~codeword3;

			compressed_samples[i] = codeword1;
			compressed_samples[i + 1] = codeword2;
			compressed_samples[i + 2] = codeword3;
			break;

		case 2:
			sample1 = (wave_ptr->samples[i] >> 2);
			sample2 = (wave_ptr->samples[i + 1] >> 2);

			sign1 = signum(sample1);
			sign2 = signum(sample2);

			mag1 = magnitude(sample1) + 33;
			mag2 = magnitude(sample2) + 33;

			codeword1 = get_codeword(sign1, mag1);
			codeword2 = get_codeword(sign2, mag2);

			codeword1 = ~codeword1;
			codeword2 = ~codeword2;

			compressed_samples[i] = codeword1;
			compressed_samples[i + 1] = codeword2;
			break;

		case 1:
			sample1 = (wave_ptr->samples[i] >> 2);

			sign1 = signum(sample1);

			mag1 = magnitude(sample1) + 33;

			codeword1 = get_codeword(sign1, mag1);

			codeword1 = ~codeword1;

			compressed_samples[i] = codeword1;
			break;

		default:
			printf("Error compressing samples\n");
			exit(1);
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
	
	int16_t sample1, sample2, sample3, sample4, sample5, sample6, sample7, sample8;
	uint16_t mag1, mag2, mag3, mag4, mag5, mag6, mag7, mag8;
	uint8_t sign1, sign2, sign3, sign4, sign5, sign6, sign7, sign8;
	uint8_t codeword1, codeword2, codeword3, codeword4, codeword5, codeword6, codeword7, codeword8;

	int i = 0;
	while (num_samples >= 8){
		codeword1 = compressed_samples[i];
		codeword2 = compressed_samples[i + 1];
		codeword3 = compressed_samples[i + 2];
		codeword4 = compressed_samples[i + 3];
		codeword5 = compressed_samples[i + 4];
		codeword6 = compressed_samples[i + 5];
		codeword7 = compressed_samples[i + 6];
		codeword8 = compressed_samples[i + 7];

		codeword1 = ~codeword1;
		codeword2 = ~codeword2;
		codeword3 = ~codeword3;
		codeword4 = ~codeword4;
		codeword5 = ~codeword5;
		codeword6 = ~codeword6;
		codeword7 = ~codeword7;
		codeword8 = ~codeword8;

		sign1 = compressed_signum(codeword1);
		sign2 = compressed_signum(codeword2);
		sign3 = compressed_signum(codeword3);
		sign4 = compressed_signum(codeword4);
		sign5 = compressed_signum(codeword5);
		sign6 = compressed_signum(codeword6);
		sign7 = compressed_signum(codeword7);
		sign8 = compressed_signum(codeword8);

		mag1 = compressed_magnitude(codeword1) - 33;
		mag2 = compressed_magnitude(codeword2) - 33;
		mag3 = compressed_magnitude(codeword3) - 33;
		mag4 = compressed_magnitude(codeword4) - 33;
		mag5 = compressed_magnitude(codeword5) - 33;
		mag6 = compressed_magnitude(codeword6) - 33;
		mag7 = compressed_magnitude(codeword7) - 33;
		mag8 = compressed_magnitude(codeword8) - 33;

		sample1 = (int16_t)(sign1 ? -mag1 : mag1);
		sample2 = (int16_t)(sign2 ? -mag2 : mag2);
		sample3 = (int16_t)(sign3 ? -mag3 : mag3);
		sample4 = (int16_t)(sign4 ? -mag4 : mag4);
		sample5 = (int16_t)(sign5 ? -mag5 : mag5);
		sample6 = (int16_t)(sign6 ? -mag6 : mag6);
		sample7 = (int16_t)(sign7 ? -mag7 : mag7);
		sample8 = (int16_t)(sign8 ? -mag8 : mag8);

		wave_ptr->samples[i] = (sample1 << 2);
		wave_ptr->samples[i + 1] = (sample2 << 2);
		wave_ptr->samples[i + 2] = (sample3 << 2);
		wave_ptr->samples[i + 3] = (sample4 << 2);
		wave_ptr->samples[i + 4] = (sample5 << 2);
		wave_ptr->samples[i + 5] = (sample6 << 2);
		wave_ptr->samples[i + 6] = (sample7 << 2);
		wave_ptr->samples[i + 7] = (sample8 << 2);

		num_samples -= 8;
		i += 8;
	}

	switch (num_samples){
		case 7:
			codeword1 = compressed_samples[i];
			codeword2 = compressed_samples[i + 1];
			codeword3 = compressed_samples[i + 2];
			codeword4 = compressed_samples[i + 3];
			codeword5 = compressed_samples[i + 4];
			codeword6 = compressed_samples[i + 5];
			codeword7 = compressed_samples[i + 6];

			codeword1 = ~codeword1;
			codeword2 = ~codeword2;
			codeword3 = ~codeword3;
			codeword4 = ~codeword4;
			codeword5 = ~codeword5;
			codeword6 = ~codeword6;
			codeword7 = ~codeword7;

			sign1 = compressed_signum(codeword1);
			sign2 = compressed_signum(codeword2);
			sign3 = compressed_signum(codeword3);
			sign4 = compressed_signum(codeword4);
			sign5 = compressed_signum(codeword5);
			sign6 = compressed_signum(codeword6);
			sign7 = compressed_signum(codeword7);

			mag1 = compressed_magnitude(codeword1) - 33;
			mag2 = compressed_magnitude(codeword2) - 33;
			mag3 = compressed_magnitude(codeword3) - 33;
			mag4 = compressed_magnitude(codeword4) - 33;
			mag5 = compressed_magnitude(codeword5) - 33;
			mag6 = compressed_magnitude(codeword6) - 33;
			mag7 = compressed_magnitude(codeword7) - 33;

			sample1 = (int16_t)(sign1 ? -mag1 : mag1);
			sample2 = (int16_t)(sign2 ? -mag2 : mag2);
			sample3 = (int16_t)(sign3 ? -mag3 : mag3);
			sample4 = (int16_t)(sign4 ? -mag4 : mag4);
			sample5 = (int16_t)(sign5 ? -mag5 : mag5);
			sample6 = (int16_t)(sign6 ? -mag6 : mag6);
			sample7 = (int16_t)(sign7 ? -mag7 : mag7);

			wave_ptr->samples[i] = (sample1 << 2);
			wave_ptr->samples[i + 1] = (sample2 << 2);
			wave_ptr->samples[i + 2] = (sample3 << 2);
			wave_ptr->samples[i + 3] = (sample4 << 2);
			wave_ptr->samples[i + 4] = (sample5 << 2);
			wave_ptr->samples[i + 5] = (sample6 << 2);
			wave_ptr->samples[i + 6] = (sample7 << 2);
			break;

		case 6:
			codeword1 = compressed_samples[i];
			codeword2 = compressed_samples[i + 1];
			codeword3 = compressed_samples[i + 2];
			codeword4 = compressed_samples[i + 3];
			codeword5 = compressed_samples[i + 4];
			codeword6 = compressed_samples[i + 5];

			codeword1 = ~codeword1;
			codeword2 = ~codeword2;
			codeword3 = ~codeword3;
			codeword4 = ~codeword4;
			codeword5 = ~codeword5;
			codeword6 = ~codeword6;

			sign1 = compressed_signum(codeword1);
			sign2 = compressed_signum(codeword2);
			sign3 = compressed_signum(codeword3);
			sign4 = compressed_signum(codeword4);
			sign5 = compressed_signum(codeword5);
			sign6 = compressed_signum(codeword6);

			mag1 = compressed_magnitude(codeword1) - 33;
			mag2 = compressed_magnitude(codeword2) - 33;
			mag3 = compressed_magnitude(codeword3) - 33;
			mag4 = compressed_magnitude(codeword4) - 33;
			mag5 = compressed_magnitude(codeword5) - 33;
			mag6 = compressed_magnitude(codeword6) - 33;

			sample1 = (int16_t)(sign1 ? -mag1 : mag1);
			sample2 = (int16_t)(sign2 ? -mag2 : mag2);
			sample3 = (int16_t)(sign3 ? -mag3 : mag3);
			sample4 = (int16_t)(sign4 ? -mag4 : mag4);
			sample5 = (int16_t)(sign5 ? -mag5 : mag5);
			sample6 = (int16_t)(sign6 ? -mag6 : mag6);

			wave_ptr->samples[i] = (sample1 << 2);
			wave_ptr->samples[i + 1] = (sample2 << 2);
			wave_ptr->samples[i + 2] = (sample3 << 2);
			wave_ptr->samples[i + 3] = (sample4 << 2);
			wave_ptr->samples[i + 4] = (sample5 << 2);
			wave_ptr->samples[i + 5] = (sample6 << 2);
			break;

		case 5:
			codeword1 = compressed_samples[i];
			codeword2 = compressed_samples[i + 1];
			codeword3 = compressed_samples[i + 2];
			codeword4 = compressed_samples[i + 3];
			codeword5 = compressed_samples[i + 4];

			codeword1 = ~codeword1;
			codeword2 = ~codeword2;
			codeword3 = ~codeword3;
			codeword4 = ~codeword4;
			codeword5 = ~codeword5;

			sign1 = compressed_signum(codeword1);
			sign2 = compressed_signum(codeword2);
			sign3 = compressed_signum(codeword3);
			sign4 = compressed_signum(codeword4);
			sign5 = compressed_signum(codeword5);

			mag1 = compressed_magnitude(codeword1) - 33;
			mag2 = compressed_magnitude(codeword2) - 33;
			mag3 = compressed_magnitude(codeword3) - 33;
			mag4 = compressed_magnitude(codeword4) - 33;
			mag5 = compressed_magnitude(codeword5) - 33;

			sample1 = (int16_t)(sign1 ? -mag1 : mag1);
			sample2 = (int16_t)(sign2 ? -mag2 : mag2);
			sample3 = (int16_t)(sign3 ? -mag3 : mag3);
			sample4 = (int16_t)(sign4 ? -mag4 : mag4);
			sample5 = (int16_t)(sign5 ? -mag5 : mag5);

			wave_ptr->samples[i] = (sample1 << 2);
			wave_ptr->samples[i + 1] = (sample2 << 2);
			wave_ptr->samples[i + 2] = (sample3 << 2);
			wave_ptr->samples[i + 3] = (sample4 << 2);
			wave_ptr->samples[i + 4] = (sample5 << 2);
			break;

		case 4:
			codeword1 = compressed_samples[i];
			codeword2 = compressed_samples[i + 1];
			codeword3 = compressed_samples[i + 2];
			codeword4 = compressed_samples[i + 3];

			codeword1 = ~codeword1;
			codeword2 = ~codeword2;
			codeword3 = ~codeword3;
			codeword4 = ~codeword4;

			sign1 = compressed_signum(codeword1);
			sign2 = compressed_signum(codeword2);
			sign3 = compressed_signum(codeword3);
			sign4 = compressed_signum(codeword4);

			mag1 = compressed_magnitude(codeword1) - 33;
			mag2 = compressed_magnitude(codeword2) - 33;
			mag3 = compressed_magnitude(codeword3) - 33;
			mag4 = compressed_magnitude(codeword4) - 33;

			sample1 = (int16_t)(sign1 ? -mag1 : mag1);
			sample2 = (int16_t)(sign2 ? -mag2 : mag2);
			sample3 = (int16_t)(sign3 ? -mag3 : mag3);
			sample4 = (int16_t)(sign4 ? -mag4 : mag4);

			wave_ptr->samples[i] = (sample1 << 2);
			wave_ptr->samples[i + 1] = (sample2 << 2);
			wave_ptr->samples[i + 2] = (sample3 << 2);
			wave_ptr->samples[i + 3] = (sample4 << 2);
			break;

		case 3:
			codeword1 = compressed_samples[i];
			codeword2 = compressed_samples[i + 1];
			codeword3 = compressed_samples[i + 2];

			codeword1 = ~codeword1;
			codeword2 = ~codeword2;
			codeword3 = ~codeword3;

			sign1 = compressed_signum(codeword1);
			sign2 = compressed_signum(codeword2);
			sign3 = compressed_signum(codeword3);

			mag1 = compressed_magnitude(codeword1) - 33;
			mag2 = compressed_magnitude(codeword2) - 33;
			mag3 = compressed_magnitude(codeword3) - 33;

			sample1 = (int16_t)(sign1 ? -mag1 : mag1);
			sample2 = (int16_t)(sign2 ? -mag2 : mag2);
			sample3 = (int16_t)(sign3 ? -mag3 : mag3);

			wave_ptr->samples[i] = (sample1 << 2);
			wave_ptr->samples[i + 1] = (sample2 << 2);
			wave_ptr->samples[i + 2] = (sample3 << 2);
			break;

		case 2:
			codeword1 = compressed_samples[i];
			codeword2 = compressed_samples[i + 1];

			codeword1 = ~codeword1;
			codeword2 = ~codeword2;

			sign1 = compressed_signum(codeword1);
			sign2 = compressed_signum(codeword2);

			mag1 = compressed_magnitude(codeword1) - 33;
			mag2 = compressed_magnitude(codeword2) - 33;

			sample1 = (int16_t)(sign1 ? -mag1 : mag1);
			sample2 = (int16_t)(sign2 ? -mag2 : mag2);

			wave_ptr->samples[i] = (sample1 << 2);
			wave_ptr->samples[i + 1] = (sample2 << 2);
			break;

		case 1:
			codeword1 = compressed_samples[i];

			codeword1 = ~codeword1;

			sign1 = compressed_signum(codeword1);

			mag1 = compressed_magnitude(codeword1) - 33;

			sample1 = (int16_t)(sign1 ? -mag1 : mag1);

			wave_ptr->samples[i] = (sample1 << 2);
			break;
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
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
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