#include "buffers.h"

void get_rbarrier(struct sharedbuffer* buffer, size_t* destination)
{
	int status = get_barrier(buffer, READ);
	if (status != -1) *destination = status;
}

void set_rbarrier(struct sharedbuffer* buffer, const size_t index)
{
	set_barrier(buffer, index, READ);
}

void get_wbarrier(struct sharedbuffer* buffer, size_t* destination)
{
	int status = get_barrier(buffer, WRITE);
	if (status != -1) *destination = status;
}

void set_wbarrier(struct sharedbuffer* buffer, const size_t index)
{
	set_barrier(buffer, index, WRITE);
}

int get_barrier(struct sharedbuffer* buffer, enum barrier_flag flag)
{
	// Determine which barrier to update
	size_t* barrier = (flag == READ) ? &buffer->read_barrier : &buffer->write_barrier;

	// Attempt to update barrier
	int retval = -1;
	if (pthread_mutex_trylock(&buffer->mutex) == 0) {
		retval = *barrier;
		int status = pthread_mutex_unlock(&buffer->mutex);
		if(status != 0) pthread_err(status, "pthread_mutex_unlock");
		status = pthread_cond_signal(&buffer->condition);
		if(status != 0) pthread_err(status, "pthread_cond_broadcast");
	}

	return retval;
}

void set_barrier(struct sharedbuffer* buffer, const size_t index, const enum barrier_flag flag)
{
	// Determine which barrier to update
	size_t* barrier = (flag == READ) ? &buffer->read_barrier : &buffer->write_barrier;

	// Attempt to update barrier
	if (pthread_mutex_trylock(&buffer->mutex) == 0) {
		*barrier = index;
		int status = pthread_mutex_unlock(&buffer->mutex);
		if(status != 0) pthread_err(status, "pthread_mutex_unlock");
		status = pthread_cond_signal(&buffer->condition);
		if(status != 0) pthread_err(status, "pthread_cond_broadcast");
	}
}

void pthread_err(const int status, char* message)
{
	errno = status;
	err(1, "%s", message);
}