#ifndef CustomInterrupts_h
#define CustomInterrupts_h

#include "Arduino.h"

#if !defined( __AVR_ATmega328P__ ) && !defined( __AVR_ATmega2560__ )
#warning "CustomInterrupts is only tested for ATmega328P and ATmega2560"
#endif

#if defined( __AVR_ATmega2560__ )
#define digitalPinToInterruptActual(p) ((p) == 2 ? 4 : ((p) == 3 ? 5 : ((p) >= 18 && (p) <= 21 ? 21 - (p) : NOT_AN_INTERRUPT)))
#else
#define digitalPinToInterruptActual(p) digitalPinToInterrupt(p)
#endif

#define NO_FUNC 0
#define NORMAL 1
#define EDGE 2
#define ARG 3
#define ARG_EDGE 4

void attachInterruptCustom( uint8_t pin , uint8_t mode , void (*func)() );
void attachInterruptCustom( uint8_t pin , uint8_t mode , void (*func)(uint8_t) );
void attachInterruptCustom( uint8_t pin , uint8_t mode , void (*func)(void*) , void *arg );
void attachInterruptCustom( uint8_t pin , uint8_t mode , void (*func)(void* , uint8_t) , void *arg );
void enableInterruptCustom( uint8_t pin );
void disableInterruptCustom( uint8_t pin );
bool interruptEnabledCustom( uint8_t pin );

#endif