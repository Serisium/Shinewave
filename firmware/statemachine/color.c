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
#include "color.h"

#include <stdint.h>

#include "constants.h"

void Color_emplace(Color *out, uint8_t r, uint8_t g, uint8_t b) {
    out->r = r;
    out->g = g;
    out->b = b;
}
void Color_interpolate(Color *output, Color a, Color b, Interpolation method, uint8_t frac) {
    switch(method) {
        default:
        case INTER_CONSTANT:
            *output = a;
            break;
        case INTER_BINARY:
            if(frac >= 128) {
                *output = b;
            } else {
                *output = a;
            }
            break;
        case INTER_LERP:
            return Color_emplace(output,
                    (((uint16_t)a.r * (256 - frac)) >> 8) + (((uint16_t)b.r * frac) >> 8),
                    (((uint16_t)a.g * (256 - frac)) >> 8) + (((uint16_t)b.g * frac) >> 8),
                    (((uint16_t)a.b * (256 - frac)) >> 8) + (((uint16_t)b.b * frac) >> 8));
            break;
    }
    // Unreachable
}
