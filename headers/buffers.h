/* Program: CS 374 Assignment 04 - MTP
 *
 * This file contains global variables and functions related to data
 * shared by multiple threads
 */

#ifndef MTP_BUFFERS_H
#define MTP_BUFFERS_H

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>

#ifndef MAX_LINE_LEN
#define MAX_LINE_LEN 80
#endif

#ifndef BUF_SIZE
#define BUF_SIZE MAX_LINE_LEN + 2
#endif

#ifndef NUM_BUFS
#define NUM_BUFS 3
#endif

// Holds named indices for referencing shared buffer structures
enum buf_name
{
	INPUT,
	PROCESSING,
	OUTPUT
};

// Flag for selecting which barrier to set or return
enum barrier_flag
{
	READ,
	WRITE
};

// A shared buffer for consumer-server threads, with an associated mutex and condition variable
struct sharedbuffer
{
	char buffer[BUF_SIZE];
	size_t write_barrier;		// The index position after the last character written
	size_t read_barrier;		// The index position of the first unread character
	pthread_mutex_t mutex;
	pthread_cond_t condition;
};

// A global array of shared buffers
struct sharedbuffer shared[NUM_BUFS];

/***********
 * Functions
 ***********/

// WRAPPER: Stores the current read barrier position of the passed in buffer in destination
void get_rbarrier(struct sharedbuffer* buffer, size_t* destination);

// WRAPPER: Sets the current position of the read barrier in a shared buffer
void set_rbarrier(struct sharedbuffer* buffer, size_t index);

// WRAPPER: Stores the current write barrier position of the passed in buffer in destination
void get_wbarrier(struct sharedbuffer* buffer, size_t* destination);

// WRAPPER: Sets the current position of the write barrier in a shared buffer
void set_wbarrier(struct sharedbuffer* buffer, size_t index);

// HELPER FUNCTION: Returns the index of the requested barrier in a shared buffer
int get_barrier(struct sharedbuffer* buffer, enum barrier_flag flag);

// HELPER FUNCTION: Attempts to set the index of the requested in a shared buffer
void set_barrier(struct sharedbuffer* buffer, size_t index, enum barrier_flag flag);

// Wrapper function for err() that sets errno to the number returned from a failed pthread function
void pthread_err(int status, char* message);

#endif //MTP_BUFFERS_H
