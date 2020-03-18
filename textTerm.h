//-----------------------------------------------------------------------------
// File:   textTerm.h
// Author: chi
//
// Created on February 12, 2020, 08:41
//-----------------------------------------------------------------------------
// EUSART1 text terminal service (console)
//-----------------------------------------------------------------------------

#ifndef TEXTTERM_H
#define	TEXTTERM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "stdbool.h"
    
bool printa(char* constantStringStartAddress);
void print_task(void);
void textTerminal_task(void);

#ifdef	__cplusplus
}
#endif

#endif	/* TEXTTERM_H */

//----------------------------------------------------------------- end of file
