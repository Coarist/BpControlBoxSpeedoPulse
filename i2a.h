//-----------------------------------------------------------------------------
// File:   i2a.h
// Author: chi
//
// Created on February 11, 2020, 09:01
//-----------------------------------------------------------------------------
// Value to text conversion utility function and associated string manipulation
// function - attempting to be leaner than standard sprintf().
//-----------------------------------------------------------------------------

#ifndef I2A_H
#define	I2A_H

#include "stdint.h"
#include "stdbool.h"
#include "uintegers2.h"
#include "cocoos.h"

//-------------------------------------------------------------------
// Function result in its own allocated RAM variable
//-------------------------------------------------------------------
char *u32_to_a11(uint32_t);

//-------------------------------------------------------------------
// Function directly modifies the operant string
//-------------------------------------------------------------------
void trimRight(char *a);

//-------------------------------------------------------------------
// Functions requires output operant with pre-allocated RAM
//-------------------------------------------------------------------
void trimLeft(char *a, char *b);
void insertAt(char*, char*, char, uint8_t);

//-------------------------------------------------------------------
// Functions with and without decimal dot inserted in desired pos.
//-------------------------------------------------------------------
void u32_a12_d(uint32_t w, uint8_t d, char o[]);
void u32_a12(uint32_t, char*);

//-------------------------------------------------------------------
// task that handles long computation jobs 
//-------------------------------------------------------------------
extern Evt_t u32GoEvent;
extern Evt_t u32DoneEvent;
extern char u32string[];
extern uint32_t u32Operant;
void u32Toa11_task(void);

//-------------------------------------------------------------------
// Hex<->text conversion function prototypes
//-------------------------------------------------------------------
char lowewrNibbleToAscii(uint8_t);
uint8_t hexCharValue(char);
char *u32_to_hex(uinteger32_t, uint8_t);
bool hexStringtoi32(char*, uint8_t, uint32_t*);
bool decStringtoi32(char*, uint8_t, uint32_t*);

#endif	/* I2A_H */

//----------------------------------------------------------------- end of file
