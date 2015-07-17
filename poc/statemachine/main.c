#include <stdio.h>
#include <stdint.h>

typedef struct Color Color;
typedef struct State State;

typedef uint8_t (*Color_fn)(uint8_t counter);
typedef uint8_t (*Condition)(uint8_t counter, uint8_t parameter);

struct Color {
	uint8_t red, green, blue;
};

// TODO: Find out a way to do exit conditions
struct State {
	uint8_t counter;
	Color_fn red, green, blue;
	uint8_t max;
	State *nextState;
};

// Simple color ramp
uint8_t linear(uint8_t counter) {
	return counter;
}

uint8_t zero(uint8_t counter) {
	return 0;
}

// TODO: Add exit condition processing
void nextFrame(State *st) {
	if(st->counter >= st->max) {
		st->counter = 0;
		st = st->nextState;
	} else {
		st->counter++;
	}
}

Color getColor(State *st) {
	uint8_t red = st->red(st->counter);
	uint8_t green = st->green(st->counter);
	uint8_t blue = st->blue(st->counter);
	Color col = {red, green, blue};
	return col;
}

State idle = {0, linear, zero, zero, 2, &idle};

int main(int arc, char **argv)
{
	printf("Entering main\n");

	State *curState = &idle;

	int i;
	for(i = 0; i < 10; i++) {
		Color col = getColor(curState);
		printf("counter: %u\tred: %u\t green: %u\tblue: %u\n", curState->counter, col.red, col.green, col.blue);
		nextFrame(curState);
	}

	return 0;
}
