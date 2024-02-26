#include "processing.h"

void* get_input(void* arg)
{
	// Initialize iterator
	struct sharedbuffer* output_buf = &shared[INPUT];
	size_t out_current = 0;

	enum status_t status = INPROGRESS;
	for(;;) {
		while(status == INPROGRESS && out_current < BUF_SIZE - 1) {
			// Read a line from input
			char* line = output_buf->buffer;
			if (!fgets(line, BUF_SIZE - out_current, stdin) && ferror(stdin))
				err (1, "fgets");
			const size_t len = strlen(line);
			if (feof(stdin)) {
				line[len] = 3;
				status = FINISHED;
			}
			if (strcmp(line, "STOP\n") == 0) {
				line[len] = 4;
				status = STOPPED;
			}
			out_current += len;

			// Attempt to set the barrier for downstream thread
			set_barrier_pos(output_buf, out_current, CONTINUE);
		}
		// Update the write barrier for downstream thread
		set_barrier_pos(output_buf, out_current, WAIT);

		// Wait until downstream output is completed before next line read
		if (status == INPROGRESS) {
			struct sharedbuffer* downstream = &shared[PROCESSING];
			pthread_mutex_lock(&downstream->mutex);
			size_t ds_pos = downstream->barrier;
			while (ds_pos < BUF_SIZE) {
				pthread_cond_wait(&downstream->condition, &downstream->mutex);
				ds_pos = downstream->barrier;
			}
			pthread_mutex_unlock(&downstream->mutex);
			pthread_cond_broadcast(&downstream->condition);
			out_current = 0;
		}
		return (void*)status;
	}
}

void* convert_newline(void* arg)
{
	// Initialize input iterator
	struct sharedbuffer* input_buf = &shared[INPUT];
	size_t in_current = 0;
	size_t in_end = 0;

	// Initialize output iterator
	struct sharedbuffer* output_buf = &shared[PROCESSING];
	size_t out_current = 0;

	enum status_t status = INPROGRESS;
	for(;;) {
		while(status == INPROGRESS && in_current < in_end) {
			// Read a char from the shared buffer
			int c = input_buf->buffer[in_current];
			++in_current;

			// Process character and place it in output buffer
			if (c == '\n') c = ' ';
			output_buf->buffer[out_current] = (char)c;
			++out_current;

			// Update output barrier if not locked
			set_barrier_pos(output_buf, out_current, CONTINUE);

			// Get current input barrier if not locked
			check_barrier_pos(input_buf, &in_end);

			// Check for terminating characters
			switch (c) {
				case 3:
					status = FINISHED; break;
				case 4:
					status = STOPPED; break;
				default:
					break;
			}
		}
		// Update the write barrier for downstream thread
		set_barrier_pos(output_buf, out_current, WAIT);

		// Wait for upstream thread to put in more data
		if (status == INPROGRESS)
			hold(input_buf, in_current, &in_end);
		else return (void*)status;
	}
}

void* convert_doubleplus(void* arg)
{
	// Initialize input iterator
	struct sharedbuffer* input_buf = &shared[PROCESSING];
	size_t in_current = 0;
	size_t in_end = 0;

	// Initialize output iterator
	struct sharedbuffer* output_buf = &shared[OUTPUT];
	size_t out_current = 0;

	enum status_t status = INPROGRESS;
	for(;;) {
		while(status == INPROGRESS && in_current < in_end) {
			// Read a char from the shared buffer
			int c = input_buf->buffer[in_current];
			++in_current;

			// Convert "++" to '^'
			if (c == '+') {
				if (in_current == in_end)
					hold(input_buf, in_current, &in_end);
				const int in_next = in_current + 1;
				const int c_next = input_buf->buffer[in_current];
				if (c_next == '+') {
					c = '^';
					in_current = in_next;
				}
			}

			// Place character in output buffer
			output_buf->buffer[out_current] = (char)c;
			++out_current;

			// Update output barrier if not locked
			set_barrier_pos(output_buf, out_current, CONTINUE);

			// Get current input barrier if not locked
			check_barrier_pos(input_buf, &in_end);

			// Check for terminating characters
			switch (c) {
				case 3:
					status = FINISHED; break;
				case 4:
					status = STOPPED; break;
				default:
					break;
			}
		}
		// Update the write barrier for downstream thread
		set_barrier_pos(output_buf, out_current, WAIT);

		// Wait for upstream thread to put in more data
		if (status == INPROGRESS)
			hold(input_buf, in_current, &in_end);
		else return (void*)status;
	}
}

void* print_output(void* arg)
{
	// Initialize input iterator
	struct sharedbuffer* input_buf = &shared[OUTPUT];
	size_t in_current = 0;
	size_t in_end = 0;

	enum status_t status = INPROGRESS;
	for(;;) {
		unsigned int num_printed = 0;	// Holds the number of characters printed in the current line

		while(status == INPROGRESS && in_current != in_end) {
			// Read a char from the shared buffer
			int c = input_buf->buffer[in_current];
			++in_current;

			// Get current input barrier if not locked
			check_barrier_pos(input_buf, &in_end);

			// Check for terminating characters
			switch (c) {
				case 3:
					status = FINISHED;
					c = '\n'; break;
				case 4:
					status = STOPPED;
					c = '\n'; break;
				default:
					break;
			}

			// Print the character to stdout
			putchar(c);
			++num_printed;

			// Wrap line if line line limit has been reached
			if (num_printed == MAX_LINE_LEN && status == INPROGRESS) {
				putchar('\n');
				num_printed = 0;
			}
		}
		// Update the barrier for upstream thread and return
		set_barrier_pos(input_buf, in_current, WAIT);

		// Wait for upstream thread to put in more data
		if (status == INPROGRESS)
			hold(input_buf, in_current, &in_end);
		else return (void*)status;
	}
}