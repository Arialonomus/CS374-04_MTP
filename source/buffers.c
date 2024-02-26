#include "buffers.h"

struct sharedbuffer shared[NUM_BUFS];

int check_barrier_pos(struct sharedbuffer* buffer, size_t* destination)
{
	// Attempt to read the barrier position
	if (pthread_mutex_trylock(&buffer->mutex) == 0) {
		*destination = buffer->barrier;
		int status = pthread_cond_broadcast(&buffer->condition);
		if(status != 0) pthread_err(status, "pthread_cond_broadcast");
		status = pthread_mutex_unlock(&buffer->mutex);
		if(status != 0) pthread_err(status, "pthread_mutex_unlock");
		return 0;
	}

	return -1;
}

int set_barrier_pos(struct sharedbuffer* buffer, const size_t index, const enum wait_flag if_locked)
{
	int status = 0;		// Holds the error status for pthread functions

	// Attempt to lock the mutex
	if (if_locked == WAIT) {
		status = pthread_mutex_lock(&buffer->mutex);
		if(status != 0) pthread_err(status, "pthread_mutex_lock");
	}
	else if (pthread_mutex_trylock(&buffer->mutex) != 0) {
		return -1;
	}

	// Update the barrier index and return
	buffer->barrier = index;
	status = pthread_cond_broadcast(&buffer->condition);
	if(status != 0) pthread_err(status, "pthread_cond_broadcast");
	status = pthread_mutex_unlock(&buffer->mutex);
	if(status != 0) pthread_err(status, "pthread_mutex_unlock");
	return 0;
}

void hold(struct sharedbuffer* buffer, const size_t current_pos, size_t* cached_barrier)
{
	int status = 0;		// Holds the error status for pthread functions

	// Poll the shared barrier until it has updated
	status = pthread_mutex_lock(&buffer->mutex);
	if(status != 0) pthread_err(status, "pthread_mutex_lock");
	*cached_barrier = buffer->barrier;
	while (current_pos == *cached_barrier) {
		status = pthread_cond_wait(&buffer->condition, &buffer->mutex);
		if(status != 0) pthread_err(status, "pthread_cond_wait");
		*cached_barrier = buffer->barrier;
	}

	// Release the mutex and return
	status = pthread_cond_broadcast(&buffer->condition);
	if(status != 0) pthread_err(status, "pthread_cond_broadcast");
	status = pthread_mutex_unlock(&buffer->mutex);
	if(status != 0) pthread_err(status, "pthread_mutex_unlock");
}

void pthread_err(const int status, char* message)
{
	errno = status;
	err(1, "%s", message);
}