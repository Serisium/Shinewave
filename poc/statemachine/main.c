#include <stdio.h>
#include <stdint.h>


typedef uint8_t (*Color)(uint8_t counter);

typedef struct State_t {
	uint8_t counter;
	Color red;
	Color green;
	Color blue;
} State;

// Simple color ramp
uint8_t linear(uint8_t counter) {
	return counter;
}

uint8_t zero(uint8_t counter) {
	return 0;
}

// Start on a simple purple and ramp up with counter
State curState = {0, linear, zero, linear};

void nextFrame(State *st) {
	st->counter++;
}


int main(int arc, char **argv)
{
	printf("Entering main\n");

	int i;
	for(i = 0; i < 10; i++) {
		nextFrame(&curState);
		printf("counter: %u\tred: %u\t green: %u\tblue: %u\n", curState.counter, curState.red(curState.counter), curState.blue(curState.counter), curState.green(curState.counter));
	}

	return 0;
}
