#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>

#define GB(n) (MB((n)) * 1024)
#define MB(n) ((n) * 1024 * 1024)

#define NATIVE_WIDTH_TYPE uint64_t
#define BUFF_SIZE MB(256)

volatile bool run = true;

volatile unsigned long long last = 0, cnt = 0;

volatile NATIVE_WIDTH_TYPE target;

void handle_sigint(int signal) {
	run = false;
}

void handle_sigalrm(int signal) {
	assert(!alarm(1));
	printf("%llu MB per second", (cnt - last) * sizeof(NATIVE_WIDTH_TYPE) / MB(1));
	fflush(stdout);
	printf("\r");
	last = cnt;
}

int main(int argc, char** argv) {
	off_t index;
	assert(BUFF_SIZE);
	size_t buff_len = BUFF_SIZE / sizeof(NATIVE_WIDTH_TYPE);
	// Ensure BUFF_SIZE is divisible by sizeof(NATIVE_WIDTH_TYPE)
	assert(buff_len * sizeof(NATIVE_WIDTH_TYPE) == BUFF_SIZE);

	NATIVE_WIDTH_TYPE *buff = malloc(BUFF_SIZE);
	assert(buff);

	assert(!signal(SIGINT, handle_sigint));
	assert(!signal(SIGALRM, handle_sigalrm));

	printf("PID %zu\n", getpid());

	alarm(1);

	while(run) {
		target = buff[index++];
		(void)target;
		if(index >= buff_len) {
			index = 0;
		}
		cnt++;
	}

	free(buff);
	return 0;
}
