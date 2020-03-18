//-----------------------------------------------------------------------------
// File:   audioVisual.h
// Author: chi
//
// Created on February 25, 2020, 1:38 PM
//-----------------------------------------------------------------------------

#ifndef AUDIOVISUAL_H
#define	AUDIOVISUAL_H

#include "stdint.h"
#include "stdbool.h"

void av_control_task(void);

//-----------------------------------------------------------------------------
// To simplify implementation concerning outputs such as LED, contacts and 
// buzzer, 4 types of pre-defined behaviors are put in place. 
//-----------------------------------------------------------------------------
#define AV_OFF      (0)  // Not turned on
#define AV_PSL      (1)  // Slow, short, periodic pulsation
#define AV_PSS      (2)  // Slow but even M-S periodic pulsation
#define AV_PRP      (3)  // Rapid, periodic pulsation
#define AV_FUL      (4)  // Active continously
#define AV_MODE_MAX (5)

//-----------------------------------------------------------------------------
// In the application as of on 25Feb2020, the following outputs and output
// devices are driven by the 'audioVisual' module. In addition to the defines
// below, there are further expansion possibilities:
//     RB0   SW(RED)
//     RB1   SW(GREEN)
//     RB3   LED(RED)
//     RB5   LED(GREEN)
//     RA2   I/O
//     RA3   I/O
//     RC5   1WIRE
//     RA1   ANA2
//     RB2   OPTO1
//     RB4   OPTO2
//-----------------------------------------------------------------------------
#define LED_i_RED  (0)  //-- on-board internal LED
#define LED_i_BLUE (1)  //-- on-board internal LED
#define BUZZER     (2)
#define RELAY      (3)
#define OC1        (4)
#define AV_MAX     (5)

//-----------------------------------------------------------------------------
// Structure to hold per-channel AV run-time control data
//-----------------------------------------------------------------------------
typedef struct avControlStruct
{
    uint8_t t0; // 0..254  'ON' time. 0 == full off (t1 non-zero)
    uint8_t t1; // 0..254 'OFF' time. 0 == full on  (t0 non-zero)
    uint8_t count;
    bool onOff;
}
avControlStruct_t;

bool avControl(uint8_t item, uint8_t mode);

#endif	/* AUDIOVISUAL_H */

//----------------------------------------------------------------- end of file
