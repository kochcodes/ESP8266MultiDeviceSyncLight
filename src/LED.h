#ifndef __LED__
#define __LED__

#include <Arduino.h>
#include <BlinkRoutines.h>
#define pin D4

enum LED_STATE
{
    LED_STATE_ON,
    LED_STATE_OFF,
    LED_STATE_DOUBLE_BLINK_ROUTINE
};

class LED
{
public:
    LED()
    {
        pinMode(pin, OUTPUT);
    };

    long offset = 0;
    LED_STATE state;
    BlinkRoutine *routine;

    void loop(long t)
    {
        analogWrite(pin, routine->loop(t - offset));
    }
    void synchronize(long t)
    {
        Serial.printf("Sync: %ld\n", t);
        this->offset = t;
    }
    int setRoutine(BlinkRoutine *r, long sync)
    {
        this->routine = r;
        this->synchronize(sync);
        return this->routine->getDuration();
    }
};

#endif