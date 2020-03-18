//-----------------------------------------------------------------------------
// File:   alarm.h
// Author: chi
//
// Created on February 25, 2020, 11:57 AM
//-----------------------------------------------------------------------------

#include "alarm.h"
#include "cocoos.h"
#include "edgeDetect.h"
#include "stdint.h"
#include "stdbool.h"
#include "audioVisual.h"
#include "uintegers2.h"
#include "i2a.h"

//-----------------------------------------------------------------------------
// Parameters pAlarmLevel and mAlarmLevel
//   - Range: 0..0x00FFFFFF
//   - Zero will never trigger thus setting them to 0 effectively disables the
//     alarm level. 
// Parameter sampleInterval is in steps of 10 milli-seconds.
//-----------------------------------------------------------------------------
uint32_t pAlarmLevel = 0;
uint32_t mAlarmLevel = 0;
uint8_t sampleInterval = 50;
static uint8_t samplingCount; // x10ms, max 2550 ms = 2.5 seconds

char __section("alarmAlg") *pAlarmHexString(void)
{
    static char s[12];
    uinteger32_t x;
    x.value = pAlarmLevel;
    trimLeft(u32_to_hex(x, 3), s);
    return s;
}

char __section("alarmAlg") *mAlarmHexString(void)
{
    static char s[12];
    uinteger32_t x;
    x.value = mAlarmLevel;
    trimLeft(u32_to_hex(x, 3), s);
    return s;
}

char __section("alarmAlg") *mIntervalDecString(void)
{
    static char s[12];
    trimLeft(u32_to_a11(sampleInterval), s);
    return s;
}
//-----------------------------------------------------------------------------
// Begin coding with simple straight-forward alarm algorithm. DSP-like feature
// such as moving average and odd value rejection are to be considered later. 
// Alarm is self-canceling (at the moment) and it does not require user action
// to clear save making the alarm condition go away. 
//-----------------------------------------------------------------------------
void __section("alarmAlg") alarm_task(void)
{
    task_open();
    //---------------------------------------------------------------
    samplingCount = 0; // initialize (sit between task_open() & for()
    avControl(LED_i_BLUE, AV_PSL);
    avControl(  BUZZER  , AV_OFF);
    //---------------------------------------------------------------
    for(;;) 
    {
        task_wait(10);
        if(++samplingCount < sampleInterval) {continue;}
        samplingCount = 0;
        uinteger32_t x;
        x.value = getPulsePeriod24();
        if (x.value < mAlarmLevel)
        {
            avControl(LED_i_BLUE, AV_FUL);
            avControl(  BUZZER  , AV_FUL);
        }
        else if (x.value < pAlarmLevel)
        {
            avControl(LED_i_BLUE, AV_PSS);
            avControl(  BUZZER  , AV_PRP);
        }
        else
        {
            avControl(LED_i_BLUE, (x.bytes.C3) ? AV_OFF : AV_PSL);
            avControl(  BUZZER  , AV_OFF);
        }
    }
    task_close(); //--------- Control will never fall onto this point
}

//----------------------------------------------------------------- end of file
