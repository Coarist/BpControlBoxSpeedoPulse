//-----------------------------------------------------------------------------
// File:   edgeDetect.h
// Author: chi
//
// Created on February 18, 2020, 2:37 PM
//-----------------------------------------------------------------------------
// Use comparator to sense pulse - suits pulses of different magnitude 
//-----------------------------------------------------------------------------

#ifndef EDGEDETECT_H
#define	EDGEDETECT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include "cmp1.h"
#include "dac.h"
    
//-----------------------------------------------------------------------------
// DAC parameters
//-----------------------------------------------------------------------------
#define DAC_V_HI (5.0)
#define DAC_V_LO (0.0)
#define DAC_N_BIT (5)    // 5-bit DAC

//-----------------------------------------------------------------------------
// Series diode VF forward voltage drop is modeled as CMP_V_DIODE
//-----------------------------------------------------------------------------
#define CMP_V_DIODE (0.5)

//-----------------------------------------------------------------------------
// PD Potential Divider resistor ratio at input terminal
//-----------------------------------------------------------------------------
#define CMP_PD_R_TOTAL (1200+82+560+6800)
#define CMP_PD_R_DIVIDE (1200)
    
//-----------------------------------------------------------------------------
// Comparator fixed voltage provided by DAC. Microchip DS40001412G-page 333 
// EQUATION 22-1. // DAC_V_LO <= v <= DAC_V_HI
//-----------------------------------------------------------------------------
#define CMP_V_THRES ((DAC_V_HI-DAC_V_LO)*VREFCON2/(1<<DAC_N_BIT)+DAC_V_LO)
#define DAC_VREFCON2(v) ((uint8_t)((1<<DAC_N_BIT)*(v-DAC_V_LO)/(DAC_V_HI-DAC_V_LO))

//-----------------------------------------------------------------------------
// Comparator 1 output gates 16-bit timer 1 which is set to FOSC/4 using MCC
//-----------------------------------------------------------------------------
#define TMR1_OVERFLOW_US (4096)   

//-----------------------------------------------------------------------------
// Functions and variables to be exposed to any code module that #include this
// header file.
//-----------------------------------------------------------------------------
extern bool printRealTimeData;
char* cmpTrigVoltage(void);
void realTimeReport_task(void); 
void senseTrigger_task(void);
uint32_t getPulsePeriod24(void);

#ifdef	__cplusplus
}
#endif

#endif	/* EDGEDETECT_H */
//----------------------------------------------------------------- end of file

