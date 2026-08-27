#ifndef EFPRIDEFLAGS_H_
#define EFPRIDEFLAGS_H_

// MIT License
//
// Copyright 2024 Eurofurence e.V. 
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the “Software”),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @author Honigeintopf
 */

#include "EFLed.h"

/**
 * @brief Pride flag LED patterns for the EFBar to use with the EFLed library
 */
class EFPrideFlags {

    public:
        const static CRGB LGBT[EFLED_EFBAR_NUM];
        const static CRGB LGBTQI[EFLED_EFBAR_NUM];
        const static CRGB Bisexual[EFLED_EFBAR_NUM];
        const static CRGB Polyamorous[EFLED_EFBAR_NUM];
        const static CRGB Polysexual[EFLED_EFBAR_NUM];
        const static CRGB Transgender[EFLED_EFBAR_NUM];
        const static CRGB Pansexual[EFLED_EFBAR_NUM];
        const static CRGB Asexual[EFLED_EFBAR_NUM];
        const static CRGB Genderfluid[EFLED_EFBAR_NUM];
        const static CRGB Genderqueer[EFLED_EFBAR_NUM];
        const static CRGB Nonbinary[EFLED_EFBAR_NUM];
        const static CRGB Intersex[EFLED_EFBAR_NUM];

};

#endif /* EFPRIDEFLAGS_H_ */
