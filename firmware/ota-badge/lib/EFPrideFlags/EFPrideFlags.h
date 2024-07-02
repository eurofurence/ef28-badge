#ifndef EFPRIDEFLAGS_H_
#define EFPRIDEFLAGS_H_

#include <FastLED.h>


#define EFPRIDEFLAGS_NUM_LEDS 11

class EFPrideFlags {

    public:
        const static CRGB LGBT[EFPRIDEFLAGS_NUM_LEDS];
        const static CRGB LGBTQI[EFPRIDEFLAGS_NUM_LEDS];
        const static CRGB Bisexual[EFPRIDEFLAGS_NUM_LEDS];
        const static CRGB Polyamorous[EFPRIDEFLAGS_NUM_LEDS];
        const static CRGB Polysexual[EFPRIDEFLAGS_NUM_LEDS];
        const static CRGB Transgender[EFPRIDEFLAGS_NUM_LEDS];
        const static CRGB Pansexual[EFPRIDEFLAGS_NUM_LEDS];
        const static CRGB Asexual[EFPRIDEFLAGS_NUM_LEDS];
        const static CRGB Genderfluid[EFPRIDEFLAGS_NUM_LEDS];
        const static CRGB Genderqueer[EFPRIDEFLAGS_NUM_LEDS];
        const static CRGB Nonbinary[EFPRIDEFLAGS_NUM_LEDS];
        const static CRGB Intersex[EFPRIDEFLAGS_NUM_LEDS];

};

#endif /* EFPRIDEFLAGS_H_ */
