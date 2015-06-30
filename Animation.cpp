
typedef struct {
	struct pt pt;
	uint8_t count;
	uint8_t idleCount;
	uint8_t idleState;
} State;

State state; 

static PT_THREAD(next(State *s, volatile uint8_t controller[]))
{
	PT_BEGIN(&s->pt);

	if(controller[4] & 0x02 && controller[7] < 80) {	// down-b
		for(s->count = 0; s->count < 21; s->count++) {
			if(s->count > 2 && controller[4] & 0x0c) {		// jump
				for(s->count = 0; s->count < 30; s->count++) {
					showColor(0, SAW_DESC(s->count, 31), 0);
					PT_YIELD(&s->pt);
				}
				PT_EXIT(&s->pt);
			}

			showColor(0, 0, SAW_DESC(s->count, 6));
			PT_YIELD(&s->pt);
		}
	}

	switch(s->idleState) {
	case 0:
		showColor(255, SAW_DESC(s->idleCount, 256), 0);		// green goes up
		break;
	case 1:
		showColor(SAW_ASC(s->idleCount, 256), 255, 0);		// red goes down
		break;
	case 2:
		showColor(0, 255, SAW_DESC(s->idleCount, 256));		// blue goes up
		break;
	case 3:
		showColor(0, SAW_ASC(s->idleCount, 256), 255);		// green goes down
		break;
	case 4:
		showColor(SAW_DESC(s->idleCount, 256), 0, 255);		// red goes up
		break;
	case 5:
		showColor(255, 0, SAW_ASC(s->idleCount, 256));		// blue goes down
		break;
	default:
		s->idleState = 0;
		break;
	}

	if(!s->idleCount--) {
		s->idleState = (s->idleState + 1) % 6;
	}
		
	PT_END(&s->pt);
}

void nextFrame(volatile uint8_t controller[])
{
	next(&state, controller);
}
