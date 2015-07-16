#include <stdio.h>
#include <stdint.h>


/*
typedef struct State_t {
	struct State_t (*next_fn)(void);
	uint8_t counter;
} State;
*/

typedef uint8_t (*Color)(uint8_t counter);

typedef struct State_t {
	uint8_t counter;
	Color red;
	Color green;
	Color blue;
} State;

// Simple color animation
uint8_t increment(uint8_t counter) {
	return counter;
}

State curState = {0, increment, increment, increment};

/*
State animation(void);
State idle(void);

State animation(void)
{
	printf("Entering animation, frame \n");
	State st = {animation, 0};
	return st;
}

State idle(void)
{
	printf("Entering idle\n");
	State st = {idle, 0};
	return st;
}
*/

int main(int arc, char **argv)
{
	printf("Entering main\n");

	//State curState = {animation};
	
	for(curState.counter = 0; curState.counter < 10; curState.counter++) {
		printf("counter: %u\tred: %u\t green: %u\tblue: %u\n", curState.counter, curState.red(curState.counter), curState.blue(curState.counter), curState.green(curState.counter));
	}


	return 0;
}
