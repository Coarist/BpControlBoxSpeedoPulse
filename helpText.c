//-----------------------------------------------------------------------------
// File:   helpText.c
// Author: chi
//
// Created on 04 February 11 2020 11:28
//-----------------------------------------------------------------------------
// EUSART1 text terminal service (console) help text strings store in Flash
// Conforming to C standard, trailing '\0' is inserted automatically. For 
// peace of mind, extra '\0' is appended manually even though testing showed
// the program function normally without them. 
//-----------------------------------------------------------------------------

#include "string.h"

const char __section("helpText") promptText[] = ">\0";

const char __section("helpText") welcomeText[] = \
"\r\nBP Control Box - SPA Speed Pulse Application v1.0\r\n\0";

const char __section("helpText") parseErrText[] = \
"\r\nCommand is unknown. Type \"help\" or \"?\" to display a list.\r\n\0";

const char __section("helpText") eulaHeadText[] = \
"\r\n\
----------------------------------------------\r\n\
This module uses cocoOS task scheduling kernel\r\n\
----------------------------------------------\r\n\
\0";

const char __section("helpText") helpText[] = \
"\r\n\
----------------------------------------------\r\n\
COM characteristics\r\n\
----------------------------------------------\r\n\
Baud (38400,8,N,1) 5V-TTL\r\n\
\r\n\
----------------------------------------------\r\n\
Text terminal keyboard operation\r\n\
----------------------------------------------\r\n\
Lower case non-capital letter ASCII commands.\r\n\
This module parse each LF or CR as 'ENTER'.\r\n\
From this module: lines terminate by CRLF.\r\n\
\r\n\
----------------------------------------------\r\n\
Commands available on this terminal\r\n\
----------------------------------------------\r\n\
help : Display this content (same as \"?\")\r\n\
eula : Show EULA \"End User License Agreement\"\r\n\
sysi : Enquire configuration parameters\r\n\
pod0 : Turn off real-time data to console\r\n\
pod1 : Turn on  real-time data (default)\r\n\
almset0 : All alarm levels reset\r\n\
almset1 : Pre- alarm captured and set\r\n\
almset2 : Main alarm captured and set\r\n\
alevel1 : Pre- alarm level set by value\r\n\
alevel2 : Main alarm level set by value\r\n\
volthre : Threshold voltage control byte\r\n\
samplei : Sampling interval (x10 ms)\r\n\
\r\n\
\0";

const char __section("helpText") devConfigText0[] = \
"\r\n\
----------------------------------------------\r\n\
Module configurations\r\n\
----------------------------------------------\r\n\
\0";

const char __section("helpText") rtDataOnText[] = \
"\r\n\
Live capture data ON.\r\n\0";

const char __section("helpText") rtDataOffText[] = \
"\r\n\
Live capture data OFF.\r\n\0";

//----------------------------------------------------------------- end of file

