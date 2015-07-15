#include <stdio.h>
#include <stdint.h>

typedef int (*State)(void);
State state1(void);
State state2(void);

State curState;

State state1(void)
{
	printf("Entering state 1\n");
	return (State)state2;
}

State state2(void)
{
	printf("Entering state 2\n");
	return (State)state1;
}

int main(void)
{
	printf("Entering main\n");

	curState = state1;

	curState = curState();
	curState = curState();
	curState = curState();

	return 0;
}

