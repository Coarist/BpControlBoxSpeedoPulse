//-----------------------------------------------------------------------------
// File:   textTerm.h
// Author: chi
//
// Created on February 12, 2020, 08:40
//-----------------------------------------------------------------------------
// EUSART1 text terminal service (console)
//-----------------------------------------------------------------------------
// Possibly due to in-line resistors and line characteristics, testing on 
// bench with short 30cm wire link saw occasional incorrect loop back
// characters. Frequency of occurrence is not negligible. The serial port baud
// rate is dropped one order from 115200 to 57600 (factor of 2), error is 
// eliminated. To allow for longer physical link, baud rate is further dropped 
// to 38400 (factor of 1.5 x 2 = 3).                                  11Feb2020
//-----------------------------------------------------------------------------
// Important: backspace character is not supported. Hitting backspace corrects
//            the typing on the connected text terminal but the parser treat
//            the re-typed characters as part of the string.          17Feb2020
//-----------------------------------------------------------------------------
// MCC generated EUSART1 code usage
//     EUSART1_Write() must check EUSART1_is_tx_ready() == true beforehand
//     EUSART1_Read()  must check EUSART1_is_rx_ready() == true beforehand
//-----------------------------------------------------------------------------

#include "textTerm.h"
#include "cocoos.h"
#include "eusart1.h"
#include "EULA.h"
#include "helpText.h"
#include "stdbool.h"
#include "string.h"
#include "edgeDetect.h"
#include "opParam.h"
#include "alarm.h"
#include "i2a.h"

//-----------------------------------------------------------------------------
// printa() implementation to queue const strings by their starting address
//-----------------------------------------------------------------------------
#define PRINTA_BUFFER_SIZE (32)
static uint8_t printaHead;
static uint8_t printaTail;
static uint8_t printaBufferRemaining;
static char* printaBuf[PRINTA_BUFFER_SIZE];

//-----------------------------------------------------------------------------
// parse input strings from EUSART1 as part of the text terminal service
//-----------------------------------------------------------------------------
#define PARSE_BUFFER_SIZE (32)
static uint8_t charCont;
static char parseBuf[PARSE_BUFFER_SIZE];

//-----------------------------------------------------------------------------
// printa()
// Input:  the starting address of a string which is to be shifted out to the 
//         EUSART1. The address is put on the local buffer. The string content
//         should not be changed before it has a chance to get shifted out. 
//         The implementation is particularly suited for ROM/FLASH strings. 
//         *a is expected to be a valid non-NULL address. 
// Output: true  == success
//         false == buffer is full, the address of the string is not cloned 
//                  to the local string pointer buffer.
//-----------------------------------------------------------------------------
bool __section("textTerm") printa(char* a)
{
    if (!printaBufferRemaining) {return false;}
    printaBuf[printaHead++] = a;
    if(PRINTA_BUFFER_SIZE <= printaHead) {printaHead = 0;}
    printaBufferRemaining--;
    return true;
}

//-----------------------------------------------------------------------------
// print_task()
// Shifted out strings (whose address in in the FIFO buffer) to EUSART1. The 
// string contents should not be changed until after they have been shifted out
// (but there is no facility to enquire this info). The implementation is 
// particularly suited for ROM/FLASH strings. 
//-----------------------------------------------------------------------------
void __section("textTerm") print_task(void)
{
    static char* a;
    task_open();
    //---------------------------------------------------------------
    // Initialization code after task_open() will only execute once. 
    //---------------------------------------------------------------
    printaHead = printaTail = 0;
    printaBufferRemaining = PRINTA_BUFFER_SIZE;
    //---------------------------------------------------------------
    // Infinite task loop
    //---------------------------------------------------------------
    for(;;) {
        if (PRINTA_BUFFER_SIZE > printaBufferRemaining)
        {
            //---------------------------------
            // Obtain the pointer to the string
            //---------------------------------
            a = printaBuf[printaTail++];
            //---------------------------------
            // Update FIFO buffer state
            //---------------------------------
            if(PRINTA_BUFFER_SIZE <= printaTail) {printaTail = 0;}
            printaBufferRemaining++;
            //---------------------------------
            // Stream the string to EUSART
            //---------------------------------
            if (a != NULL)
            {
                while (*a != '\0')
                {
                    while (!EUSART1_is_tx_ready()) {task_wait(5);}
                    EUSART1_Write(*a++);
                }
            }
        }
        task_wait(10);
    }
    task_close(); //--------- control will never fall onto this point
}

//-----------------------------------------------------------------------------
// textTerminal_task()
// Text parser to provide minimal text terminal service to EUSART1. At present, 
// OS ticks at 1 ms interval. @38400 baud this is 38.4 characters for each
// task_wait(1).
//-----------------------------------------------------------------------------
// Code construct to prevent string loss
//     while (!printa(testToPrint)) {task_wait(1);}
// within the outer for(;;) inside switch() hangs the task. This is possibly a
// bug in cocoOS where such construct did not resolve correctly inside nested 
// structure. The code is reverted to:
//     printa(testToPrint);
// If the output buffer(s) is full the printed text would simply be lose. A 
// desirable effect of doing so is much smaller code size -898 bytes. 17Feb2020
//-----------------------------------------------------------------------------
void __section("textTerm") textTerminal_task(void)
{
    uint8_t a;
    task_open();
    charCont = 0;
    printa((char*)welcomeText); 
    printa((char*)promptText);
    for(;;) {
        while (!EUSART1_is_rx_ready()) {task_wait(20);}
        a = EUSART1_Read();
        while (!EUSART1_is_tx_ready()) {task_wait(5);}
        EUSART1_Write(a);
        if (a == '\r' || a == '\n') 
        {
        //----------------------------------------------------------------
        // CR / LF : the input string collected from previous iterations 
        //           will be parsed as a command. 
        //----------------------------------------------------------------
            if (charCont > 0) 
            {
                //--------------------------------------------------------
                // trim leading spaces. i => index of first non-space char
                //--------------------------------------------------------
                uint8_t i = 0;
                while ((i < charCont) && (parseBuf[i] == ' ')) {++i;}
                
                //--------------------------------------------------------
                // trim trailing spaces. j => index first space after 
                // string body
                //--------------------------------------------------------
                uint8_t j = charCont;
                while ((j > i) && (parseBuf[j-1] == ' ')) {--j;}
                
                //--------------------------------------------------------
                // Use string length as preliminary check if command is 
                // valid. If so execute the identified command. 
                //--------------------------------------------------------
                bool isCommandValid = false;
                switch (j-i)
                {
                    case 0:
                        isCommandValid = true; // empty input is pardoned
                        break;
                    case 1:
                        if (parseBuf[i] == '?') 
                        {
                            printa((char*)helpText);
                            isCommandValid = true;
                        }
                        break;
                    case 4:
                        if (parseBuf[i+0] == 'h' && parseBuf[i+1] == 'e'
                         && parseBuf[i+2] == 'l' && parseBuf[i+3] == 'p')
                        {
                            printa((char*)helpText);
                            isCommandValid = true;
                        }
                        else if (parseBuf[i+0] == 'e' && parseBuf[i+1] == 'u'
                              && parseBuf[i+2] == 'l' && parseBuf[i+3] == 'a')
                        {
                            printa((char*)eulaHeadText);
                            printa((char*)cocoOSLicenseText);
                            isCommandValid = true;
                        }
                        else if (parseBuf[i+0] == 'c' && parseBuf[i+1] == 'm'
                              && parseBuf[i+2] == 'p' && parseBuf[i+3] == 's')
                        {
                            printa((char*)"\r\nTrigger at \0");
                            printa(cmpTrigVoltage());
                            printa((char*)" volts\r\n\0");
                            isCommandValid = true;
                        }
                        else if (parseBuf[i+0] == 's' && parseBuf[i+1] == 'y'
                              && parseBuf[i+2] == 's' && parseBuf[i+3] == 'i')
                        {
                            printa((char*)devConfigText0);
                            
                            printa((char*)"Pre- alarm \0");
                            printa(pAlarmHexString());
                            printa((char*)"\r\n\0");
                            
                            printa((char*)"Main alarm \0");
                            printa(mAlarmHexString());
                            printa((char*)"\r\n\0");
                            
                            printa((char*)"Sampling interval \0");
                            printa(mIntervalDecString());
                            printa((char*)" x10ms\r\n\0");
                            
                            printa((char*)"Hi-Lo threshold \0");
                            printa(cmpTrigVoltage());
                            printa((char*)" volt(s)\r\n\r\n\0");
                            
                            isCommandValid = true;    
                        }
                        else if (parseBuf[i+0] == 'p' && parseBuf[i+1] == 'o'
                              && parseBuf[i+2] == 'd')
                        {
                            switch(parseBuf[i+3])
                            {
                                case '0':
                                    printa((char*)rtDataOffText);
                                    printRealTimeData = false;
                                    isCommandValid = true;    
                                    break;
                                case '1':
                                    printa((char*)rtDataOnText);
                                    printRealTimeData = true;
                                    isCommandValid = true;
                                    break;
                                default:
                                    break;
                            }
                        }
                        break;
                    case 7:
                        if (parseBuf[i+0] == 'a' && parseBuf[i+1] == 'l'
                         && parseBuf[i+2] == 'm' && parseBuf[i+3] == 's'
                         && parseBuf[i+4] == 'e' && parseBuf[i+5] == 't')
                        {
                            switch(parseBuf[i+6])
                            {
                                case '0':
                                    opZeroAllAlarmLevels();
                                    printa((char*)"\r\nAlarm levels zero-ed\r\n\0");
                                    isCommandValid = true;    
                                    break;    
                                case '1':
                                    printa((char*)"\r\nSet pre- alarm \0");
                                    opSetPre_AlarmFromCapture();
                                    printa(pAlarmHexString());
                                    printa((char*)"\r\n\0");
                                    isCommandValid = true;    
                                    break;
                                case '2':
                                    printa((char*)"\r\nSet main alarm \0");
                                    opSetMainAlarmFromCapture();
                                    printa(mAlarmHexString());
                                    printa((char*)"\r\n\0");
                                    isCommandValid = true;    
                                    break;
                                default:
                                    break;
                            }
                        }
                    default:
                        //------------------------------------------------
                        //aleve1 alevel2
                        //------------------------------------------------
                        if (parseBuf[i+0] == 'a' && parseBuf[i+1] == 'l'
                         && parseBuf[i+2] == 'e' && parseBuf[i+3] == 'v'
                         && parseBuf[i+4] == 'e' && parseBuf[i+5] == 'l')
                        {
                            if((parseBuf[i+6]!='1') && (parseBuf[i+6]!='2'))
                                { break; }
                            
                            uint32_t v;
                            bool isValid = false;
                            
                            if ((j-i)>7) 
                            {
                                isValid = hexStringtoi32(&parseBuf[i+7], j-i-7, &v);
                                if (!isValid) 
                                    {isValid = decStringtoi32(&parseBuf[i+7], j-i-7, &v);}
                            }
                            
                            if (!isValid || v > 0xFFFFFF) 
                            {
                                printa((char*)"\r\nInvalid Value\r\n\0"); 
                                isCommandValid = true; 
                                break;
                            }

                            switch (parseBuf[i+6])
                            {
                                case '1':
                                    printa((char*)"\r\nSet pre- alarm \0");
                                    opSetPre_AlarmByValue(v);
                                    printa(pAlarmHexString());
                                    printa((char*)"\r\n\0");
                                    isCommandValid = true;    
                                    break;
                                case '2':
                                    printa((char*)"\r\nSet main alarm \0");
                                    opSetMainAlarmByValue(v);
                                    printa(mAlarmHexString());
                                    printa((char*)"\r\n\0");
                                    isCommandValid = true;    
                                    break;
                                default:
                                    break;
                            }
                            //--------------------------------------------
                            // When program control flow after this block, 
                            // if isCommandValid is 'false' a message 
                            // corresponding to this issue will be emitted
                            // to the ESUART to appropriately notify the
                            // user. 
                            //--------------------------------------------
                        }
                        //------------------------------------------------
                        //volthre
                        //------------------------------------------------
                        else if (parseBuf[i+0] == 'v' && parseBuf[i+1] == 'o'
                         && parseBuf[i+2] == 'l' && parseBuf[i+3] == 't'
                         && parseBuf[i+4] == 'h' && parseBuf[i+5] == 'r'
                         && parseBuf[i+6] == 'e')
                        {
                            uint32_t v;
                            bool isValid = false;
                            if ((j-i)>7) 
                            {
                                isValid = hexStringtoi32(&parseBuf[i+7], j-i-7, &v);
                                if (!isValid) 
                                    {isValid = decStringtoi32(&parseBuf[i+7], j-i-7, &v);}
                            }
                            
                            if (!isValid || v > UINT8_MAX) 
                                {printa((char*)"\r\nInvalid Value\r\n\0");}
                            else   
                            {
                                static char s[11];
                                printa((char*)"\r\nHi-Lo threshold \0");
                                opSetCmpVoltThresholdByValue((uint8_t)(v & 0xFF));
                                printa(cmpTrigVoltage());
                                printa((char*)" volts\r\n\0");
                            }
                            
                            isCommandValid = true;    
                        }
                        //------------------------------------------------
                        //samplei
                        //------------------------------------------------
                        else if (parseBuf[i+0] == 's' && parseBuf[i+1] == 'a'
                         && parseBuf[i+2] == 'm' && parseBuf[i+3] == 'p'
                         && parseBuf[i+4] == 'l' && parseBuf[i+5] == 'e'
                         && parseBuf[i+6] == 'i')
                        {
                            uint32_t v;
                            bool isValid = false;
                            if ((j-i)>7) 
                            {
                                isValid = hexStringtoi32(&parseBuf[i+7], j-i-7, &v);
                                if (!isValid) 
                                    {isValid = decStringtoi32(&parseBuf[i+7], j-i-7, &v);};
                            }
                            
                            if (!isValid || v > UINT8_MAX) 
                                {printa((char*)"\r\nInvalid Value\r\n\0");}
                            else   
                            {
                                static char s[11];
                                printa((char*)"\r\nSampling interval \0");
                                opSetAlarmSamplingInterval((uint8_t)(v & 0xFF));
                                printa(mIntervalDecString());
                                printa((char*)" x10ms\r\n\0");
                            }
                            
                            isCommandValid = true;    
                        }
                        break;
                }
                //--------------------------------------------------------
                // Prompt user with help suggestions
                //--------------------------------------------------------
                if (isCommandValid == false)
                {
                    printa((char*)parseErrText);
                }
            }
            printa((char*)"\r\n\0");
            //------------------------------------------------------------
            // The string is consumed. Need to be cleared.
            //------------------------------------------------------------
            charCont = 0;
            //------------------------------------------------------------
            // Display / output a fresh prompt to the user
            //------------------------------------------------------------
            printa((char*)promptText);
        }
        //----------------------------------------------------------------
        // Support for ASCII 0x08 "BS" backspace and 'DEL'       28FEb2020
        //----------------------------------------------------------------
        else if (a == '\b' || a == 127) 
        {
            if (charCont > 0) {--charCont;}
        }
        else
        //----------------------------------------------------------------
        // All non-CR non-LF input characters are copied to the input 
        // buffer await further processing (await CR LF) up to the point
        // that the buffer is completely filled. Excess characters will be
        // forgone. They will be lost. 
        //----------------------------------------------------------------
        {
            if (charCont < PARSE_BUFFER_SIZE) {parseBuf[charCont++] = a;}
        }
    }
    task_close(); //--------- control will never fall onto this point
}

#undef PRINTA_BUFFER_SIZE
#undef PARSE_BUFFER_SIZE

//----------------------------------------------------------------- end of file
