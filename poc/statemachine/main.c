#include <stdio.h>
#include <stdint.h>

typedef uint8_t (*Color_fn)(uint8_t counter);

typedef struct Color_t {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} Color;

// TODO: Find out a way to do exit conditions
typedef struct State_t {
	uint8_t counter;
	Color_fn red;
	Color_fn green;
	Color_fn blue;
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

// TODO: Add exit condition processing
void nextFrame(State *st) {
	st->counter++;
}

Color getColor(State *st) {
	uint8_t red = st->red(st->counter);
	uint8_t green = st->green(st->counter);
	uint8_t blue = st->blue(st->counter);
	Color col = {red, green, blue};
	return col;
}

int main(int arc, char **argv)
{
	printf("Entering main\n");

	int i;
	for(i = 0; i < 10; i++) {
		Color col = getColor(&curState);
		printf("counter: %u\tred: %u\t green: %u\tblue: %u\n", curState.counter, col.red, col.green, col.blue);
		nextFrame(&curState);
	}

	return 0;
}
