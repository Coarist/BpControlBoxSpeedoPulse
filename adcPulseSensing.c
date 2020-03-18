//-----------------------------------------------------------------------------
// File:   adcPulseSensing.c
// Author: chi
//
// Created on February 11, 2020, 16:39
//-----------------------------------------------------------------------------
// Use the ADC to sense pulse - suits pulses of different magnitude 
//-----------------------------------------------------------------------------

#include <xc.h>
#include "adcPulseSensing.h"
#include "adc.h"
#include "cocoos.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "i2a.h"
#include "tmr3.h"
#include "tmr5.h"
#include "textTerm.h"

bool printRealTimeData = false;

//-----------------------------------------------------------------------------
// Low level parameter for debugging and for convenience of field use.
//-----------------------------------------------------------------------------
static char w1[12];
static char w2[12];
static char w3[12];

char __section("adcPulseSen") *adcPerVoltValue(void)  { return w1; }
char __section("adcPulseSen") *adcTrigVoltValue(void) { return w2; }
char __section("adcPulseSen") *adcSampRateValue(void) { return w3; }

void __section("adcPulseSen") adcPerVoltValueInit(void)
{
    u32_a12_d((uint32_t)(1000000UL*ADC_STEPS(1.0)), 10-6, w1);
    return;
}

void __section("adcPulseSen") adcTriggerThreshold(void)
{
    u32_a12_d((uint32_t)(100*(ADC_V_THRES+ADC_V_DIODE)), 10-2, w2);
    return;
}

void __section("adcPulseSen") adcTheoreticalSamplingRate(void)
{
    u32_a12_d((uint32_t)(ADC_F_MAX), 10-3, w3);
    return;
}

//-----------------------------------------------------------------------------
// ADC pulse detection
//-----------------------------------------------------------------------------
// Due to ADC input series diode 1N4148, model the input voltage by a 0.5V
// junction voltage drop. The minimal voltage when ADC reads zero would be this
// voltage value - the lower-end "blind spot". 
//     ADC_N_DBNCE 1..8
//     adcThreshold 0..255
//-----------------------------------------------------------------------------
#define LOGIC_LO (false)
#define LOGIC_HI (true)
volatile static bool currentLogicState = LOGIC_LO;
//static uint8_t debounceMask = (uint8_t)((1<<ADC_N_DBNCE) - 1);
static uint8_t adcThreshold = (uint8_t)(ADC_STEPS(ADC_V_THRES));
//volatile static uint8_t debounceRegister = 0;
volatile static uint8_t tmr3Byte2 = 0;
volatile static uint8_t tmr5Byte2 = 0;
volatile uinteger24_t timeIntervalHL;
volatile uinteger24_t timeIntervalLH;
volatile uint8_t adcValueLog;
    
//-----------------------------------------------------------------------------
// 16-bit timers timer3 and timer5 are allocated to accurately measure time 
// between successive H->L edges and successive L->H edges. 
//-----------------------------------------------------------------------------
//
// 16-Bit mode provides the user with the ability to accurately read all 16 
// bits of the Timer1/3/5 value from a single instance in time. In this mode
// the TMRxH register is not directly readable or writable and all read and 
// write operations take place through the use of the "TMRxH buffer" register. 
// This would be transparent to the source code. 
//
// When a read from the TMRxL register is requested, the value of the TMRxH 
// register is simultaneously loaded into the TMRxH buffer register. 
//
// When a read from the TMRxH register is requested, the value is provided
// from the TMRxH buffer register instead. 
//
// When a write request of the TMRxL register is requested, the TMRxH buffer 
// register is simultaneously updated with the contents of the TMRxH register. 
// The value of TMRxH must be preloaded into the TMRxH buffer register prior to
// the write request for the TMRxL register. This provides the user with the 
// ability to write all 16 bits to the TMRxL:TMRxH register pair at the same 
// time.
// 
// Any requests to write to the TMRxH directly does not clear the Timer1/3/5 
// prescaler value. The prescaler value is only cleared through write requests 
// to the TMRxL register.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// adcTmrxHanlder(). Timer is set to 0.5 us and is allocated to time pulse edges.
// It is 16-bit, overflows to generate an interrupt every 32.768 ms (relatively
// infrequent). tmrxByte2 is used to extend TMRxL:TMRxH to 3 x 8 == 24-bit
// which overflows at 8.39 s. When tmrxByte2 has full value of 255, the lower 
// order bytes are to be treated as 0xFFs to prevent ambiguity. 
//-----------------------------------------------------------------------------
void __section("adcPulseSen") adcTmr3Hanlder(void)
{
    if (tmr3Byte2 < UINT8_MAX) {++tmr3Byte2;}
    //---------------------------------------------------------------
    // The following line raises response time when pulse is too 
    // slow. This single line +52 bytes of code == 26 instructions ==
    // 1.625 us @ 16 MIPS (PIC18 is 2-byte 16-bit instruction word). 
    // It does not cause significant performance issue since the 
    // condition for the curly bracket to execute occurs only when 
    // the pulse train is very sparse. 
    //---------------------------------------------------------------
    if (tmr3Byte2 > TMR35_BYTE2_LIMIT) {U24ZEROIZE(timeIntervalLH);}
    return;
}
void __section("adcPulseSen") adcTmr5Hanlder(void)
{
    if (tmr5Byte2 < UINT8_MAX) {++tmr5Byte2;}
    if (tmr5Byte2 > TMR35_BYTE2_LIMIT) {U24ZEROIZE(timeIntervalHL);}
    return;
}

//-----------------------------------------------------------------------------
// The highest 8 bits of ADC conversion result are used. Remaining lowest order
// 2 bits are forgone. Effectively 8-bit sampling. 
// Device should have been configured to:
//     - left-justified conversion result
//     - ADC interrupt enabled
//-----------------------------------------------------------------------------
// 202 bytes == 101 instruction words == 6.3125 microsecond if all code 
// branches executes. Due to two same-pattern if-blocks, at most only one block
// will be executed. If no edge is detected, none of these will execute. 
// More detailed estimates:
//    No edge detected:        1 us (16 code words)
//    HL or LH edge detected:  4 us (59 code words)
// MCU execution time 
//    Very high pulse rate 4 / 19.5 is roughly 21%, leaves about 79%
//    Very low  pulse rate 1 / 19.5 is roughly  5%, leaves about 95%
// Worse case 79% execution context left - a fairly okay scenario. 
//-----------------------------------------------------------------------------
void __section("adcPulseSen") adcDataHandler(void)
{
    //---------------------------------------------------------------
    // Obtain ADC result +2 bytes == +1 code word
    //---------------------------------------------------------------
    adcValueLog = ADRESH;
    //---------------------------------------------------------------
    // Immediately start the next ADC "acquisition and conversion"
    // +2 bytes == +1 code word
    //---------------------------------------------------------------
    ADCON0bits.GO_nDONE = 1;
    //---------------------------------------------------------------
    // Shift in new (logic) bit +18 bytes == +9 code words
    //---------------------------------------------------------------
    //debounceRegister <<= 1;
    //if (adcValueLog > adcThreshold) {debounceRegister |= 1;}
    bool z = (adcValueLog > adcThreshold) ? true : false;
    //---------------------------------------------------------------
    // De-bounce pattern check, to identify pulse edges. +6 bytes ==
    // +3 code words. 
    //---------------------------------------------------------------
    //uint8_t x = debounceRegister & debounceMask;
    //---------------------------------------------------------------
    // Each same-pattern if-block is 88 bytes == 44 code words
    //----------------------------------------------------------------
    if (!currentLogicState && z) 
    {
        //-----------------------------------------------------------
        // L->H edge detected - timer3 + timeIntervalLH
        //-----------------------------------------------------------
        // MCC provides API such as TMRX_StartTimer(), TMRX_Reload(),
        // TMRX_ReadTimer(), TMRX_StopTimer(). Due to use in frequent
        // interrupt, function call overhead needs to be avoided. The
        // equivalent code is hand-written into this function.
        //-----------------------------------------------------------
        // halt timer 3
        //-----------------------------------------------------------
        T3CONbits.TMR3ON = 0;
        //-----------------------------------------------------------
        // Snapshot timer 3 counter. This line consumes +18 bytes 
        // Flash == +9 instructions. Correction value of +9/8 ==
        // +1.125 needs to be added to timeIntervalXX if precision 
        // is needed.                                       18Feb2020
        //-----------------------------------------------------------
        // Notes: (1) TMR is in "16-bit mode". Reading TMRxL will 
        //            snapshot the higher order byte 
        //        (2) which can be obtained by reading TMRxH.
        // For the above reasons, the statement order is as below. 
        //-----------------------------------------------------------
        timeIntervalLH.bytes.C0 = TMR3L; //-- (1)
        timeIntervalLH.bytes.C1 = TMR3H; //-- (2)
        timeIntervalLH.bytes.C2 = tmr3Byte2;
        //-----------------------------------------------------------
        // Reload timer 3 (use full 65535 interval therefore clear to
        // zero). Must follow this write order. 
        //-----------------------------------------------------------
        // Notes: (1) TMR is in "16-bit mode". Load high order byte
        //            in TMRxH. This value does not go into the 
        //            physical register but instead it is placed in 
        //            the "TMRxH buffer register". 
        //        (2) Write lower order byte to TMRxL. TMRxH will 
        //            load the high order byte value simultaneously. 
        // For the above reasons, the statement order is as below. 
        //-----------------------------------------------------------
        TMR3H = 0; //-- (1)
        TMR3L = 0; //-- (2)
        //-----------------------------------------------------------
        // Clear the added 3rd order byte
        //-----------------------------------------------------------
        tmr3Byte2 = 0;
        //-----------------------------------------------------------
        // Re-enable timer 3 to free running.
        //-----------------------------------------------------------
        T3CONbits.TMR3ON = 1;
        //-----------------------------------------------------------
        // Update variable so the next edge will process correctly
        //-----------------------------------------------------------
        currentLogicState = LOGIC_HI;
    }
    else if (currentLogicState && !z) 
    {
        //-----------------------------------------------------------
        // H->L edge detected - timer5 + timeIntervalHL
        //-----------------------------------------------------------
        T5CONbits.TMR5ON = 0;
        timeIntervalHL.bytes.C0 = TMR5L;
        timeIntervalHL.bytes.C1 = TMR5H;
        timeIntervalHL.bytes.C2 = tmr5Byte2;
        TMR5H = 0;
        TMR5L = 0;
        tmr5Byte2 = 0;
        T5CONbits.TMR5ON = 1;
        currentLogicState = LOGIC_LO;
    }
    // else
    // {
    //     no edge, no action.
    // }
    return;
}

static char realTimeIntervalLH[11], realTimeIntervalHL[11];
static char realTimeVoltage[12];

void __section("adcPulseSen") adc_task(void)
{
    task_open();
    //---------------------------------------------------------------
    // Statements after task_open executes once so this is an ideal 
    // spot to insert initialization code lines. 
    //---------------------------------------------------------------
    adcPerVoltValueInit();
    adcTriggerThreshold();
    adcTheoreticalSamplingRate();
    U24ZEROIZE(timeIntervalLH);
    U24ZEROIZE(timeIntervalHL);
    
    //---------------------------------------------------------------
    // Typical Co-RTOS task has an infinite loop within it lies at 
    // least one statement giving control back to OS. 
    //---------------------------------------------------------------
    for(;;) {
        task_wait( 1000 );
        if (printRealTimeData)
        {
            //-------------------------------------------------------
            // Note: TX and RX will interlace. Printing here can 
            //       interlace printing in textTerm.c.
            //-------------------------------------------------------
            u32_a12((uint32_t)U24GETVALUE(timeIntervalLH), (char*)realTimeIntervalLH);
            printa((char*) "Low to high edge time ");
            printa((char*)realTimeIntervalLH);
            printa((char*) "\n\0");
            task_wait(1);
            u32_a12((uint32_t)U24GETVALUE(timeIntervalHL), (char*)realTimeIntervalHL);
            printa((char*) "High to low edge time ");
            printa((char*)realTimeIntervalHL);
            printa((char*) "\n\0");
            task_wait(1);
            u32_a12((uint32_t)(adcValueLog), (char*)realTimeVoltage);
            printa((char*) "Pulse magnitude value ");
            printa((char*)realTimeVoltage);
            printa((char*) "\n\0");
            printa((char*) "\n\0");
        }
    }
    //---------------------------------------------------------------
    // Control will never fall beyound this point
    //---------------------------------------------------------------
    task_close();
}

#undef LOGIC_HI
#undef LOGIC_LO

//----------------------------------------------------------------- end of file
