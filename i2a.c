//-----------------------------------------------------------------------------
// File:   i2a.c
// Author: chi
//
// Created on February 11, 2020, 16:39
//-----------------------------------------------------------------------------
// Value to text conversion utility function and associated string manipulation
// function - attempting to be leaner than standard sprintf(). If during code
// development further sophistication is needed, this might be forgone and use
// sprintf() instead. )
//-----------------------------------------------------------------------------

#include "i2a.h"
#include "string.h"
#include "stdint.h"
#include "cocoos.h"
#include "stdbool.h"

//-----------------------------------------------------------------------------
// u32Toa11_task() : 32-bit unsigned integer convert to text representation
//-----------------------------------------------------------------------------
// How to use, steps: 
//    (1) Load operant u32Numerator
//    (2) event_signal( u32GoEvent );
//    (3) event_wait( u32DoneEvent );
//    (4) Obtain output u32string[]
// i2a notes: 
//    32-bit corresponds to "4 giga" requiring 10 digits. 
//    The output string u32string[] will be at most 10 + 1 = 11 bytes long. 
//    Output string is right-justified
//    Leading digit(s) is(are) patched with space ' ' character.
//    Retain value until the next u32GoEvent to set off computation. 
//-----------------------------------------------------------------------------

#define LENSZE (11)
Evt_t u32GoEvent, u32DoneEvent;
uint32_t u32Operant;
char u32string[LENSZE];
void __section("i2a") u32Toa11_task(void)
{
    task_open();
    for(;;) {
        static uint8_t i;
        event_wait( u32GoEvent );
        
        //-----------------------------------------------------------
        // String termination character after right-most digit
        //-----------------------------------------------------------
        // Compose string backward, because of two observations:
        //   (1) String is of fixed, known length
        //   (2) This way the arithmetic is much more elegant
        //-----------------------------------------------------------
        // Least significant to most significant digit, right-to-left
        // right-justified
        //-----------------------------------------------------------
        i = LENSZE;
        u32string[--i] = '\0';
        
        //-----------------------------------------------------------
        // Have to use "goto". Placing code block inside switch() 
        // will result in this task permanently blocked (hung). 
        // It looks like cocoOS doesn't like having task_wait() 
        // nested too deeply.                               27Feb2020
        //-----------------------------------------------------------
        //switch (u32ConvOp)
        //{
        //    case U32OP_I2A_32:  goto U32LABEL0; break;
        //    case U32OP_I2H_24:  goto U32LABEL1; break;
        //    case U32OP_I2H_32:  goto U32LABEL2; break;
        //    default:            goto U32LABELY; break;
        //}
        //-----------------------------------------------------------
        
        do {
            u32string[--i] = '0'+(uint8_t)(u32Operant%10);
            u32Operant /= 10;
            task_wait(1); //------------- make system more responsive
        } 
        while (u32Operant);

        //-----------------------------------------------------------
        // If there are unused left digit(s) patch blank space(s)
        //-----------------------------------------------------------
        while (i) { u32string[--i] = ' '; }
        
        event_signal( u32DoneEvent );
    }
    task_close(); //------- Control will never fall beyond this point
}
#undef LENSZE

//-----------------------------------------------------------------------------
// u32_to_a11() : 32-bit unsigned integer to text representation
//    x : input value to be converted to decimal text numerical representation.
//        32-bit corresponds to "4 giga" requiring 10 digits. The output string 
//        will be 10 + 1 = 11 bytes long.
//    Function return value: a pointer to a string of length 11 which retains 
//        its value until the next call to this function. The caller may use 
//        this for further processing, and eventually perhaps have it copied 
//        to another RAM location. Output string is right-justified, unused
//        digit(s) is filled with space ' ' character.
//-----------------------------------------------------------------------------
char __section("i2a") *u32_to_a11(uint32_t x)
{
    #define LENSZE (11)
    static char a[LENSZE];
    uint8_t i = LENSZE;
    //---------------------------------------------------------------
    // Compose string backward, because of two characteristics:
    //   (1) String is of fixed, known length
    //   (2) This way the arithmetic is much more elegant (efficient)
    //---------------------------------------------------------------
    // String termination character after right-most digit
    //---------------------------------------------------------------
    a[--i] = '\0';
    //---------------------------------------------------------------
    // Least significant to most significant digit, right-to-left
    //---------------------------------------------------------------
     do {
        a[--i] = '0'+(uint8_t)(x%10);
        x /= 10;
    } 
    while (x);
    //---------------------------------------------------------------
    // If there are unused left digit(s) patch with space character
    //---------------------------------------------------------------
    while (i) { a[--i] = ' '; }
    return a;
    #undef LENSZE
}

//-----------------------------------------------------------------------------
// u32_to_hex() : a variant of *u32_to_a11(). Function call argument j is the 
// number of lease significant bytes to be converted. For example
//     4 = convert 4 bytes to 0x******** 10-characters
//     3 = convert lowest 3 bytes to 0x****** patch left with spaces
//-----------------------------------------------------------------------------
char __section("i2a") *u32_to_hex(uinteger32_t x, uint8_t j)
{
    #define LENSZE (11)
    static char a[LENSZE]; //-------- declare static to persist value
    uint8_t i;
    for (i = 0; i < LENSZE; ++i) {a[i] = ' ';}
    a[10] = '\0';
    i = 8 ;
    if (j >= 1)
    {
        a[9] = lowewrNibbleToAscii(x.bytes.C0);
        a[8] = lowewrNibbleToAscii(x.bytes.C0 >> 4);
        i -= 2;
    }
    if (j >= 2)
    {
        a[7] = lowewrNibbleToAscii(x.bytes.C1);
        a[6] = lowewrNibbleToAscii(x.bytes.C1 >> 4);
        i -= 2;
    }
    if (j >= 3)
    {
        a[5] = lowewrNibbleToAscii(x.bytes.C2);
        a[4] = lowewrNibbleToAscii(x.bytes.C2 >> 4);
        i -= 2;
    }
    if (j >= 4)
    {
        a[3] = lowewrNibbleToAscii(x.bytes.C3);
        a[2] = lowewrNibbleToAscii(x.bytes.C3 >> 4);
        a[1] = 'x'; a[0] = '0';
        i -= 2;
    }
    a[i] = '0'; a[i+1] = 'x';
    return a;
    #undef LENSZE
}

//-----------------------------------------------------------------------------
// trimRight() : remove trailing spaces in string *a which is done by simply
//    inserting an extra '\0' upon finding non-space character from the right. 
//    The processing loop terminates by encountering an end of string character 
//    '\0'. Invalid string can cause indefinite execution and RAM corruption. 
//-----------------------------------------------------------------------------
void __section("i2a") trimRight(char *a)
{
    uint8_t i = 0;
    while (a[i++] != '\0');         //--- go to the end of string
    while (i > 0 && a[--i] == ' '); //--- back to first non-space
    a[++i] = '\0';                  // put '\0' immediately after
    return;
}

//-----------------------------------------------------------------------------
// trimLeft() : remove leading spaces in string a, output to string b. 
//    Calling function must ensure b[] has pre-allocated enough RAM to hold the
//    result. size of array b must be >= size of array a. The processing loop
//    terminates by encountering an end of string character '\0'. Invalid
//    string can cause indefinite execution and RAM corruption. 
//-----------------------------------------------------------------------------
void __section("i2a") trimLeft(char *a, char *b)
{
    while (*a == ' ') { ++a; }     //-- move to first non-space
    while ((*b++ = *a++) != '\0'); //---------- copy until '\0'
    return;
}

//-----------------------------------------------------------------------------
// insertAt() : insert given character at position specified by the caller
//    Calling function must ensure b[] has pre-allocated enough RAM to hold the
//    result. size of array b must be >= size of array a + 1. The processing 
//    loop terminates by encountering an end of string character '\0'. Invalid
//    string can cause indefinite execution and RAM corruption. Processing will 
//    not get pass position index 255 max. String of length longer than allowed
//    will not produce correct result. 
//        a : original string (will be left unmodified)
//        b : output string   (content will be changed)
//        c : character to be inserted.
//        p : position index of insertion, 0 is insert up front. Max. 255.
//-----------------------------------------------------------------------------
void __section("i2a") insertAt(char *a, char *b, char c, uint8_t p)
{
    uint8_t i = 0;
    //------------------------------------------------------------------------
    // The two while-loops terminates when end of string character is detected
    //------------------------------------------------------------------------
    while (i != p && (b[i] = a[i]) != '\0') {++i;} //-- copy characters across
    b[p++] = c;                                    //-------- insert character
    while ((b[p++] = a[i++]) != '\0');             // copy remaining character
    return;
}

//-----------------------------------------------------------------------------
// Convert unsigned 32 to string and optionally insert decimal point
//     w: the binary value to be converted
//     d: position of decimal point to be inserted (count from left)
//     o: pre-allocated array of at least 12 characters long for output
// Output string will be placed in o. There is no zero patching neither on the 
// left nor on the right of the decimal point. The decimal point is a simple 
// character insertion in position d. Output is formatted to trim leading 
// spaces so called left-justified. Output will have no spaces on the right. 
// Terminate by '\0'. 
//-----------------------------------------------------------------------------
void __section("i2a") u32_a12_d(uint32_t w, uint8_t d, char* o)
{
    char *u;
    char v[11];
    u = u32_to_a11(w);
    insertAt(u, v, '.', d);
    trimLeft(v, o);
    return;
}
void __section("i2a") u32_a12(uint32_t w, char* o)
{
    char *u;
    u = u32_to_a11(w);
    trimLeft(u, o);
    return;
}

//-----------------------------------------------------------------------------
// hexadecimal digits <-> byte value conversion                       27Feb2020
//-----------------------------------------------------------------------------
const char __section("i2a") hexDigit[]=\
{'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

char __section("i2a") lowewrNibbleToAscii(uint8_t b) 
{ 
    return ((char)hexDigit[b & 0x0F]); 
}

uint8_t __section("i2a") hexCharValue(char b)
{
    for (uint8_t i = 0; i < sizeof(hexDigit)/sizeof(const char); ++i)
    {
        if (b == (char)hexDigit[i])
        {
            return i;
        }
    }
    return 0xFF; 
}

//-----------------------------------------------------------------------------
// atoi() implementations. Auto-ignore leading spaces, the hex version enforces
// '0x' prefix.                                                       02Mar2020
//    a : the string to be converted, e.g. "123" "0x1f"
//    charCont: the number of characters to be processed
//    value : output 
//    Function return value 'true' == success. If 'false', *value contains 
//    garbage (should be discarded, should not be used). 
//-----------------------------------------------------------------------------

bool __section("i2a") hexStringtoi32(char* a, uint8_t charCont, uint32_t *value)
{
    uint8_t i = 0;
    while ((i < charCont) && (a[i] == ' ')) {++i;}
    if (charCont - i < 3) {return false;}
    if ((a[i] != '0') || (a[i+1] != 'x')) {return false;}
    i += 2;
    *value = 0;
    while (i < charCont)
    {
        uint8_t k = a[i++];
        if ((k>='a') && (k<='f')) {k += ('A'-'a');}
        if (!(((k>='0') && (k<='9')) || ((k>='A') && (k<='F')))) {return false;}
        *value <<= 4;
        *value |= hexCharValue(k);
    }
    return true;
}

bool __section("i2a") decStringtoi32(char* a, uint8_t charCont, uint32_t *value)
{
    uint8_t i = 0;
    while ((i < charCont) && (a[i] == ' ')) {++i;}
    if (charCont - i < 1) {return false;}
    *value = 0;
    while (i < charCont)
    {
        uint8_t k = a[i++];
        if (!((k >= '0') && (k <= '9'))) {return false;}
        *value *= 10;
        *value += k - '0';
    }
    return true;
}

//----------------------------------------------------------------- end of file
