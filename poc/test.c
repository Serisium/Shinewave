#include <stdio.h>
#include <stdint.h>
#include "pt-1.4/pt.h"

typedef struct {
	struct pt pt;
	int counter;
} state;

static int counter1 = 60;

static PT_THREAD(example1(state *s))
{
	PT_BEGIN(&s->pt);

	while(1) {
		PT_WAIT_UNTIL(&s->pt, s->counter == 60);
		printf("Threshold1 reached!\n");
		s->counter = 0;
	}

	PT_END(&s->pt);
}

int main(void)
{
	state thisState;
	thisState.counter = 0;

	PT_INIT(&thisState.pt);

	printf("Hello world!\n");

	while(1) {
		example1(&thisState);
		thisState.counter++;

		usleep(17 * 1000);
	}

	return 0;
}
