# SENG440
A .wav file audio compressor and decompressor written in C and optimized for an ARM-based embedded system.  Created for UVic SENG 440: Embedded Systems.

# Usage
`ssh <netlinkID>@seng440.ece.uvic.ca`

`mkdir /tmp/<your-name>`

`git clone https://github.com/brettjanz/SENG440.git /tmp/<your-name>`

`cd /tmp/<your-name>`

`make`

# Makefile Options
- `make` : Build main.exe, deploy it to the arm machine, and run it with test.wav
- `make opt` : Build main_optimized.exe, deploy it to the arm machine, and run it with test.wav
- `make all` : Build both mains, deploy them to the arm machine, run both with test.wav, and compare their outputs
- `make clean` : Clean up any files on the arm machine

# Optimizations
## Global Variables
Global variables will never be placed in a register. We can change this by making variables local to the functions that use them
- `FILE* input_file`, `FILE* output_file`, `Wave* wave`, and `uint32_t num_samples` have been made local to `main()`
- `uint16_t bytes_per_sample` has been made local to `read_wav()`
- `CompressedWave compressedWave` is now `uint16_t* samples` local to `main()` and `compress_data()`

## If/Else Optimization
Since smaller values occur more often, we can change the order in which we check for different magnitudes in order to speed up execution
- `get_codeword()` now uses a switch case that checks lower values first
- `compressed_magnitude()` now uses a switch case that checks lower values first

## File Read/Write Minimization
We can save on overhead by reducing the amount of times we perform file I/O with `fread()` and `fwrite()`
- `read_wav()` has been reduced to two read operations, one for the header and one for the sample data
- `write_wav()` has been reduced to two write operations, one for the header and one for the sample data

## Function Inlining
We can save on overhead with simple but often-used functions by inlining their return value instead of making a function call
- `signum()` has been removed and had all its instances inlined
- `magnitude()` has been removed and had all its instances inlined
- `compressed_signum()` has been removed and had all its instances inlined
- `write_wav()` has been removed and had all its instances inlined

## Loop Unrolling
We can save on overhead by reducing the number of iterations performed in `for` loops
- `compress_data()` now operates in blocks of 8 samples with a switch case handling the modulo remainder
- `compress_data()` now uses a single-statement `while` loop instead of a three-statement `for` loop
- `decompress_data()` now operates in blocks of 8 samples with a switch case handling the modulo remainder
- `decompress_data()` now uses a single-statement `while` loop instead of a three-statement `for` loop

## Print Statements and Timers
Calls to `printf()` and debug timers are slow and unnecessary in an embedded system
- All non-error `printf()` calls have been removed
