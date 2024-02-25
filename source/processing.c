#include "processing.h"

void* get_input(void* arg)
{
	// Initialize iterator
	struct sharedbuffer* shr_buf = &shared[INPUT];
	enum status_t status = INPROGRESS;
	size_t end = BUF_SIZE - 1;
	size_t current = 0;

	while(!feof(stdin)) {
		while(current != end && !feof(stdin)) {
			// Read a char from input and write it to the buffer
			int c = getchar();
			if (ferror(stdin)) err (1, "getchar");
			if (c == EOF) c = 3;
			shr_buf->buffer[current] = (char)c;
			if (++current == BUF_SIZE) current = 0;

			// Check for STOP sequence after newline
			if (c == '\n' && check_stop() == true) {
				c = 3;
				shr_buf->buffer[current] = (char)c;
				status = STOPPED;
				return (void*)status;
			}

			// Update the write barrier if not locked
			set_wbarrier(shr_buf, current);
		}
		// Get new end position if not locked
		get_rbarrier(shr_buf, &end);
	}

	// Set status and return
	status = FINISHED;
	return (void*)status;
}

bool check_stop()
{
	// Read the next 5 characters from stdin
	char sequence[6] = {0};
	fgets(sequence, 6, stdin);
	if (ferror(stdin)) err (1, "fgets");
	if(strcmp(sequence, "STOP\n") == 0) return true;

	// Replace the characters to stdin in reverse order
	for (int i = strlen(sequence) - 1; i >= 0; --i)
		ungetc(sequence[i], stdin);
	return false;
}