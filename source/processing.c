#include "processing.h"

void* get_input(void* arg)
{
	// Initialize iterator
	struct sharedbuffer* output_buf = &shared[INPUT];
	size_t out_current = 0;
	const size_t out_end = BUF_SIZE - 1;

	enum status_t status = INPROGRESS;
	for(;;) {
		while(status == INPROGRESS && out_current < out_end) {
			// Read a char from input and write it to the buffer
			int c = getchar();
			if (ferror(stdin)) err (1, "getchar");
			if (c == EOF) {
				c = 3;
				status = FINISHED;
			}
			output_buf->buffer[out_current] = (char)c;
			++out_current;

			// Update the barrier if not locked
			set_barrier_pos(output_buf, out_current, CONTINUE);

			// Check for STOP sequence after newline
			if (c == '\n' && check_stop() == true) {
				c = 4;
				output_buf->buffer[out_current] = (char)c;
				status = STOPPED;
			}
		}
		// Update the barrier for downstream thread and return
		set_barrier_pos(output_buf, out_current, WAIT);
		return (void*)status;
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

void* convert_newline(void* arg)
{
	// Initialize input iterator
	struct sharedbuffer* input_buf = &shared[INPUT];
	size_t in_current = 0;
	size_t in_end = 0;

	// Initialize output iterator
	struct sharedbuffer* output_buf = &shared[PROCESSING];
	size_t out_current = 1;
	size_t out_end = BUF_SIZE - 1;

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
	size_t out_current = 1;
	size_t out_end = BUF_SIZE - 1;

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
			if (++in_current == BUF_SIZE) in_current = 0;

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