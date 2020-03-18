/**
  @Generated Pin Manager Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    pin_manager.h

  @Summary:
    This is the Pin Manager file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides APIs for driver for .
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.78.1
        Device            :  PIC18F26K22
        Driver Version    :  2.11
    The generated drivers are tested against the following:
        Compiler          :  XC8 2.10 and above
        MPLAB 	          :  MPLAB X 5.30	
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

/**
  Section: Included Files
*/

#include <xc.h>

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set channel_AN0 aliases
#define channel_AN0_TRIS                 TRISAbits.TRISA0
#define channel_AN0_LAT                  LATAbits.LATA0
#define channel_AN0_PORT                 PORTAbits.RA0
#define channel_AN0_ANS                  ANSELAbits.ANSA0
#define channel_AN0_SetHigh()            do { LATAbits.LATA0 = 1; } while(0)
#define channel_AN0_SetLow()             do { LATAbits.LATA0 = 0; } while(0)
#define channel_AN0_Toggle()             do { LATAbits.LATA0 = ~LATAbits.LATA0; } while(0)
#define channel_AN0_GetValue()           PORTAbits.RA0
#define channel_AN0_SetDigitalInput()    do { TRISAbits.TRISA0 = 1; } while(0)
#define channel_AN0_SetDigitalOutput()   do { TRISAbits.TRISA0 = 0; } while(0)
#define channel_AN0_SetAnalogMode()      do { ANSELAbits.ANSA0 = 1; } while(0)
#define channel_AN0_SetDigitalMode()     do { ANSELAbits.ANSA0 = 0; } while(0)

// get/set LED_RED aliases
#define LED_RED_TRIS                 TRISAbits.TRISA4
#define LED_RED_LAT                  LATAbits.LATA4
#define LED_RED_PORT                 PORTAbits.RA4
#define LED_RED_SetHigh()            do { LATAbits.LATA4 = 1; } while(0)
#define LED_RED_SetLow()             do { LATAbits.LATA4 = 0; } while(0)
#define LED_RED_Toggle()             do { LATAbits.LATA4 = ~LATAbits.LATA4; } while(0)
#define LED_RED_GetValue()           PORTAbits.RA4
#define LED_RED_SetDigitalInput()    do { TRISAbits.TRISA4 = 1; } while(0)
#define LED_RED_SetDigitalOutput()   do { TRISAbits.TRISA4 = 0; } while(0)

// get/set LED_BLUE aliases
#define LED_BLUE_TRIS                 TRISAbits.TRISA5
#define LED_BLUE_LAT                  LATAbits.LATA5
#define LED_BLUE_PORT                 PORTAbits.RA5
#define LED_BLUE_ANS                  ANSELAbits.ANSA5
#define LED_BLUE_SetHigh()            do { LATAbits.LATA5 = 1; } while(0)
#define LED_BLUE_SetLow()             do { LATAbits.LATA5 = 0; } while(0)
#define LED_BLUE_Toggle()             do { LATAbits.LATA5 = ~LATAbits.LATA5; } while(0)
#define LED_BLUE_GetValue()           PORTAbits.RA5
#define LED_BLUE_SetDigitalInput()    do { TRISAbits.TRISA5 = 1; } while(0)
#define LED_BLUE_SetDigitalOutput()   do { TRISAbits.TRISA5 = 0; } while(0)
#define LED_BLUE_SetAnalogMode()      do { ANSELAbits.ANSA5 = 1; } while(0)
#define LED_BLUE_SetDigitalMode()     do { ANSELAbits.ANSA5 = 0; } while(0)

// get/set OPTO1 aliases
#define OPTO1_TRIS                 TRISBbits.TRISB2
#define OPTO1_LAT                  LATBbits.LATB2
#define OPTO1_PORT                 PORTBbits.RB2
#define OPTO1_WPU                  WPUBbits.WPUB2
#define OPTO1_ANS                  ANSELBbits.ANSB2
#define OPTO1_SetHigh()            do { LATBbits.LATB2 = 1; } while(0)
#define OPTO1_SetLow()             do { LATBbits.LATB2 = 0; } while(0)
#define OPTO1_Toggle()             do { LATBbits.LATB2 = ~LATBbits.LATB2; } while(0)
#define OPTO1_GetValue()           PORTBbits.RB2
#define OPTO1_SetDigitalInput()    do { TRISBbits.TRISB2 = 1; } while(0)
#define OPTO1_SetDigitalOutput()   do { TRISBbits.TRISB2 = 0; } while(0)
#define OPTO1_SetPullup()          do { WPUBbits.WPUB2 = 1; } while(0)
#define OPTO1_ResetPullup()        do { WPUBbits.WPUB2 = 0; } while(0)
#define OPTO1_SetAnalogMode()      do { ANSELBbits.ANSB2 = 1; } while(0)
#define OPTO1_SetDigitalMode()     do { ANSELBbits.ANSB2 = 0; } while(0)

// get/set OPTO2 aliases
#define OPTO2_TRIS                 TRISBbits.TRISB4
#define OPTO2_LAT                  LATBbits.LATB4
#define OPTO2_PORT                 PORTBbits.RB4
#define OPTO2_WPU                  WPUBbits.WPUB4
#define OPTO2_ANS                  ANSELBbits.ANSB4
#define OPTO2_SetHigh()            do { LATBbits.LATB4 = 1; } while(0)
#define OPTO2_SetLow()             do { LATBbits.LATB4 = 0; } while(0)
#define OPTO2_Toggle()             do { LATBbits.LATB4 = ~LATBbits.LATB4; } while(0)
#define OPTO2_GetValue()           PORTBbits.RB4
#define OPTO2_SetDigitalInput()    do { TRISBbits.TRISB4 = 1; } while(0)
#define OPTO2_SetDigitalOutput()   do { TRISBbits.TRISB4 = 0; } while(0)
#define OPTO2_SetPullup()          do { WPUBbits.WPUB4 = 1; } while(0)
#define OPTO2_ResetPullup()        do { WPUBbits.WPUB4 = 0; } while(0)
#define OPTO2_SetAnalogMode()      do { ANSELBbits.ANSB4 = 1; } while(0)
#define OPTO2_SetDigitalMode()     do { ANSELBbits.ANSB4 = 0; } while(0)

// get/set RELAY aliases
#define RELAY_TRIS                 TRISCbits.TRISC0
#define RELAY_LAT                  LATCbits.LATC0
#define RELAY_PORT                 PORTCbits.RC0
#define RELAY_SetHigh()            do { LATCbits.LATC0 = 1; } while(0)
#define RELAY_SetLow()             do { LATCbits.LATC0 = 0; } while(0)
#define RELAY_Toggle()             do { LATCbits.LATC0 = ~LATCbits.LATC0; } while(0)
#define RELAY_GetValue()           PORTCbits.RC0
#define RELAY_SetDigitalInput()    do { TRISCbits.TRISC0 = 1; } while(0)
#define RELAY_SetDigitalOutput()   do { TRISCbits.TRISC0 = 0; } while(0)

// get/set RC1 procedures
#define RC1_SetHigh()            do { LATCbits.LATC1 = 1; } while(0)
#define RC1_SetLow()             do { LATCbits.LATC1 = 0; } while(0)
#define RC1_Toggle()             do { LATCbits.LATC1 = ~LATCbits.LATC1; } while(0)
#define RC1_GetValue()              PORTCbits.RC1
#define RC1_SetDigitalInput()    do { TRISCbits.TRISC1 = 1; } while(0)
#define RC1_SetDigitalOutput()   do { TRISCbits.TRISC1 = 0; } while(0)

// get/set OC1 aliases
#define OC1_TRIS                 TRISCbits.TRISC2
#define OC1_LAT                  LATCbits.LATC2
#define OC1_PORT                 PORTCbits.RC2
#define OC1_ANS                  ANSELCbits.ANSC2
#define OC1_SetHigh()            do { LATCbits.LATC2 = 1; } while(0)
#define OC1_SetLow()             do { LATCbits.LATC2 = 0; } while(0)
#define OC1_Toggle()             do { LATCbits.LATC2 = ~LATCbits.LATC2; } while(0)
#define OC1_GetValue()           PORTCbits.RC2
#define OC1_SetDigitalInput()    do { TRISCbits.TRISC2 = 1; } while(0)
#define OC1_SetDigitalOutput()   do { TRISCbits.TRISC2 = 0; } while(0)
#define OC1_SetAnalogMode()      do { ANSELCbits.ANSC2 = 1; } while(0)
#define OC1_SetDigitalMode()     do { ANSELCbits.ANSC2 = 0; } while(0)

// get/set IO_RC3 aliases
#define IO_RC3_TRIS                 TRISCbits.TRISC3
#define IO_RC3_LAT                  LATCbits.LATC3
#define IO_RC3_PORT                 PORTCbits.RC3
#define IO_RC3_ANS                  ANSELCbits.ANSC3
#define IO_RC3_SetHigh()            do { LATCbits.LATC3 = 1; } while(0)
#define IO_RC3_SetLow()             do { LATCbits.LATC3 = 0; } while(0)
#define IO_RC3_Toggle()             do { LATCbits.LATC3 = ~LATCbits.LATC3; } while(0)
#define IO_RC3_GetValue()           PORTCbits.RC3
#define IO_RC3_SetDigitalInput()    do { TRISCbits.TRISC3 = 1; } while(0)
#define IO_RC3_SetDigitalOutput()   do { TRISCbits.TRISC3 = 0; } while(0)
#define IO_RC3_SetAnalogMode()      do { ANSELCbits.ANSC3 = 1; } while(0)
#define IO_RC3_SetDigitalMode()     do { ANSELCbits.ANSC3 = 0; } while(0)

// get/set IO_RC4 aliases
#define IO_RC4_TRIS                 TRISCbits.TRISC4
#define IO_RC4_LAT                  LATCbits.LATC4
#define IO_RC4_PORT                 PORTCbits.RC4
#define IO_RC4_ANS                  ANSELCbits.ANSC4
#define IO_RC4_SetHigh()            do { LATCbits.LATC4 = 1; } while(0)
#define IO_RC4_SetLow()             do { LATCbits.LATC4 = 0; } while(0)
#define IO_RC4_Toggle()             do { LATCbits.LATC4 = ~LATCbits.LATC4; } while(0)
#define IO_RC4_GetValue()           PORTCbits.RC4
#define IO_RC4_SetDigitalInput()    do { TRISCbits.TRISC4 = 1; } while(0)
#define IO_RC4_SetDigitalOutput()   do { TRISCbits.TRISC4 = 0; } while(0)
#define IO_RC4_SetAnalogMode()      do { ANSELCbits.ANSC4 = 1; } while(0)
#define IO_RC4_SetDigitalMode()     do { ANSELCbits.ANSC4 = 0; } while(0)

// get/set RC6 procedures
#define RC6_SetHigh()            do { LATCbits.LATC6 = 1; } while(0)
#define RC6_SetLow()             do { LATCbits.LATC6 = 0; } while(0)
#define RC6_Toggle()             do { LATCbits.LATC6 = ~LATCbits.LATC6; } while(0)
#define RC6_GetValue()              PORTCbits.RC6
#define RC6_SetDigitalInput()    do { TRISCbits.TRISC6 = 1; } while(0)
#define RC6_SetDigitalOutput()   do { TRISCbits.TRISC6 = 0; } while(0)
#define RC6_SetAnalogMode()         do { ANSELCbits.ANSC6 = 1; } while(0)
#define RC6_SetDigitalMode()        do { ANSELCbits.ANSC6 = 0; } while(0)

// get/set IO_RC7 aliases
#define IO_RC7_TRIS                 TRISCbits.TRISC7
#define IO_RC7_LAT                  LATCbits.LATC7
#define IO_RC7_PORT                 PORTCbits.RC7
#define IO_RC7_ANS                  ANSELCbits.ANSC7
#define IO_RC7_SetHigh()            do { LATCbits.LATC7 = 1; } while(0)
#define IO_RC7_SetLow()             do { LATCbits.LATC7 = 0; } while(0)
#define IO_RC7_Toggle()             do { LATCbits.LATC7 = ~LATCbits.LATC7; } while(0)
#define IO_RC7_GetValue()           PORTCbits.RC7
#define IO_RC7_SetDigitalInput()    do { TRISCbits.TRISC7 = 1; } while(0)
#define IO_RC7_SetDigitalOutput()   do { TRISCbits.TRISC7 = 0; } while(0)
#define IO_RC7_SetAnalogMode()      do { ANSELCbits.ANSC7 = 1; } while(0)
#define IO_RC7_SetDigitalMode()     do { ANSELCbits.ANSC7 = 0; } while(0)

/**
   @Param
    none
   @Returns
    none
   @Description
    GPIO and peripheral I/O initialization
   @Example
    PIN_MANAGER_Initialize();
 */
void PIN_MANAGER_Initialize (void);

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handling routine
 * @Example
    PIN_MANAGER_IOC();
 */
void PIN_MANAGER_IOC(void);



#endif // PIN_MANAGER_H
/**
 End of File
*/