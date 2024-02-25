#include "processing.h"

void* get_input(void* arg)
{
	// Initialize iterator
	struct sharedbuffer* shr_buf = &shared[INPUT];
	enum status_t status = INPROGRESS;
	size_t current = 0;
	size_t end = BUF_SIZE - 1;
	int setw_status = -1;

	while(status == INPROGRESS) {
		while(current != end && !feof(stdin)) {
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
			setw_status = set_wbarrier(shr_buf, current);

			// Check for STOP sequence after newline
			if (c == '\n' && check_stop() == true) {
				c = 4;
				shr_buf->buffer[current] = (char)c;
				status = STOPPED;
				while (setw_status != 0)
					setw_status = set_wbarrier(shr_buf, current);
				return (void*)status;
			}
		}
		// Get new end position if not locked
		get_rbarrier(shr_buf, &end);
	}

	// Set status and return
	status = FINISHED;
	while (setw_status != 0)
		setw_status = set_wbarrier(shr_buf, current);
	return (void*)status;
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