#include <stdio.h>
#include <sys/time.h>

int main() {
	struct timeval tv = {};
	int ret = gettimeofday(&tv, NULL);
	printf("tv.tv_sec: %d\n", tv.tv_sec);
	return 0;
}
