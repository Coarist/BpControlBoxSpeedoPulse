//-----------------------------------------------------------------------------
// File:   audioVisual.c
// Author: chi
//
// Created on February 25, 2020, 1:39 PM
//-----------------------------------------------------------------------------

#include "xc.h"
#include "audioVisual.h"
#include "pin_manager.h"
#include "cocoos.h"
#include "stdbool.h"
#include "pin_manager.h"
#include "tmr2.h"

static avControlStruct_t avc[AV_MAX], *p;
#define ONETENTHSECOND (4)

//-----------------------------------------------------------------------------
// Configure the designated AV device to one of the standard output mode.
// Function return value:
//     true  == success
//     false == failed
// t0, t1 scales to clock tick rate implemented in the control code module. 
// Propose 25 ms, x4 = 100 ms. 
//-----------------------------------------------------------------------------
bool __section("AV") avControl(uint8_t item, uint8_t mode)
{
    if (item >= AV_MAX) {return false;}
    avControlStruct_t *c = &avc[item];
    switch(mode)
    {
        case AV_OFF: 
            c->t0 = 0;
            c->t1 = 1;
            break;
        case AV_PSL:
            c->t0 = (uint8_t)( 0.25 * ONETENTHSECOND);
            c->t1 = (uint8_t)(19.75 * ONETENTHSECOND);  
            break;
        case AV_PSS:
            c->t0 = (uint8_t)(5 * ONETENTHSECOND);
            c->t1 = (uint8_t)(5 * ONETENTHSECOND);  
            break;
        case AV_PRP:
            c->t0 = (uint8_t)(2.5 * ONETENTHSECOND);
            c->t1 = (uint8_t)(2.5 * ONETENTHSECOND);
            break;
        case AV_FUL:
            c->t0 = 1;
            c->t1 = 0;            
            break;
        default:
            return false;
            break;
    }
    return true;
}

//-----------------------------------------------------------------------------
// Second function call argument *p is moved outside to local file-scope 
// variable. The intention was to save function call overhead. To use this 
// function, p must be pre-loaded with the correct pointer address.   26Feb2020
//-----------------------------------------------------------------------------
// Note LEDs are opposite logic, pull low = ON, pull high = OFF, due to circuit
// Originally PWM is used for the red LED. Although modulation of light 
// intensity can easily be done with PWM, gradation is not needed. The red LED 
// driver was reverted to simple ON-OFF.                              14Feb2020
//-----------------------------------------------------------------------------
static void switchState(uint8_t device)
{
    switch(device)
    {
        case LED_i_RED: 
            if (!p->onOff) {LED_RED_SetHigh(); } else {LED_RED_SetLow(); }
            break;
        case LED_i_BLUE:
            if (!p->onOff) {LED_BLUE_SetHigh();} else {LED_BLUE_SetLow();}
            break;
        case BUZZER:
            if (!p->onOff) {TMR2_StopTimer();  } else {TMR2_StartTimer();}
            break;
        case RELAY:
            if (!p->onOff) {RELAY_SetLow();    } else {RELAY_SetHigh();  }
            break;
        case OC1:
            if (!p->onOff) {OC1_SetLow(); /*-*/} else {OC1_SetHigh();/**/}
            break;
        default:
            break; //-------------------------------------- no action
    }
    return;
}

//-----------------------------------------------------------------------------
// Task that centrally controls every attached available audio-visual device.
// Base tick rate is set by task_wait() in the infinite for-loop.
//-----------------------------------------------------------------------------
void __section("AV") av_control_task(void)
{
    task_open();
    
    //---------------------------------------------------------------
    // Initialization sits between task_open() and for(;;)
    //---------------------------------------------------------------
    for (uint8_t i = 0; i < AV_MAX; ++i)
    {
        p = &avc[i];
        p->count = p->t0 = p->t1 = 0;
        p->onOff = false;
        switchState(i); //------------------------- set initial state
    }
    //---------------------------------------------------------------
    // System health indicator uses the in-circuit red LED
    //---------------------------------------------------------------
    avControl(LED_i_RED , AV_PSS);

    //---------------------------------------------------------------
    // Task indefinite for(;;) with no termination condition
    //---------------------------------------------------------------
    for(;;) 
    {
        //-----------------------------------------------------------
        // Pace at 25 OS ticks each 1 ms. Total 25 ms. 
        //-----------------------------------------------------------
        task_wait( 25 );
        
        for (uint8_t i = 0; i < AV_MAX; ++i)
        {
            p = &avc[i];
            
            uint8_t j = p->count + 1;
            if(!p->onOff)
            {
                if (p->t0 && (!p->t1 || (j > p->t1))) 
                    {j = 0; p->onOff = true; switchState(i);}
            }
            else
            {
                if (p->t1 && (!p->t0 || (j > p->t0))) 
                    {j = 0; p->onOff = false; switchState(i);}
            }
            p->count = j;
        }
    }
    task_close(); //--------- control will never fall onto this point
}

#undef ONETENTHSECOND

//----------------------------------------------------------------- end of file
