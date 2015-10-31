
typedef struct {
	struct pt pt;
	uint8_t count;
	uint16_t idleWait;
	uint8_t idleClock;
	uint8_t idleState;
} State;

typedef struct {
	struct pt pt;
	uint8_t count;
} JumpState;

uint8_t brightness = 8;
State state; 
JumpState jumpState;

// Ascending saw wave across an increasing n
static uint8_t saw(uint8_t n, uint16_t period) {
	return LOOKUP((uint16_t) (n % period) * (NUM_SAMPLES - 1) / (period - 1)) * brightness / 8;
}

static PT_THREAD(jump(JumpState *s, volatile uint8_t controller[]))
{
	PT_BEGIN(&s->pt);
	for(s->count = 0; s->count < 30; s->count++) {
		if(controller[4] & 0x0c && s->count > 20) {		// Double jump
			s->count = 0;
		} else if(controller[9] < 80) {				// Down-air
			for(s->count = 0; s->count < 24; s->count++) {
				if(controller[4] & 0x02 && controller[7] < 80) {		// down-b
					for(s->count = 0; s->count < 21; s->count++) {
						showColor(saw(255 - s->count, 6), 215 * saw(255 - s->count, 6) / 255, 0, brightness);
						PT_YIELD(&s->pt);
					}
					PT_EXIT(&s->pt);
				}
				showColor(saw(24 - s->count, 6), 0, 0, brightness);
                showColor(saw(24 - s->count, 6), 165 * saw(24 - s->count, 6) / 255, 0, brightness);
				PT_YIELD(&s->pt);
			}
			PT_EXIT(&s->pt);
		}
		showColor(saw(30 - s->count, 30), 140 * saw(30 - s->count, 30) / 255, 0, brightness);
		PT_YIELD(&s->pt);
	}

	PT_END(&s->pt);
}

static PT_THREAD(next(State *s, volatile uint8_t controller[]))
{
	PT_BEGIN(&s->pt);

	if(controller[4] & 0x02 && controller[7] < 80) {		// down-b
		for(s->count = 0; s->count < 21; s->count++) {
			if(s->count > 2 && controller[4] & 0x0c) {	// jump
				PT_SPAWN(&s->pt, &jumpState.pt, jump(&jumpState, controller));
				PT_EXIT(&s->pt);
			}

            showColor(saw(255 - s->count, 6), 215 * saw(255 - s->count, 6) / 255, 0, brightness);
			PT_YIELD(&s->pt);
		}
		RESET_IDLE;
		PT_EXIT(&s->pt);
	} else if(controller[4] & 0x0c) {				// jump
		PT_SPAWN(&s->pt, &jumpState.pt, jump(&jumpState, controller));
		RESET_IDLE;
		PT_EXIT(&s->pt);
	} else if(controller[5] & 0x0b) {				// taunt
		for(s->count = 0; s->count < 114; s->count++) {
			showColor(saw(s->count, 5), saw(s->count, 13), saw(s->count, 17), brightness);
			PT_YIELD(&s->pt);
		}
		RESET_IDLE;
		PT_EXIT(&s->pt);
	} else if(controller[5] & 0x04) {
		brightness = (brightness + 1) % 9;
		for(s->count = 0; s->count < 20; s->count++) {
			showColor(255, 255, 255, brightness);
			PT_YIELD(&s->pt);
		}
		RESET_IDLE;
		PT_EXIT(&s->pt);
	}
	
	if(controller[4] & 0x1f || controller[5] & 0x7f) {	// other action
		RESET_IDLE;
		PT_EXIT(&s->pt);
	}

	if(!s->idleWait){
        showColor(50, 50, 50, brightness);
	} else {
		showColor(0, 0, 0, brightness);
		s->idleWait--;
	}
		
	PT_END(&s->pt);
}

void initAnimation(void)
{
	PT_INIT(&state.pt);
	PT_INIT(&jumpState.pt);
}

void nextFrame(volatile uint8_t controller[])
{
	next(&state, controller);
}
