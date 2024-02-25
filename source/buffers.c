#include "buffers.h"

struct sharedbuffer shared[NUM_BUFS];

void get_rbarrier(struct sharedbuffer* buffer, size_t* destination)
{
	const int status = get_barrier(buffer, READ);
	if (status != -1) *destination = status;
}

int set_rbarrier(struct sharedbuffer* buffer, const size_t index)
{
	return set_barrier(buffer, index, READ);
}

void get_wbarrier(struct sharedbuffer* buffer, size_t* destination)
{
	const int status = get_barrier(buffer, WRITE);
	if (status != -1) *destination = status;
}

int set_wbarrier(struct sharedbuffer* buffer, const size_t index)
{
	return set_barrier(buffer, index, WRITE);
}

int get_barrier(struct sharedbuffer* buffer, const enum barrier_flag flag)
{
	// Determine which barrier to read
	const size_t* barrier = flag == READ ? &buffer->read_barrier : &buffer->write_barrier;

	// Attempt to update barrier
	int retval = -1;
	if (pthread_mutex_trylock(&buffer->mutex) == 0) {
		retval = *barrier;
		int status = pthread_mutex_unlock(&buffer->mutex);
		if(status != 0) pthread_err(status, "pthread_mutex_unlock");
	}

	return retval;
}

int set_barrier(struct sharedbuffer* buffer, const size_t index, const enum barrier_flag flag)
{
	int result = -1;	// Flag for if the barrier was successfully changed

	// Determine which barrier to update
	size_t* barrier = flag == READ ? &buffer->read_barrier : &buffer->write_barrier;

	// Attempt to update barrier
	if (pthread_mutex_trylock(&buffer->mutex) == 0) {
		*barrier = index;
		int status = pthread_mutex_unlock(&buffer->mutex);
		if(status != 0) pthread_err(status, "pthread_mutex_unlock");
		result = 0;
	}

	return result;
}

void pthread_err(const int status, char* message)
{
	errno = status;
	err(1, "%s", message);
}