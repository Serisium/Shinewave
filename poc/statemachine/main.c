#include <stdio.h>
#include <stdint.h>

typedef struct Color Color;
typedef struct State State;
typedef struct Exit Exit;

typedef uint8_t (*Color_fn)(uint8_t counter);
typedef uint8_t (*Condition)(uint8_t counter, uint8_t parameter);


// TODO: Find out a way to do exit conditions
struct State {
	uint8_t counter;
	Color_fn red, green, blue;
	uint8_t max;
	State *nextState;
	Exit *exit;
};

struct Exit {
	uint8_t byte;
	uint8_t mask;
	State *exitState;
};

struct Color {
	uint8_t red, green, blue;
};

// Simple color functions
uint8_t linear(uint8_t counter) {
	return counter;
}
uint8_t zero(uint8_t counter) {
	return 0;
}
uint8_t max(uint8_t counter) {
	return 255;
}

// Button checks
uint8_t buttonBuffer[7] = {0,0,0,0,0,0,0};
uint8_t checkButton(uint8_t byte, uint8_t mask) {
	if(buttonBuffer[byte] & mask) 
		return 1;
	else
		return 0;
}

// TODO: Add exit condition processing
State *nextFrame(State *st) {
	if(st->counter >= st->max) {
		st->counter = 0;
		st = st->nextState;
	} else if(checkButton(st->exit->byte, st->exit->mask)) {
		printf("B pressed!\n");
		st->counter = 0;
		st = st->exit->exitState;
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


State idleState;
State shineState;
Exit shineExit;

int main(int arc, char **argv)
{
	printf("Entering main\n");

	idleState = {0, linear, zero, zero, 2, &idleState, &shineExit};
	shineState = {0, zero, zero, max, 5, &idleState, &shineExit};
	shineExit = {1, 0b00000010, &shineState};

	State *curState = &idleState;

	// Progress through ten idle frames
	int i;
	for(i = 0; i < 10; i++) {
		Color col = getColor(curState);
		printf("counter: %u\tred: %u\t green: %u\tblue: %u\n", curState->counter, col.red, col.green, col.blue);
		curState = nextFrame(curState);
	}

	// Shine for one frame
	buttonBuffer[1] = 0b00000010;
	curState = nextFrame(curState);
	buttonBuffer[1] = 0b00000000;
	Color col = getColor(curState);
	printf("counter: %u\tred: %u\t green: %u\tblue: %u\n", curState->counter, col.red, col.green, col.blue);

	// Wait ten more frames
	for(i = 0; i < 10; i++) {
		Color col = getColor(curState);
		printf("counter: %u\tred: %u\t green: %u\tblue: %u\n", curState->counter, col.red, col.green, col.blue);
		curState = nextFrame(curState);
	}

	return 0;
}
