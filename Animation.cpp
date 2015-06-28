
typedef struct {
	struct pt pt;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} State;

State state; 

static PT_THREAD(next(State *s, volatile uint8_t controller[]))
{
	PT_BEGIN(&s->pt);

	if(controller[4] & 0x02) {
		showColor(0, 0, 255);
		PT_YIELD(&s->pt);
		showColor(0, 0, 0);
		PT_YIELD(&s->pt);
		showColor(0, 0, 255);
		PT_YIELD(&s->pt);
		showColor(0, 0, 0);
		PT_YIELD(&s->pt);
	}

	PT_END(&s->pt);
}

void nextFrame(volatile uint8_t controller[])
{
	next(&state, controller);
}
