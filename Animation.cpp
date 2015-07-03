
typedef struct {
	struct pt pt;
	uint8_t count;
	uint8_t idleWait;
	uint8_t idleClock;
	uint8_t idleState;
} State;

State state; 

// Ascending saw wave across an increasing n
static uint8_t saw(uint8_t n, uint16_t period) {
	return LOOKUP((uint16_t) (n % period) * (NUM_SAMPLES - 1) / (period - 1));
}

static PT_THREAD(next(State *s, volatile uint8_t controller[]))
{
	PT_BEGIN(&s->pt);

	if(controller[4] & 0x02 && controller[7] < 80) {	// down-b
		s->idleWait = 255;
		for(s->count = 0; s->count < 21; s->count++) {
			if(s->count > 2 && controller[4] & 0x0c) {		// jump
				for(s->count = 0; s->count < 30; s->count++) {
					showColor(0, saw(255 - s->count, 31), 0);
					PT_YIELD(&s->pt);
				}
				PT_EXIT(&s->pt);
			}

			showColor(0, 0, saw(255 - s->count, 6));
			PT_YIELD(&s->pt);
		}
	}


	if(!s->idleWait){
		switch(s->idleState) {
		case 0:
			showColor(255, 255 - s->idleClock, 0);		// green goes up
			break;
		case 1:
			showColor(s->idleClock, 255, 0);		// red goes down
			break;
		case 2:
			showColor(0, 255, 255 - s->idleClock);		// blue goes up
			break;
		case 3:
			showColor(0, s->idleClock, 255);		// green goes down
			break;
		case 4:
			showColor(255 - s->idleClock, 0, 255);		// red goes up
			break;
		case 5:
			showColor(255, 0, s->idleClock);		// blue goes down
			break;
		default:
			s->idleState = 0;
			break;
		}

		if(!s->idleClock--) {
			s->idleState = (s->idleState + 1) % 6;
		}
	} else {
		showColor(0, 0, 0);
		s->idleWait--;
	}
		
	PT_END(&s->pt);
}

void nextFrame(volatile uint8_t controller[])
{
	next(&state, controller);
}
