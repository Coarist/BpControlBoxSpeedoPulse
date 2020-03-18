//-----------------------------------------------------------------------------
// File:   adcPulseSensing.h
// Author: chi
//
// Created on February 11, 2020, 16:38
//-----------------------------------------------------------------------------
// Use the ADC to sense pulse - suits pulses of different magnitude 
//-----------------------------------------------------------------------------

#ifndef ADCPULSESENSING_H
#define	ADCPULSESENSING_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "xc.h"
#include "stdint.h"
#include "stdbool.h"
#include "uintegers2.h"

extern bool printRealTimeData;
    
char *adcPerVoltValue(void);
char *adcTrigVoltValue(void);
char *adcSampRateValue(void);
void adcDataHandler(void);
void adc_task(void);
void adcTmr3Hanlder(void);
void adcTmr5Hanlder(void);

//=============================================================================
// Hardware and low level parameters pertaining to ADC1
//=============================================================================

//-----------------------------------------------------------------------------
// Series diode VF forward voltage drop is modeled as ADC_V_DIODE
//-----------------------------------------------------------------------------
#define ADC_V_DIODE (0.5)

//-----------------------------------------------------------------------------
// Threshold voltage above which value is counted as '1'. Input at connector 
// need to be ADC_V_DIODE + ADC_V_THRES to trigger a '1'. 
//-----------------------------------------------------------------------------
#define ADC_V_THRES (0.8)

//-----------------------------------------------------------------------------
// Full scale voltage at which ADC output is maximum value and above which 
// ADC will not be able to resolve (plus possible damage to input pin)
//-----------------------------------------------------------------------------
#define ADC_V_FULLS (5.0)

//-----------------------------------------------------------------------------
// Input circuit potential divider expressed as fraction (to avoid floating 
// point calculation due to MCU is integer-only)
//-----------------------------------------------------------------------------
#define ADC_R_NUMER (1200)
#define ADC_R_DENOM (6800+560+82+ADC_R_NUMER)

//-----------------------------------------------------------------------------
// ADC resolution is 10 bits. The output is left-justified, and the upper byte
// is used. Effectively using 8 most significant bits.
//-----------------------------------------------------------------------------
#define ADC_N_BITS (8)
#define ADC_N_RESOLUTION (1<<ADC_N_BITS)

//-----------------------------------------------------------------------------
// Combining the parameters, for the given voltage v, ADC will give reading of
// ADC_STEPS(v).
//-----------------------------------------------------------------------------
#define ADC_STEPS(v) (v*ADC_R_NUMER*ADC_N_RESOLUTION/ADC_R_DENOM/ADC_V_FULLS)

//-----------------------------------------------------------------------------
// Acquisition Time refer "BP_Control_Box_ADC_sampling_speed". This refers to 
// the "settling time" required by the "sample and hold" circuit prior to ADC
// conversion. Unit is micro-second us. Either by setting ADCON2bits.ACQT 
// or by manually inserting this time interval before setting GO/DONE to begin
// conversion. Capacitor charge up is exponential grow curve, shorter charge up
// time will achieve a high percentage voltage being held across the capacitor 
// plates. Where accuracy f pulse magnitude is not required, T_ACQ can be 
// reduced to trade in higher sampling rate. Calculated precise voltage capture
// up to temperature 80C gives T_ACQ == 8.695. 
//-----------------------------------------------------------------------------
// Fine tuning notes: 
// Tested to confirm proper pulse width measurement up to about 9 kHz. After
// reducing T_ACQ (see below) this goes up to about 18K. Speed pulses are 
// expected to be sub-kHz which is comfortably below this limit. 
// Since capacitor charge up voltage rises exponentially, it is observed that 
// shortly after initial charging up, voltage rises to significant portion of 
// of the steady state final voltage. Use a lower threshold voltage together 
// with a reduction of T_ACQ increases the maximum frequency that we are able
// to handle.                                                         18Feb2020
//-----------------------------------------------------------------------------
//#define ADC_T_ACQ (8.695)
#define ADC_T_ACQ (ADCON2bits.ACQT)

//-----------------------------------------------------------------------------
// Conversion Time refer "BP_Control_Box_ADC_sampling_speed". This refers to 
// the time required for the successive approximation ADC to complete. Unit is
// micro-seconds us. 
//-----------------------------------------------------------------------------
#define ADC_T_CON (11.5)

//-----------------------------------------------------------------------------
// "Acquisition" + "Conversion": sampling interval and sampling rate  
//-----------------------------------------------------------------------------
#define ADC_T_MIN (ADC_T_ACQ+ADC_T_CON)
#define ADC_F_MAX (1000000.0/ADC_T_MIN)

//-----------------------------------------------------------------------------
// Software de-bounce count 1..8 should NOT be zero. The higher the de-bounce 
// count, the greater noise rejection, at a small trade-off of response time.
// 1 == no de-bounce, 0 is not allowed. 
//-----------------------------------------------------------------------------
#define ADC_N_DBNCE (1)

//=============================================================================
// Parameters that are planned to be made configurable such as ADC_N_DBNCE and
// ADC_T_ACQ would probably be moved to EEPROM at a later stage of code
// development when this code is more mature.                         16Feb2020
//=============================================================================

////-----------------------------------------------------------------------------
//// Timer 3,5 characteristics (timer 3,5 are allocated to count time intervals  
//// for pulse edges.
////-----------------------------------------------------------------------------
//#define TMR35_OVERFLOW_US (32768)
//
////-----------------------------------------------------------------------------
//// Timer 3,5 are 16-bit. An addition byte tmrxByte2 is used to extend to 24-bit
////-----------------------------------------------------------------------------
//#define TMR35_BYTE2_LIMIT_SECONDS (2.0)
//#define TMR35_BYTE2_LIMIT (TMR35_BYTE2_LIMIT_SECONDS*1000000/TMR35_OVERFLOW_US)
//
#ifdef	__cplusplus
}
#endif

#endif	/* ADCPULSESENSING_H */
//----------------------------------------------------------------- end of file

