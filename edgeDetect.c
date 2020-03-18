//-----------------------------------------------------------------------------
// File:   edgeDetect.c
// Author: chi
//
// Created on February 18, 2020, 14:45
//-----------------------------------------------------------------------------
// CMP1 -> gates TMR1 -> obtains 24-bit unsigned integer counter
//-----------------------------------------------------------------------------
// Alert: Code in this file requires intense reading. Characteristics are:
//        (1) Tightly knit business logic
//        (2) Hand-optimized to minimal RAM size
//        (3) Hand-optimized to low code execution and ROM/FLASH usage
//        (4) Highest possible performance and functionality
//-----------------------------------------------------------------------------
// Pulse period detection and timing 8% error at 2 MHz. Timer 1 clock at FOSC/4
// can be re-configured to FOSC giving 8 MHz BW. This will need changing from
// more efficient 24-bit unsigned computation to 32-bit unsigned. For speed 
// pulse sensing, efficiency is preferred over BW.                    21Feb2020
//-----------------------------------------------------------------------------
// "sparse edge mode" : sense time interval between successive pulse edges of
//                      opposite polarity (half-period count)
// "continuous mode"  : sense full pulse period by counting time interval 
//                      between successive edges of the same polarity, 
//                      automatically start sensing the next pulse period 
//                      immediately, no missing pulse.
// "single pulse mode": At higher pulse frequencies, sampling every successive
//                      pulse period causes too much interrupts which eats up
//                      MCU execution time. Full pulse period is sampled every
//                      100 ms. 
//-----------------------------------------------------------------------------
// Variables whose value is altered only by ISR:
//     (1) Declare volatile
//-----------------------------------------------------------------------------
// Variable whose value is altered by both normal code and ISR:
// Variable whose value fetch needs more than one instruction cycle
//     (1) Declare volatile
//     (2) Disable the related interrupt when main code accesses them
//-----------------------------------------------------------------------------

#include <xc.h>
#include "edgeDetect.h"
#include "cocoos.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "i2a.h"
#include "uintegers2.h"
#include "cmp1.h"
#include "tmr1.h"
#include "textTerm.h"
#include "i2a.h"

#define T0T1_INIT_VAL (45)
#define C25MS_NO_PULSE_THRE (42*4)

//-----------------------------------------------------------------------------
// Extend detection to sub-Hz pulses. In speed pulse application, it is not
// possible to detect standstill and moving slowly. The response time will be
// minimum 1 pulse interval. This is slow. Added such extension improves 
// coverage at the lower end - increase safety.                       21Feb2020
//-----------------------------------------------------------------------------
volatile static bool sparseEdgeMode = false;
volatile static bool isFirstSampleAfterModeSwitching;

//-----------------------------------------------------------------------------
// Timer1 Gate Single-Pulse mode:
//     Continuous mode when triggered at > 200kHz pulse frequency can generate 
//     frequent interrupt depriving OS of execution time. Mode switching is 
//     implemented likened to gears in a car. The T1GSPM bit on the T1GCON 
//     register flags this status.
//-----------------------------------------------------------------------------
// Timer1 Gate Toggle mode
//     This mode measures the full-cycle length of the gate signal, as opposed
//     to the duration of a single level pulse. This method is chose since in 
//     speed pulse detection we do not wish to place an unnecessary requirement
//     that pulse-high will have same duration of pulse-low. In this mode, a
//     rising edge releases the gate, the next rising edge triggers the gate to
//     stop timer 1 counting. Long pulses at low speed will register 
//     excessively slowly. A count up variable is introduced to help with this
//     scenario (improve response time to really slow or a complete lack of 
//     pulses).                                                       20Feb2020
//-----------------------------------------------------------------------------
volatile static uint8_t c25ms, t0, t1; 

//-----------------------------------------------------------------------------
// Control flag if 'true' will print to EUSART real-time acquisition data
//-----------------------------------------------------------------------------
bool printRealTimeData = false;

//-----------------------------------------------------------------------------
// An additional high order byte is tmrxByte2 is added to extend the 16-bit
// timer to 3-byte 24-bit unsigned. 
//-----------------------------------------------------------------------------
volatile static uint8_t tmr1Byte2 = 0;
volatile uinteger24_t t24;

//-----------------------------------------------------------------------------
// Expose capture data to the outside world. Function returns:
//     Slow pulse  0x00FFFFFF 
//     No pulse    0xFFxxxxxx
//     Other value == pulse period
//     Most significant byte is always 0x00
//-----------------------------------------------------------------------------
uint32_t getPulsePeriod24(void) 
{
    uinteger32_t x;
    if (sparseEdgeMode)
    {
        if (c25ms >= C25MS_NO_PULSE_THRE)
        {
            //-------------------------------------
            x.bytes.C3 = UINT8_MAX; //-- "No pulse"
            //-------------------------------------
        }
        else
        { 
            //-------------------------------------
            x.value = 0x00FFFFFF;   // "Slow pulse"
            //-------------------------------------
        }
    }
    else
    {
        //-----------------------------------------------------------
        // t24 access suspends TMR1 gate interrupt. We can only do so
        // in non-"sparseEdgeMode".                         25Feb2020
        //-----------------------------------------------------------
        PIE3bits.TMR1GIE = 0;
        x.value = U24GETVALUE(t24);
        PIE3bits.TMR1GIE = 1;
        //-----------------------------------------------------------
        // The third most significant byte is the highest order 
        // value. It is obtained by attaching an extra byte 
        // 'tmr1Byte1' to the 16-bit timer TMR1 giving 24-bit 
        // resolution. Due to code construct, when this higher order 
        // byte is maximum, the lower order bytes carry on to wrap 
        // back and continue counting up. Therefore, when this third 
        // MSB is 0xFF, lower order bytes will not be valid. We must 
        // assume full value on them. 
        //-----------------------------------------------------------
        if (x.bytes.C2 == UINT8_MAX) {x.bytes.C0 = x.bytes.C1 = UINT8_MAX;}   
    }
    return x.value;
}

//-----------------------------------------------------------------------------
// TMR1 interrupt and TMR1 gate interrupts are enabled in non-"sparse edge 
// mode" and they are disabled in "sparse edge mode". Since TMR1 is gated,
// it may sometimes be paused to stop counting by the gate. 
//-----------------------------------------------------------------------------
void __section("edgeDetect") TMR1_ISR(void)
{
    PIR1bits.TMR1IF = 0;
    U8_INC_UNTIL_LIMIT_2(tmr1Byte2);
    return;
}

//-----------------------------------------------------------------------------
// On a gate event interrupt, Timer 1 registers has captured a pulse period. 
//-----------------------------------------------------------------------------
// Testing found that this runs every other cycle on square waves, but not
// every successive cycle.                                            24Feb2020
//-----------------------------------------------------------------------------
void __section("edgeDetect") TMR1_GATE_ISR(void)
{
    //---------------------------------------------------------------
    // Stop the timer (required by the MCU) to read and reload its 
    // values.
    //---------------------------------------------------------------
    T1CONbits.TMR1ON = 0;
    
    //---------------------------------------------------------------
    // Retrieve this new data. Timer 1 is set to 16-bit access. 
    // Reading TMR1L will snapshot TMR1H. Code statements below 
    // explicitly express the necessary read order. 
    //---------------------------------------------------------------
    t24.bytes.C0 = TMR1L;
    t24.bytes.C1 = TMR1H;
    t24.bytes.C2 = tmr1Byte2;

    //---------------------------------------------------------------
    // Timer 1 reload with zeroes to count up. Writing literal 0 to 
    // TMR1H and TMR1L but not non-zero values, as in the case of 
    // Microchip MIPS core, translate into less machine code. Lowest
    // possible ISR latency. 
    //---------------------------------------------------------------
    // Order of writing to Timer 1 by 16-bit access is the reverse of
    // reading. TMR1H first, then write to TMR1L will automatically 
    // simultaneously put the value of TMR1H in effect in the same
    // instruction cycle. 
    //---------------------------------------------------------------
    // If correction value is needed, it can be placed in the final
    // calculation.                                         22Feb2020
    //---------------------------------------------------------------
    TMR1H = 0;            // Write to "TMR1H buffer register"
    TMR1L = 0;            // This write will transfer TMR1H as well

    //---------------------------------------------------------------
    // Mode switching (gear shifting) uses two levels (hysteresis). 
    // To avoid costly uint32 arithmetic, the levels are hard coded.
    // Each tick on tmr1Byte2 is TMR1_OVERFLOW_US which is currently 
    // configured to 4096 us. "No trigger" region is 12 <= r <= 24.
    //---------------------------------------------------------------
    // Single Pulse mode: Clearing the TxGSPM bit of the TxGCON 
    // register will also clear the TxGGO/DONE bit. Continuous 
    // capture while this ISR is executing will be forgone. When in 
    // this mode, an OS task outside of this ISR triggers the 
    // T1GGO/DONE bit at a timed manner. This arrangement is to 
    // ensure at high pulse rate the ISR will only sample pulses
    // relatively infrequently to prevent repeat firing of ISR
    // eat up too much execution context. 
    //---------------------------------------------------------------
    if /**/ (tmr1Byte2 < 12) {T1GCONbits.T1GSPM = 1;}
    else if (tmr1Byte2 > 24) {T1GCONbits.T1GSPM = 0;}
    
    //---------------------------------------------------------------
    // Re-enable the timer to capture next pulse period.
    //---------------------------------------------------------------
    PIR3bits.TMR1GIF = 0;
    T1CONbits.TMR1ON = 1;

    //---------------------------------------------------------------
    // To obtain maximum precision, code not sensitive to TMR1 gate
    // interrupt is placed after TMR1 is turn back on. 
    //---------------------------------------------------------------    
    tmr1Byte2 = c25ms = 0;
    isFirstSampleAfterModeSwitching = false;

    return;
}

//-----------------------------------------------------------------------------
// CMP1 interrupt is enabled in "sparse edge mode" and it is disabled
// otherwise. CMP1_ISR() self-disables to avoid glitches. It is enabled by 
// 100 ms task. 
//-----------------------------------------------------------------------------
void __section("edgeDetect") CMP1_ISR(void) 
{
    static bool polarity;
    isFirstSampleAfterModeSwitching = false;
    if (polarity)
        { polarity = false; t0 = c25ms; }
    else
        { polarity = true;  t1 = c25ms; }
    c25ms = 0; 

    //---------------------------------------------------------------
    // Important: reading or writing CM1CON0 clears mismatch
    //            condition otherwise testing show this code cannot
    //            function properly. 
    //---------------------------------------------------------------
    CM1CON0bits.C1ON = 1;
    
    //---------------------------------------------------------------
    // This next line is for self-turning off to wait for a task to  
    // start the next sample. We use continuous mode to catch every 
    // other edge (of opposite polarity) therefore we do not turn
    // off the interrupt and let CMP1 capture every other edge. 
    //---------------------------------------------------------------
    //PIE2bits.C1IE = 1;
    
    PIR2bits.C1IF = 0;

    return;
}

//-----------------------------------------------------------------------------
// RAM reserved for strings to be composed and put to EUSART1
//-----------------------------------------------------------------------------
static char s0[12], s1[12], s2[12], s3[12];

char __section("edgeDetect") *cmpTrigVoltage(void)
{
    static char s[12];
    u32_a12_d((uint32_t)((CMP_V_THRES*CMP_PD_R_TOTAL/CMP_PD_R_DIVIDE+CMP_V_DIODE)*1000), 10-3, s);
    return s;
}

void __section("edgeDetect") realTimeReport_task(void)
{
    task_open();
    //---------------------------------------------------------------
    // Statements after task_open executes once so this is an ideal 
    // spot to insert initialization code lines. 
    //---------------------------------------------------------------
    U24ZEROIZE(t24);
    
    //---------------------------------------------------------------
    // Typical Co-RTOS task has an infinite loop within it lies at 
    // least one statement giving control back to OS. 
    //---------------------------------------------------------------
    for(;;) {
        task_wait( 1000 );
        if (printRealTimeData)
        {
            #define CORRECTION_VALUE (0)
            //-------------------------------------------------------
            // Code block below could interlace (may interfere with) 
            // printing in textTerm.c.
            //-------------------------------------------------------
            if (sparseEdgeMode)
            {
                //---------------------------------------------------
                // Note: power on startup is "Slow pulses" 
                //---------------------------------------------------
                if (c25ms >= C25MS_NO_PULSE_THRE)
                    { printa((char*)"No pulse\r\n\r\n\0"); }
                else
                    { printa((char*)"Slow pulses\r\n\r\n\0"); }
            }
            else
            {
                uinteger32_t f;
                //---------------------------------------------------
                // t24 access requires disabling interrupt  22Feb2020
                //---------------------------------------------------
                PIE3bits.TMR1GIE = 0;
                f.value = U24GETVALUE(t24);
                PIE3bits.TMR1GIE = 1;
                
                if (f.value)
                {
                    f.value += CORRECTION_VALUE;
                    char u[11];
                    
                    //-----------------------------------------------
                    // Hexadecimal output raw data. '3' == 24-bit
                    //-----------------------------------------------
                    trimLeft(u32_to_hex(f, 3), s0);
                    printa((char*)"Raw data  \0");
                    printa((char*)s0);
                    printa((char*)" ticks\r\n\0");

                    //-----------------------------------------------
                    // fire an event to u32Toa11_task() which 
                    // performs i2a conversion where after each digit
                    // returns control to OS scheduler to prevent 
                    // execution contention. 
                    //-----------------------------------------------
                    u32Operant = f.value;
                    event_signal( u32GoEvent );
                    event_wait( u32DoneEvent );

                    //-----------------------------------------------
                    // Remove leading white space
                    //-----------------------------------------------
                    trimLeft(u32string, s1);

                    //-----------------------------------------------
                    // Send to textTerm print_task() stream to EUSART
                    //-----------------------------------------------
                    printa((char*)"Decimal   \0");
                    printa((char*)s1);
                    printa((char*)" ticks\r\n\0");

                    //-----------------------------------------------
                    // Next output the equivalent micro-seconds. TMR
                    // is configured to FOSC/4 == 64/4 == 16 steps 
                    // per micro-second.
                    //-----------------------------------------------
                    u32Operant = f.value * 100 / 16;
                    //-----------------------------------------------
                    task_wait(1); //-- yield to let the event resolve
                    //-----------------------------------------------
                    event_signal( u32GoEvent );
                    event_wait( u32DoneEvent );

                    //-----------------------------------------------
                    // Add decimal point + remove leading white space
                    //-----------------------------------------------
                    insertAt(u32string, u, '.', 10-2);
                    trimLeft(u, s2);

                    //-----------------------------------------------
                    // Send to textTerm print_task() stream to EUSART
                    //-----------------------------------------------
                    printa((char*)"Interval  \0");
                    printa((char*)s2);
                    printa((char*)" us\r\n\0");
                    
                    u32Operant = (uint32_t)(100*1000000*16) / f.value;
                    //-----------------------------------------------
                    task_wait(1); //-- yield to let the event resolve
                    //-----------------------------------------------
                    event_signal( u32GoEvent );
                    event_wait( u32DoneEvent );

                    //-----------------------------------------------
                    // Add decimal point + remove leading white space
                    //-----------------------------------------------
                    insertAt(u32string, u, '.', 10-2);
                    trimLeft(u, s3);

                    //-----------------------------------------------
                    // Send to textTerm print_task() stream to EUSART
                    //-----------------------------------------------
                    printa((char*)"Frequency \0");
                    printa((char*)s3);
                    printa((char*)" Hz\r\n\r\n\0");
                }
                else
                {
                    printa((char*)"Overflow\r\n\r\n\0");
                }
            }
            #undef CORRECTION_VALUE
        }
    }
    //---------------------------------------------------------------
    // Control will never fall beyond this point
    //---------------------------------------------------------------
    task_close();
}

void __section("edgeDetect") senseTrigger_task(void)
{
    task_open();
    U24SETMAX(t24);
    c25ms = 50*4;
    for(;;)
    {
        static uint8_t j = 0;
        task_wait( 25 );
        
        //-----------------------------------------------------------
        // c100ms runs up to limit value at which point it cease to
        // increment and its value stay at the limit. The variable
        // is zeroed by multiple interrupts. Occasionally it might 
        // resist to zero amid the operation of the macro. The value
        // is checked before entering ISR, and after returning from
        // the ISR the original value get incremented and written 
        // back to c100ms missing the set zero operation. 
        //-----------------------------------------------------------
        U8_INC_UNTIL_LIMIT_2(c25ms);
        
        //-----------------------------------------------------------
        // This code file uses two Mode types combined to give three 
        // modes of operation (c.f. automotive gears). The higher  
        // level mode-switch is implemented here. 
        //-----------------------------------------------------------
        if (sparseEdgeMode && ((uint16_t)(t0+t1) < 10*4 - 2))
        {
            //-------------------------------------------------------
            // Turn off comparator 1 interrupt
            //-------------------------------------------------------
            PIE2bits.C1IE = 0;
            //-------------------------------------------------------
            // Set starting state 
            //-------------------------------------------------------
            T1GCONbits.T1GSPM = 1;
            TMR1H = 0;      //------ Write to "TMR1H buffer register"
            TMR1L = 0;      // This write will transfer TMR1H as well
            tmr1Byte2 = 0;
            c25ms = 0;
            U24SETMAX(t24);
            //-------------------------------------------------------
            // Change to non-sparseEdgeMode
            //-------------------------------------------------------
            sparseEdgeMode = false;
            isFirstSampleAfterModeSwitching = true;
            j = 3;
            //-------------------------------------------------------
            // Turn on timer 1 and its interrupt and gate interrupt
            //-------------------------------------------------------
            PIR1bits.TMR1IF = 0;
            PIR3bits.TMR1GIF = 0;
            T1CONbits.TMR1ON = 1;
            PIE1bits.TMR1IE = 1;
            T1GCONbits.TMR1GE = 1;
            PIE3bits.TMR1GIE = 1;
            //-------------------------------------------------------
            // Debugging code to ascertain state regularity 1/2
            //-------------------------------------------------------
            //#include "pin_manager.h"
            //LED_BLUE_SetHigh(); // High == OFF
        }
        //-----------------------------------------------------------
        // Value 20 due to sampling every other pulse period (found 
        // out this fact by actual testing where LED is blinked in 
        // the respective ISR)
        //-----------------------------------------------------------
        else if (!sparseEdgeMode && (((c25ms > 20*4+2) && !isFirstSampleAfterModeSwitching) || (c25ms > 40*4+2)))
        {
            //-------------------------------------------------------
            // Turn off timer 1 and its interrupt and gate interrupt
            //-------------------------------------------------------
            PIE1bits.TMR1IE = 0;
            T1CONbits.TMR1ON = 0;
            T1GCONbits.TMR1GE = 0;
            PIE3bits.TMR1GIE = 0;
            //-------------------------------------------------------
            // Set starting state
            //-------------------------------------------------------
            c25ms = 0;
            t0 = t1 = T0T1_INIT_VAL;
            //-------------------------------------------------------
            // Change to sparseEdgeMode
            //-------------------------------------------------------
            sparseEdgeMode = isFirstSampleAfterModeSwitching = true;
            j = 3;
            //-------------------------------------------------------
            // Debugging code to ascertain state regularity 2/2
            //-------------------------------------------------------
            //LED_BLUE_SetLow(); // Low == ON
        }
        
        //-----------------------------------------------------------
        // 100 millisecond section
        //-----------------------------------------------------------
        if (++j < 4) {continue;}
        j = 0;
        
        if (!sparseEdgeMode)
        {
            //-------------------------------------------------------
            // "Single-shot mode" for high pulse frequency: timer 1 
            // capture is stopped by hardware after every capture.
            // When this is the case, single-shot is re-triggered
            // by this code. All parameters are bits from T1GCONbits,
            // e.g. T1GCONbits.T1GSPM.
            //-------------------------------------------------------
            if (T1GSPM && !T1GGO_nDONE) {T1GGO=1;}
        }
        else
        {
            //-------------------------------------------------------
            // CMP1 interrupt is self-disabling. This is to prevent 
            // catching edge glitches causing frequent jumping out
            // of sparseEdgeMode. This interrupt is re-enabled by the
            // code below. 
            //-------------------------------------------------------
            // There are many issues working with CMP1 interrupt. The
            // solutions to these problems are remarked with notes. 
            //-------------------------------------------------------
            if (!PIE2bits.C1IE)
            {
                //---------------------------------------------------
                // "The comparator interrupt flag will be set 
                // whenever there is a change in the output value of 
                // the comparator". We set the initial mode for
                // faster response. 
                //---------------------------------------------------
                CM1CON0bits.C1POL = C1POL ? 0 : 1;
                //---------------------------------------------------
                // Bug fix: The comparator output "mismatch condition
                // will persist until the first latch value is update 
                // by performing a read of the CMxCON0 register". 
                // Testing showed that without the following code 
                // line the ISR keep being triggered. 
                //---------------------------------------------------
                CM1CON0bits.C1ON = 1;
                PIR2bits.C1IF = 0;
                PIE2bits.C1IE = 1;
            }
        }
    }
    task_close();
}

#undef C25MS_NO_PULSE_THRE
#undef T0T1_INIT_VAL

//----------------------------------------------------------------- end of file
