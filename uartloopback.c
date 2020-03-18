

//-----------------------------------------------------------------------------
// Possibly due to in-line resistors and line characteristics, testing on 
// bench with short 30cm wire link saw occasional incorrect loop back
// characters. Frequency of occurrence is not negligible. The serial port baud
// rate is dropped one order from 115200 to 57600 (factor of 2), error is 
// eliminated. To allow for longer physical link, baud rate is further dropped 
// to 38400 (factor of 1.5 x 2 = 3).                                  11Feb2020
//-----------------------------------------------------------------------------

#include "cocoos.h"
#include "eusart1.h"
#include "EULA.h"
#include "stdbool.h"

//-----------------------------------------------------------------------------
// showEULA()
//     Stream the End User License Agreement text to EUSART1
// Function return value:
//     true  : completed. The full EULA text has been shifted out tp the serial
//             port EUSART1.
//     false : yet to be completed - serial buffer is filled up. Wait some time 
//             for the buffer content to be shifted out to EUSART1-TX. Call  
//             this function again and check function return value. Repeat  
//             until 'true' is returned. 
//-----------------------------------------------------------------------------
static bool showEULA0(void)
{
    static char* a = cocoOSLicenseText;
    while (*a != '\0')
    {
        if (!EUSART1_is_tx_ready()) {return false;}
        EUSART1_Write(*a++);
    }
    a = cocoOSLicenseText;
    return true;
}


void __section("loopback") uartloopback_task(void)
{
    task_open();
    
    //--------------------------------------------------------------
    // Demonstrate streaming EULA to EUSART1 Success       11Feb2020
    //--------------------------------------------------------------
    while (!showEULA0()) task_wait( 2 ); 
    
    for(;;) {
        //----------------------------------------------------------
        // loop back (testing) Success                     11Feb2020
        //----------------------------------------------------------
        while (EUSART1_is_tx_ready() && EUSART1_is_rx_ready())
        {
            EUSART1_Write(EUSART1_Read());
        }
        //if (EUSART1_is_tx_ready())
        //{
        //    EUSART1_Write('z');
        //}
        //task_wait( 250 );
        //task_wait( 250 );
        //task_wait( 250 );
        task_wait( 1 );
    }
    task_close(); //--------- control will never fall onto this point
}

//----------------------------------------------------------------- end of file
