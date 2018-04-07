#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>

volatile bool run = true;

volatile unsigned long long last = 0, cnt = 0;

void handle_sigint(int signal) {
	run = false;
}

void handle_sigalrm(int signal) {
	assert(!alarm(1));
	printf("%llu k spins per second\n", (cnt - last) / 1000);
	last = cnt;
}

int main(int argc, char** argv) {
	assert(!signal(SIGINT, handle_sigint));
	assert(!signal(SIGALRM, handle_sigalrm));
	alarm(1);

	while(run) {
		cnt++;
	}

	return 0;
}
