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

enum {
	THRASH_RO,
	THRASH_WO,
	THRASH_RW
};

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

void thrash_ro() {
	off_t index;
	assert(BUFF_SIZE);
	size_t buff_len = BUFF_SIZE / sizeof(NATIVE_WIDTH_TYPE);
	// Ensure BUFF_SIZE is divisible by sizeof(NATIVE_WIDTH_TYPE)
	assert(buff_len * sizeof(NATIVE_WIDTH_TYPE) == BUFF_SIZE);

	NATIVE_WIDTH_TYPE *buff = malloc(BUFF_SIZE);
	assert(buff);

	while(run) {
		target = buff[index++];
		(void)target;
		if(index >= buff_len) {
			index = 0;
		}
		cnt++;
	}

	free(buff);
}

void thrash_wo() {
	off_t index;
	assert(BUFF_SIZE);
	size_t buff_len = BUFF_SIZE / sizeof(NATIVE_WIDTH_TYPE);
	// Ensure BUFF_SIZE is divisible by sizeof(NATIVE_WIDTH_TYPE)
	assert(buff_len * sizeof(NATIVE_WIDTH_TYPE) == BUFF_SIZE);

	NATIVE_WIDTH_TYPE *buff = malloc(BUFF_SIZE);
	assert(buff);

	while(run) {
		buff[index++] = target;
		if(index >= buff_len) {
			index = 0;
		}
		cnt++;
	}

	free(buff);
}

void thrash_rw() {
	assert(BUFF_SIZE);
	size_t buff_len = BUFF_SIZE / sizeof(NATIVE_WIDTH_TYPE);
	// Ensure BUFF_SIZE is divisible by sizeof(NATIVE_WIDTH_TYPE)
	assert(buff_len * sizeof(NATIVE_WIDTH_TYPE) == BUFF_SIZE);

	NATIVE_WIDTH_TYPE *buff_r = malloc(BUFF_SIZE);
	assert(buff_r);
	NATIVE_WIDTH_TYPE *buff_w = malloc(BUFF_SIZE);
	assert(buff_w);

	while(run) {
		assert(memcpy(buff_w, buff_r, BUFF_SIZE) == buff_w);
		cnt += buff_len;
	}

	free(buff_w);
	free(buff_r);
}

int main(int argc, char** argv) {
	int mode = THRASH_RW;
	if(argc > 1) {
		assert(argc == 2);
		if(!strcmp("rw", argv[1])) {
//			Everything ok, default to mode = THRASH_RW;
		} else if(strchr(argv[1], 'r') == argv[1]) {
			mode = THRASH_RO;
		} else {
			assert(strchr(argv[1], 'w') == argv[1]);
			mode = THRASH_WO;
		}
	}

	assert(!signal(SIGINT, handle_sigint));
	assert(!signal(SIGALRM, handle_sigalrm));

	printf("PID %zu\n", getpid());

	alarm(1);

	switch(mode) {
		case THRASH_RO: printf("Performing RO test\n"); thrash_ro(); break;
		case THRASH_WO: printf("Performing WO test\n"); thrash_wo(); break;
		case THRASH_RW: printf("Performing RW test\n"); thrash_rw(); break;
	}

	printf("\n");

	return 0;
}
