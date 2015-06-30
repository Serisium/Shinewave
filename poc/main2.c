#include <stdio.h>
#include <stdint.h>

#define DELAY 17	// delay in ms, 60fps

int main(void)
{
	printf("\e[1;1H\e[2J");		// Clear console

	uint8_t i = 0;

	while(1) {
		printf("%i\n", i);
		if(!i--) {
			printf("Zero\n");
		}
		usleep(DELAY * 1000);
	}

	return 0;
}
