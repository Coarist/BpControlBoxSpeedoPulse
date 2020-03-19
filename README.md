# BpControlBoxSpeedoPulse

BCBSP BP Control Box adapted by software to speed pulse application. Comparing with sister project **06SENSE_PULSE**, cost per unit is higher whereas:
- Pulse edge much higher 1000 KHz
- Pulse magnitude much wider range 2V to 36V
- Higher accuracy
- Has 5V COM port text terminal service

This project produces firmware for **BP Control Box**. Function is to sense (speed) pulse time interval. When this time interval is smaller than a predetermined limit (meaning higher frequency hence speed), an alert or an alarm will be produced.  

# CocoOS
The CocoOS RTOS kernel is used in 'time-slicing' 'cooperative multitasking' 'co-routine' mode.

# IDE and build tool
```
Microchip MPLAB X IDE v5.25
Microchip XC8 v1.42 compiler
```
