#include <stdio.h>
#include <stdint.h>


typedef struct State_t {
	struct State_t (*next_fn)(void);
} State;

State state1(void);
State state2(void);

State state1(void)
{
	printf("Entering state 1\n");
	State st = {state2};
	return st;
}

State state2(void)
{
	printf("Entering state 2\n");
	State st = {state1};
	return st;
}

int main(int arc, char **argv)
{
	printf("Entering main\n");

	State curState = {state1};

	curState = curState.next_fn();
	curState = curState.next_fn();
	curState = curState.next_fn();

	return 0;
}

