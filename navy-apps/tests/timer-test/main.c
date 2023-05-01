#include <stdio.h>
// #include <sys/time.h>

#include <NDL.h>

int main() {
	// struct timeval tv = {};
	// gettimeofday(&tv, NULL);
	// time_t old = tv.tv_usec;
	NDL_Init(0);
	uint32_t old = NDL_GetTicks();
	while (1) {
		// gettimeofday(&tv, NULL);
		uint32_t new = NDL_GetTicks();
		if (new - old > 500) {
			old = new;
			printf("hello timer.\n");
		}
	}
	NDL_Quit();
	return 0;
}
