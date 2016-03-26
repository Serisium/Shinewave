// The MIT License (MIT)

// Copyright (c) 2016 Reid Levenick

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#include "constants.h"

#include <stdint.h>
#include <stdlib.h>

uint_fast8_t Direction_inside(Direction dir, uint8_t x, uint8_t y) {
    int16_t x_ext = (int16_t)x - 127;
    int16_t y_ext = (int16_t)y - 127;
    // The stick is considered to be in neutral if it's within 5 units of the center.
    uint8_t in_neutral = (x_ext * x_ext + y_ext * y_ext) <= 25;

    switch(dir) {
        default:
        case DIRECTION_NEUTRAL:
            return in_neutral;
        case DIRECTION_NORTH:
            return !in_neutral && y_ext >= 0 && abs(x_ext) <= y_ext;
        case DIRECTION_EAST:
            return !in_neutral && x_ext >= 0 && abs(y_ext) < x_ext;
        case DIRECTION_SOUTH:
            return !in_neutral && y_ext < 0 && -abs(x_ext) >= y_ext;
        case DIRECTION_WEST:
            return !in_neutral && x_ext < 0 && -abs(y_ext) > x_ext;
    }
    // Unreachable
}

uint8_t Pattern_light_reversal_map[40] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 1, 1,
    0, 0, 1, 0, 0,
    0, 0, 1, 1, 1,
    1, 1, 0, 0, 0,
    1, 1, 0, 1, 1,
    1, 1, 1, 0, 0,
    1, 1, 1, 1, 1
};
