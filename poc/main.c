#include <stdio.h>
#include <stdint.h>
#include "pt-1.4/pt.h"
#include "controller.h"

#define DELAY 17	// delay in ms, 60fps

// Example code
uint8_t controller_data[11] = {0};
static Controller *controller;

typedef struct {
	struct pt pt;
	int count;
} blinkState;

static PT_THREAD(blink(blinkState *s))
{
	PT_BEGIN(&s->pt);

	printf("Blink entered!\n");

	PT_WAIT_UNTIL(&s->pt, CONTROLLER_B(*controller));

	for(s->count = 0; s->count < 20; s->count++) {
		printf("Frame %i entered\n", s->count);
		PT_YIELD(&s->pt);
	}

	PT_END(&s->pt);
}

int main(void)
{
	blinkState thisState;
	thisState.count = 0;

	PT_INIT(&thisState.pt);

	printf("\e[1;1H\e[2J");
	printf("Hello world!\n");

	while(1) {
		controller_data[5] = 0x04;		// Press B
		controller = (Controller*)controller_data;
		blink(&thisState);

		usleep(DELAY * 1000);
	}

	return 0;
}
