#include <sys/time.h>
#include <stdio.h>
extern uint32_t NDL_GetTicks();
int main() {
	struct timeval temp;
	gettimeofday(&temp, NULL);
	int sec = 500;
	while (1) {
		while(NDL_GetTicks() < sec) {
			gettimeofday(&temp, NULL);
		}
		sec += 500;
		printf("times %d \n", sec / 500);
	}
	return 0;
}
