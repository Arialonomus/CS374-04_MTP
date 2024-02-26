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

// Flag to instruct operation to wait or continue if the mutex is locked
enum wait_flag
{
	CONTINUE,
	WAIT
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
extern struct sharedbuffer shared[];

/***********
 * Functions
 ***********/

// Attempts to store the index of the requested barrier in a shared buffer in the destination, returns 0 on success
int check_barrier_pos(struct sharedbuffer* buffer, const enum barrier_flag flag, size_t* destination);

// Sets a barrier, to a desired index, returns 0 on success. Will block if WAIT flag is set
int set_barrier_pos(struct sharedbuffer* buffer, const enum barrier_flag flag, size_t index, enum wait_flag if_locked);

// Hold until the status of a barrier has changed
void hold(struct sharedbuffer* buffer, const enum barrier_flag flag, size_t current_pos, size_t* cached_barrier);

// Wrapper function for err() that sets errno to the number returned from a failed pthread function
void pthread_err(int status, char* message);

#endif //MTP_BUFFERS_H