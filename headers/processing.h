/* Program: CS 374 Assignment 04 - MTP
 *
 * This file contains functions related to reading and processing a line of input
 */

#ifndef MTP_PROCESSING_H
#define MTP_PROCESSING_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "buffers.h"

enum status_t
{
 INPROGRESS,
 STOPPED,
 FINISHED,
 ERROR
};

// Reads characters from stdin and stores them in input_buf
// Terminates input if the stop processing sequence "STOP\n" is received
void* get_input(void* arg);

// Returns true if the next line of input is "STOP\n"
bool check_stop(size_t* current_pos);

// Converts all line separator characters from input into spaces
void* convert_newline(void* arg);

// Converts all instances of "++" to "^"
void* convert_doubleplus(void* arg);

// Prints processed output to stdout in lines of 80 characters
void* print_output(void* arg);

#endif //MTP_PROCESSING_H