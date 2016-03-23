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
#ifndef STATEMACHINE_CONSTANTS_H
#define STATEMACHINE_CONSTANTS_H

#include <stdint.h>

typedef uint8_t Direction;
#define DIRECTION_NEUTRAL   0
#define DIRECTION_NORTH     1
#define DIRECTION_EAST      2
#define DIRECTION_SOUTH     3
#define DIRECTION_WEST      4

uint_fast8_t Direction_inside(Direction dir, uint8_t x, uint8_t y);

typedef uint8_t Interpolation;
#define INTER_CONSTANT      0
#define INTER_BINARY        1
#define INTER_LERP          2

typedef uint8_t Looping;
#define LOOP_STICK          0
#define LOOP_WRAP           1
#define LOOP_BOUNCE         2

typedef uint8_t Selection;
#define SELECTION_ALL       0
#define SELECTION_ANY       1

// Patterns for the 5 lights
// Bits 0 through 4 bitmask which lights are on.
// 0b___00001 = far left
// 0b___00010 = second from left
// etc...
// Bits 5 through 7 bitmask which lights are reversed to travel B -> A rather than A -> B
// 0b000_____ = reverse nothing
// 0b001_____ = reverse right
// 0b010_____ = reverse center
// 0b100_____ = reverse left
typedef uint8_t Pattern;
uint8_t Pattern_light_reversal_map[40];

#define PATTERN_LIGHT_ON(pattern, light) ((pattern) & (1 << light))
#define PATTERN_LIGHT_REVERSE(pattern, light) (Pattern_light_reversal_map[((pattern) >> 5) * 5 + (light)])

#endif
