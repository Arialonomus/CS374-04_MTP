#include "processing.h"

void* get_input(void* arg)
{
	// Initialize iterator
	struct sharedbuffer* shr_buf = &shared[INPUT];
	size_t current = 0;
	size_t end = BUF_SIZE - 1;

	enum status_t status = INPROGRESS;
	for(;;) {
		while(status == INPROGRESS && current != end) {
			// Read a char from input and write it to the buffer
			int c = getchar();
			if (ferror(stdin)) err (1, "getchar");
			if (c == EOF) {
				c = 3;
				status = FINISHED;
			}
			shr_buf->buffer[current] = (char)c;
			if (++current == BUF_SIZE) current = 0;

			// Update the write barrier if not locked
			set_barrier_pos(shr_buf, WRITE, current, CONTINUE);

			// Check for STOP sequence after newline
			if (c == '\n' && check_stop() == true) {
				c = 4;
				shr_buf->buffer[current] = (char)c;
				status = STOPPED;
			}
		}
		// Update the write barrier for downstream thread
		set_barrier_pos(shr_buf, WRITE, current, WAIT);

		// Wait for downstream thread to process more data
		if (current == end && status == INPROGRESS)
			hold(shr_buf, READ, current, &end);
		else return (void*)status;
	}
}

bool check_stop()
{
	// Read the next 5 characters from stdin
	char sequence[6] = {0};
	fgets(sequence, 6, stdin);
	if (ferror(stdin)) err (1, "fgets");

	// Check if line is the stop sequence
	if(strcmp(sequence, "STOP\n") == 0) return true;

	// Replace the characters to stdin in reverse order
	for (int i = strlen(sequence) - 1; i >= 0; --i)
		ungetc(sequence[i], stdin);
	return false;
}

void* ls_converter(void* arg)
{
	// Initialize input iterator
	struct sharedbuffer* input_buf = &shared[INPUT];
	size_t in_current = 0;
	size_t in_end = 0;

	// Initialize output iterator
	struct sharedbuffer* output_buf = &shared[PROCESSING];
	size_t out_current = 0;
	size_t out_end = BUF_SIZE - 1;

	enum status_t status = INPROGRESS;
	for(;;) {
		while(status == INPROGRESS && in_current != in_end && out_current != out_end) {
			// Read a char from the shared buffer
			int c = input_buf->buffer[in_current];
			if (++in_current == BUF_SIZE) in_current = 0;

			// Update the input buffer's barriers if not locked
			set_barrier_pos(input_buf, READ, in_current, CONTINUE);
			check_barrier_pos(input_buf, WRITE, &in_end);

			// Process character and place it in output buffer
			if (c == '\n') c = ' ';
			output_buf->buffer[out_current] = (char)c;
			if (++out_current == BUF_SIZE) out_current = 0;

			// Update output buffer's barriers if not locked
			set_barrier_pos(output_buf, WRITE, out_current, CONTINUE);
			check_barrier_pos(output_buf, READ, &out_end);

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
		// Update the read barrier for upstream thread
		set_barrier_pos(input_buf, READ, in_current, WAIT);

		// Update the write barrier for downstream thread
		set_barrier_pos(output_buf, WRITE, out_current, WAIT);

		if (status == INPROGRESS) {
			// Wait for upstream thread to put in more data
			if (in_current == in_end)
				hold(input_buf, WRITE, in_current, &in_end);

			// Wait for downstream thread to process more data
			if (out_current == out_end)
				hold(output_buf, READ, out_current, &out_end);
		}
		else return (void*)status;
	}
}

void* plus_converter(void* arg)
{
	// Initialize input iterator
	struct sharedbuffer* input_buf = &shared[PROCESSING];
	size_t in_current = 0;
	size_t in_end = 0;

	// Initialize output iterator
	struct sharedbuffer* output_buf = &shared[OUTPUT];
	size_t out_current = 0;
	size_t out_end = BUF_SIZE - 1;

	enum status_t status = INPROGRESS;
	for(;;) {
		while(status == INPROGRESS && in_current != in_end && out_current != out_end) {
			// Read a char from the shared buffer
			int c = input_buf->buffer[in_current];
			if (++in_current == BUF_SIZE) in_current = 0;

			// Update the input buffer's barriers if not locked
			set_barrier_pos(input_buf, READ, in_current, CONTINUE);
			check_barrier_pos(input_buf, WRITE, &in_end);

			// Convert "++" to '^' and place in ouput buffer
			if (c == '+') {
				if (in_current == in_end)
					hold(input_buf, WRITE, in_current, &in_end);
				int in_next = in_current + 1;
				if (in_next == BUF_SIZE) in_next = 0;
				const int c_next = input_buf->buffer[in_current];
				if (c_next == '+') {
					c = '^';
					in_current = in_next;
				}
			}
			output_buf->buffer[out_current] = (char)c;
			if (++out_current == BUF_SIZE) out_current = 0;

			// Update output buffer's barriers if not locked
			set_barrier_pos(output_buf, WRITE, out_current, CONTINUE);
			check_barrier_pos(output_buf, READ, &out_end);

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
		// Update the read barrier for upstream thread
		set_barrier_pos(input_buf, READ, in_current, WAIT);

		// Update the write barrier for downstream thread
		set_barrier_pos(output_buf, WRITE, out_current, WAIT);

		if (status == INPROGRESS) {
			// Wait for upstream thread to put in more data
			if (in_current == in_end)
				hold(input_buf, WRITE, in_current, &in_end);

			// Wait for downstream thread to process more data
			if (out_current == out_end)
				hold(output_buf, READ, out_current, &out_end);
		}
		else return (void*)status;
	}
}