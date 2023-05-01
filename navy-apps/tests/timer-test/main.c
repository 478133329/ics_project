#include <stdio.h>
#include <sys/time.h>

int main() {
	struct timeval tv = {};
	gettimeofday(&tv, NULL);
	time_t old = tv.tv_usec;
	while (1) {
		gettimeofday(&tv, NULL);
		if (tv.tv_usec - old > 500 * 1000) {
			old = tv.tv_usec;
			printf("hello timer.\n");
		}
	}
	return 0;
}
