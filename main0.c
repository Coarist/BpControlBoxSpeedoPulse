/**
  This main source file is written by adding to code template generated by
  the Microchip Code Configurator MCC.

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Description:
    This header file provides implementations for driver APIs for all modules 
    selected in the GUI.

    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.78.1
        Device            :  PIC18F26K22
        Driver Version    :  2.00
*/

#include "xc.h"
#include "mcc.h"
#include "cocoos.h"
#include "blinky.h"
#include "adcPulseSensing.h"
#include "textTerm.h"

//-----------------------------------------------------------------------------
// Notes: ICD reset usually occurs multiple times in succession.      11Feb2020
//-----------------------------------------------------------------------------

void main(void)
{
    //-------------------------------------------------------------------------
    // Initialization generally ordering lowest level (near hardware) first   
    // progressing towards highest level (application). 
    //-------------------------------------------------------------------------
    SYSTEM_Initialize(); //----------- MCC Microchip Code Configurator
    TMR0_SetInterruptHandler(os_tick); //--- Allocate timer to OS tick
    ADC_SetInterruptHandler(adcDataHandler);
    os_init(); //----------------- Initialize OS before creating tasks
    SWDTEN=0;
    
    //--------------------------------------------------------------c -----------
    // Create kernel objects
    //-------------------------------------------------------------------------
    // Third argument is task priority. Important:
    //     (1) 0..255, the smaller the number the higher the priority
    //     (2) No two tasks is to be assigned the same priority 
    //     (3) Even when scheduling scheme is "round-robin" (1) and (2) still 
    //         apply.
    // Tests the above if not observed the MCU will hang.
    //-------------------------------------------------------------------------
    task_create( adc_task,          NULL, 127, NULL, 0, 0 );
    task_create( blinky_task,       NULL, 128, NULL, 0, 0 );
    task_create( print_task,        NULL, 129, NULL, 0, 0 );
    task_create( textTerminal_task, NULL, 130, NULL, 0, 0 );
    
    //-------------------------------------------------------------------------
    // If using interrupts in PIC18 High/Low Priority Mode, enable the Global 
    // High and Low Interrupts
    //-------------------------------------------------------------------------
    // If using interrupts in PIC Mid-Range Compatibility Mode, enable the 
    // Global and Peripheral Interrupts
    //-------------------------------------------------------------------------

    // Enable high priority global interrupts
    INTERRUPT_GlobalInterruptHighEnable();

    // Enable low priority global interrupts.
    INTERRUPT_GlobalInterruptLowEnable();

    // Disable high priority global interrupts
    //INTERRUPT_GlobalInterruptHighDisable();

    // Disable low priority global interrupts.
    //INTERRUPT_GlobalInterruptLowDisable();

    // Enable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptEnable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

    //INTERRUPT_PeripheralInterruptEnable();
    //INTERRUPT_PeripheralInterruptDisable();

    //-------------------------------------------------------------------------
    // Entering the application (configuration and initialization are done)
    //-------------------------------------------------------------------------
    os_start();

    //-------------------------------------------------------------------------
    // Program execution (control flow) is expected to never fall to this 
    // code section since the OS is an infinite loop in nature. 
    //-------------------------------------------------------------------------
    return;
}
/**
 End of File
*/