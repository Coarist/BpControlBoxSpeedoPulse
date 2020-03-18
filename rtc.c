//-----------------------------------------------------------------------------
// File:   rtc.c
// Author: chi
//
// Created on March 3, 2020, 9:21 AM
//-----------------------------------------------------------------------------
// Real-time clock time-date sub-system, using XC8 library. Note that RTC 
// should ideally be hardware for best performance in terms of both accuracy
// and low power consumption.
//-----------------------------------------------------------------------------

#include "rtc.h"
#include "stdint.h"

#define COUNTER_RELOAD_VALUE (100)
static time_t datetime;
static uint8_t i = COUNTER_RELOAD_VALUE;

//-----------------------------------------------------------------------------
// rtcTick() should be called at precise 1 second intervals typically from a 
//           timer ISR. Alternatively, use rtcTick1() instead.
//-----------------------------------------------------------------------------
void __section("rtc") rtcTick(void) {++datetime; return;}

//-----------------------------------------------------------------------------
// rtcTick1() should be called at precise fraction of seconds intervals  
//           typically from a timer ISR. EITHER use rtcTick1() OR rtcTick()
//           to drive the clock. The rate is COUNTER_RELOAD_VALUE ticks per
//           second. Adjust COUNTER_RELOAD_VALUE to suit exactly what the 
//           "fraction" is. 
//-----------------------------------------------------------------------------
void __section("rtc") rtcTick1(void) 
{
    if(i) {--i;} else {i=COUNTER_RELOAD_VALUE; ++datetime;}
    return;
}

//-----------------------------------------------------------------------------
// time() is prototyped in Microchip XC8 compiler. It is implemented here
//           in accordance with XC8 User Guide specifications:
// Return the current time in seconds which will be interpreted as the number 
// of seconds since 00:00:00 on Jan 1, 1970. If the argument t is not equal to 
// NULL, the same value will also be stored into the object pointed to by t. 
//-----------------------------------------------------------------------------
time_t __section("rtc") time(time_t *t)
{
    if (t != NULL) {*t = datetime;}
    return datetime;
}

#undef COUNTER_RELOAD_VALUE

//----------------------------------------------------------------- end of file
