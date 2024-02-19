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
#include <bits/pthreadtypes.h>

#include "processing.h"

#define MAX_LINE_LEN 80
#define BUF_SIZE MAX_LINE_LEN + 2

int main(int argc, char *argv[])
{
 /* Open file for reading */
 if (argc != 2) errx(1, "program takes a single filepath argument");
 char* input_filepath = argv[1];
 if(!freopen(input_filepath, "re", stdin)) err(1, "fopen(): %s", input_filepath);

 /* Initialize buffers & their associated mutexes */
 char input_buf[BUF_SIZE] = {0};
 size_t input_index = 0;
 pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;
 pthread_cond_t input_sig = PTHREAD_COND_INITIALIZER;

 char process_buf[BUF_SIZE] = {0};
 size_t process_index = 0;
 pthread_mutex_t process_mutex = PTHREAD_MUTEX_INITIALIZER;
 pthread_cond_t process_sig = PTHREAD_COND_INITIALIZER;

 char output_buf[BUF_SIZE] = " ";
 size_t output_index = 0;
 pthread_mutex_t outputm_mutex = PTHREAD_MUTEX_INITIALIZER;
 pthread_cond_t output_sig = PTHREAD_COND_INITIALIZER;


}