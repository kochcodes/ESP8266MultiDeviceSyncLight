#ifndef __BLINK_ROUTINES__
#define __BLINK_ROUTINES__

#include <Arduino.h>

#define LED_MIN 256
#define LED_MAX 0

class BlinkRoutine
{
public:
    // will be overwritten by child class
    virtual int loop(long t)
    {
        return 0;
    };

    int getDuration()
    {
        return this->duration;
    }
    BlinkRoutine()
    {
        duration = 1000;
    }

protected:
    long duration;
};

class DoubleBlinkSemiRoutine : public BlinkRoutine
{
private:
    int state_1_on = 25;
    int state_2_off = state_1_on + 100;
    int state_3_on = state_2_off + 25;
    int state_4_off = state_3_on + 100;
    int state_5_semi = state_4_off + 1200;
    int duration = state_5_semi + 100;

public:
    DoubleBlinkSemiRoutine() {}

    virtual int loop(long t)
    {
        int v = t % duration;
        if (v < state_1_on)
        {
            return 0;
        }
        else if (v < state_2_off)
        {
            return 255;
        }
        else if (v < state_3_on)
        {
            return 0;
        }
        else if (v < state_4_off)
        {
            return 255;
        }
        else if (v < state_5_semi)
        {
            return 250;
        }
        else
        {
            return 255;
        }
    }
};

class SingleBlinkRoutine : public BlinkRoutine
{
private:
    long on_state = 80;   // ms
    long off_state = 900; // ms

public:
    SingleBlinkRoutine()
    {
        on_state = 80;                   // ms
        off_state = 900;                 // ms
        duration = on_state + off_state; // ms
    }
    SingleBlinkRoutine(int on_time_in_ms, int off_time_in_ms)
    {
        on_state = on_time_in_ms;                    // ms
        off_state = off_time_in_ms;                  // ms
        duration = this->on_state + this->off_state; // ms
    }
    virtual int loop(long t)
    {
        return (t % duration <= on_state) ? LED_MAX : LED_MIN;
    }
};

class DoubleBlinkRoutine : public BlinkRoutine
{
private:
    long on_state = 80;    // ms
    long off_state = 160;  // ms
    long on_state_2 = 240; // ms

public:
    DoubleBlinkRoutine()
    {
        this->duration = 1000;
    }
    DoubleBlinkRoutine(int on_time_in_ms, int off_time_in_ms, int on_time_in_ms_2, int off_time_in_ms_2)
    {
        on_state = on_time_in_ms;                                                       // ms
        off_state = on_time_in_ms + off_time_in_ms;                                     // ms
        on_state_2 = on_time_in_ms + off_time_in_ms + on_time_in_ms_2;                  // ms
        duration = on_time_in_ms + off_time_in_ms + on_time_in_ms_2 + off_time_in_ms_2; // ms
    }
    virtual int loop(long t)
    {
        if (t % duration <= on_state)
            return LED_MAX;
        if (t % duration <= off_state)
            return LED_MIN;
        if (t % duration <= on_state_2)
            return LED_MAX;
        return LED_MIN;
    }
};

class FadeRoutine : public BlinkRoutine
{
public:
    FadeRoutine()
    {
        this->duration = 1000;
    };
    FadeRoutine(long duration)
    {
        this->duration = duration;
    }
    int loop(long t)
    {
        return abs(((t % duration) / (duration * 1.0) * 2 * LED_MIN) - LED_MIN);
    }
};

#endif