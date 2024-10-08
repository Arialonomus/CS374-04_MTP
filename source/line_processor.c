/* Program: CS 374 Assignment 04 - MTP
* Author: Jacob Barber
 * UID: 934561945
 *
 * This program performs simple multi-threaded line processing to demonstrate understanding
 * of basic concurrent programming concepts.
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <err.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "processing.h"

int main(int argc, char *argv[])
{
	// Check for valid argument count
	if (argc > 1) errx(1, "too many arguments");

	// Initialize shared buffers
	for (int i = 0; i < NUM_BUFS; ++i) {
		pthread_mutex_init(&shared[i].mutex, NULL);
		pthread_cond_init(&shared[i].condition, NULL);
	}

	// Create process threads
	pthread_t input_reader;
	pthread_create(&input_reader, NULL, get_input, NULL);

	pthread_t newline_converter;
	pthread_create(&newline_converter, NULL, convert_newline, NULL);

	pthread_t plus_converter;
	pthread_create(&plus_converter, NULL, convert_doubleplus, NULL);

	pthread_t output_printer;
	pthread_create(&output_printer, NULL, print_output, NULL);

	// Join threads after processing is complete
	pthread_join(input_reader, NULL);
	pthread_join(newline_converter, NULL);
	pthread_join(plus_converter, NULL);
	pthread_join(output_printer, NULL);

	// Cleanup shared buffers
	for (int i = 0; i < NUM_BUFS; ++i) {
		pthread_mutex_destroy(&shared[i].mutex);
		pthread_cond_destroy(&shared[i].condition);
	}

	return EXIT_SUCCESS;
}