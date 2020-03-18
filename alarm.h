//-----------------------------------------------------------------------------
// File:   alarm.h
// Author: chi
//
// Created on February 25, 2020, 11:57 AM
//-----------------------------------------------------------------------------

#ifndef ALARM_H
#define	ALARM_H

#include "stdint.h"
#include "stdbool.h"

void alarm_task(void);

extern uint32_t pAlarmLevel;
extern uint32_t mAlarmLevel;
extern uint8_t sampleInterval;
char *pAlarmHexString(void);
char *mAlarmHexString(void);
char *mIntervalDecString(void);

#endif	/* ALARM_H */

//----------------------------------------------------------------- end of file
