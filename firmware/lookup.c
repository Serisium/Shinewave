#include "lookup.h"

uint8_t lookup(uint8_t value) {
    return pgm_read_byte(&(table[value % NUM_SAMPLES]));
}
