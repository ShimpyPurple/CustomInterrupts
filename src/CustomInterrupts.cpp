#include "CustomInterrupts.h"

void emptyFunc_CustomInterrupts() {}
void emptyFuncEdge_CustomInterrupts( uint8_t ) {}
void emptyFuncArg_CustomInterrupts( void* ) {}
void emptyFuncArgEdge_CustomInterrupts( void* , uint8_t ) {}

struct {
    uint8_t pin;
    uint8_t mode;
    bool enabled = false;
    void (*func)() = emptyFunc_CustomInterrupts;
    void (*funcEdge)(uint8_t) = emptyFuncEdge_CustomInterrupts;
    void (*funcArg)(void*) = emptyFuncArg_CustomInterrupts;
    void (*funcArgEdge)(void* , uint8_t) = emptyFuncArgEdge_CustomInterrupts;
    uint8_t type = INT_NO_FUNC;
    void *arg = nullptr;
    volatile bool prev = false;
} pcIntStruct[3][8];

struct {
    uint8_t pin;
    void (*func)() = emptyFunc_CustomInterrupts;
    void (*funcEdge)(uint8_t) = emptyFuncEdge_CustomInterrupts;
    void (*funcArg)(void*) = emptyFuncArg_CustomInterrupts;
    void (*funcArgEdge)(void* , uint8_t) = emptyFuncArgEdge_CustomInterrupts;
    uint8_t type = INT_NO_FUNC;
    void *arg = nullptr;
} extIntStruct[
#if defined( __AVR_ATmega2560__ )
    8
#else
    2
#endif
];

void setExtIntRegs( uint8_t extIntNum , uint8_t mode ) {
    uint8_t *EICR = nullptr;
    uint8_t ISCn1;
    uint8_t ISCn0;
    
    switch ( extIntNum ) {
        case 0: EICR = &EICRA; ISCn1 = ISC01;  ISCn0 = ISC00; EIMSK |= ( 1<<INT0 ); break;
        case 1: EICR = &EICRA; ISCn1 = ISC11;  ISCn0 = ISC10; EIMSK |= ( 1<<INT1 ); break;
#if defined( __AVR_ATmega2560__ )
        case 2: EICR = &EICRA; ISCn1 = ISC21;  ISCn0 = ISC20; EIMSK |= ( 1<<INT2 ); break;
        case 3: EICR = &EICRA; ISCn1 = ISC31;  ISCn0 = ISC30; EIMSK |= ( 1<<INT3 ); break;
        case 4: EICR = &EICRB; ISCn1 = ISC41;  ISCn0 = ISC40; EIMSK |= ( 1<<INT4 ); break;
        case 5: EICR = &EICRB; ISCn1 = ISC51;  ISCn0 = ISC50; EIMSK |= ( 1<<INT5 ); break;
        case 6: EICR = &EICRB; ISCn1 = ISC61;  ISCn0 = ISC60; EIMSK |= ( 1<<INT6 ); break;
        case 7: EICR = &EICRB; ISCn1 = ISC71;  ISCn0 = ISC70; EIMSK |= ( 1<<INT7 ); break;
#endif
        default: return;
    }
    
    *EICR &= ~( (1<<ISCn1) | (1<<ISCn0) );
    switch ( mode ) {
        case LOW:                                           break;
        case CHANGE:  *EICR |=                ( 1<<ISCn0 ); break;
        case FALLING: *EICR |= ( 1<<ISCn1 );                break;
        case RISING:  *EICR |= ( 1<<ISCn1 ) | ( 1<<ISCn0 ); break;
    }
}

void attachInterruptCustom( uint8_t pin , uint8_t mode , void (*func)() ) {
    uint8_t oldSREG = SREG;
    cli();
    
    if ( digitalPinToInterruptActual(pin) != NOT_AN_INTERRUPT ) {
        uint8_t extIntNum = digitalPinToInterruptActual( pin );
        setExtIntRegs( extIntNum , mode );
        extIntStruct[extIntNum].pin  = pin;
        extIntStruct[extIntNum].func = func;
        extIntStruct[extIntNum].type = INT_NORMAL;
    } else if ( digitalPinToPCICR(pin) != nullptr ) {
        uint8_t PCICRbit = digitalPinToPCICRbit( pin );
        uint8_t *PCMSKn  = digitalPinToPCMSK( pin );
        uint8_t PCMSKbit = digitalPinToPCMSKbit( pin );
        PCICR   |= ( 1<<PCICRbit );
        *PCMSKn |= ( 1<<PCMSKbit );
        pcIntStruct[PCICRbit][PCMSKbit].pin     = pin;
        pcIntStruct[PCICRbit][PCMSKbit].mode    = mode;
        pcIntStruct[PCICRbit][PCMSKbit].enabled = true;
        pcIntStruct[PCICRbit][PCMSKbit].func    = func;
        pcIntStruct[PCICRbit][PCMSKbit].type    = INT_NORMAL;
        pcIntStruct[PCICRbit][PCMSKbit].prev    = digitalRead( pin );
    }
    
    SREG = oldSREG;
}

void attachInterruptCustom( uint8_t pin , uint8_t mode , void (*func)(uint8_t) ) {
    uint8_t oldSREG = SREG;
    cli();
    
    if ( digitalPinToInterruptActual(pin) != NOT_AN_INTERRUPT ) {
        uint8_t extIntNum = digitalPinToInterruptActual( pin );
        setExtIntRegs( extIntNum , mode );
        extIntStruct[extIntNum].pin      = pin;
        extIntStruct[extIntNum].funcEdge = func;
        extIntStruct[extIntNum].type     = INT_EDGE;
    } else if ( digitalPinToPCICR(pin) != nullptr ) {
        uint8_t PCICRbit = digitalPinToPCICRbit( pin );
        uint8_t *PCMSKn  = digitalPinToPCMSK( pin );
        uint8_t PCMSKbit = digitalPinToPCMSKbit( pin );
        PCICR   |= ( 1<<PCICRbit );
        *PCMSKn |= ( 1<<PCMSKbit );
        pcIntStruct[PCICRbit][PCMSKbit].pin      = pin;
        pcIntStruct[PCICRbit][PCMSKbit].mode     = mode;
        pcIntStruct[PCICRbit][PCMSKbit].enabled  = true;
        pcIntStruct[PCICRbit][PCMSKbit].funcEdge = func;
        pcIntStruct[PCICRbit][PCMSKbit].type     = INT_EDGE;
        pcIntStruct[PCICRbit][PCMSKbit].prev     = digitalRead( pin );
    }
    
    SREG = oldSREG;
}

void attachInterruptCustom( uint8_t pin , uint8_t mode , void (*func)(void*) , void *arg ) {
    uint8_t oldSREG = SREG;
    cli();
    
    if ( digitalPinToInterruptActual(pin) != NOT_AN_INTERRUPT ) {
        uint8_t extIntNum = digitalPinToInterruptActual( pin );
        setExtIntRegs( extIntNum , mode );
        extIntStruct[extIntNum].pin     = pin;
        extIntStruct[extIntNum].funcArg = func;
        extIntStruct[extIntNum].type    = INT_ARG;
        extIntStruct[extIntNum].arg     = arg;
    } else if ( digitalPinToPCICR(pin) != nullptr ) {
        uint8_t PCICRbit = digitalPinToPCICRbit( pin );
        uint8_t *PCMSKn  = digitalPinToPCMSK( pin );
        uint8_t PCMSKbit = digitalPinToPCMSKbit( pin );
        PCICR   |= ( 1<<PCICRbit );
        *PCMSKn |= ( 1<<PCMSKbit );
        pcIntStruct[PCICRbit][PCMSKbit].pin     = pin;
        pcIntStruct[PCICRbit][PCMSKbit].mode    = mode;
        pcIntStruct[PCICRbit][PCMSKbit].enabled = true;
        pcIntStruct[PCICRbit][PCMSKbit].funcArg = func;
        pcIntStruct[PCICRbit][PCMSKbit].type    = INT_ARG;
        pcIntStruct[PCICRbit][PCMSKbit].arg     = arg;
        pcIntStruct[PCICRbit][PCMSKbit].prev    = digitalRead( pin );
    }
    
    SREG = oldSREG;
}

void attachInterruptCustom( uint8_t pin , uint8_t mode , void (*func)(void* , uint8_t) , void *arg ) {
    uint8_t oldSREG = SREG;
    cli();
    
    if ( digitalPinToInterruptActual(pin) != NOT_AN_INTERRUPT ) {
        uint8_t extIntNum = digitalPinToInterruptActual( pin );
        setExtIntRegs( extIntNum , mode );
        extIntStruct[extIntNum].pin         = pin;
        extIntStruct[extIntNum].funcArgEdge = func;
        extIntStruct[extIntNum].type        = INT_ARG_EDGE;
        extIntStruct[extIntNum].arg         = arg;
    } else if ( digitalPinToPCICR(pin) != nullptr ) {
        uint8_t PCICRbit = digitalPinToPCICRbit( pin );
        uint8_t *PCMSKn  = digitalPinToPCMSK( pin );
        uint8_t PCMSKbit = digitalPinToPCMSKbit( pin );
        PCICR   |= ( 1<<PCICRbit );
        *PCMSKn |= ( 1<<PCMSKbit );
        pcIntStruct[PCICRbit][PCMSKbit].pin         = pin;
        pcIntStruct[PCICRbit][PCMSKbit].mode        = mode;
        pcIntStruct[PCICRbit][PCMSKbit].enabled     = true;
        pcIntStruct[PCICRbit][PCMSKbit].funcArgEdge = func;
        pcIntStruct[PCICRbit][PCMSKbit].type        = INT_ARG_EDGE;
        pcIntStruct[PCICRbit][PCMSKbit].arg         = arg;
        pcIntStruct[PCICRbit][PCMSKbit].prev        = digitalRead( pin );
    }
    
    SREG = oldSREG;
}

void enableInterruptCustom( uint8_t pin ) {
    uint8_t oldSREG = SREG;
    cli();
    
    switch ( digitalPinToInterruptActual(pin) ) {
        case 0: EIMSK |= ( 1<<INT0 ); break;
        case 1: EIMSK |= ( 1<<INT1 ); break;
#if defined( __AVR_ATmega2560__ )
        case 2: EIMSK |= ( 1<<INT2 ); break;
        case 3: EIMSK |= ( 1<<INT3 ); break;
        case 4: EIMSK |= ( 1<<INT4 ); break;
        case 5: EIMSK |= ( 1<<INT5 ); break;
        case 6: EIMSK |= ( 1<<INT6 ); break;
        case 7: EIMSK |= ( 1<<INT7 ); break;
#endif
        default:
            if ( digitalPinToPCICR(pin) != nullptr ) {
                uint8_t PCICRbit = digitalPinToPCICRbit( pin );
                uint8_t *PCMSKn  = digitalPinToPCMSK( pin );
                uint8_t PCMSKbit = digitalPinToPCMSKbit( pin );
                PCICR   |= ( 1<<PCICRbit );
                *PCMSKn |= ( 1<<PCMSKbit );
                pcIntStruct[PCICRbit][PCMSKbit].enabled = true;
            }
            break;
    }
    
    SREG = oldSREG;
}

void disableInterruptCustom( uint8_t pin ) {
    uint8_t oldSREG = SREG;
    cli();
    
    switch ( digitalPinToInterruptActual(pin) ) {
        case 0: EIMSK &= ~( 1<<INT0 ); break;
        case 1: EIMSK &= ~( 1<<INT1 ); break;
#if defined( __AVR_ATmega2560__ )
        case 2: EIMSK &= ~( 1<<INT2 ); break;
        case 3: EIMSK &= ~( 1<<INT3 ); break;
        case 4: EIMSK &= ~( 1<<INT4 ); break;
        case 5: EIMSK &= ~( 1<<INT5 ); break;
        case 6: EIMSK &= ~( 1<<INT6 ); break;
        case 7: EIMSK &= ~( 1<<INT7 ); break;
#endif
        default:
            if ( digitalPinToPCICR(pin) != nullptr ) {
                uint8_t PCICRbit = digitalPinToPCICRbit( pin );
                uint8_t *PCMSKn  = digitalPinToPCMSK( pin );
                uint8_t PCMSKbit = digitalPinToPCMSKbit( pin );
                *PCMSKn &= ~( 1<<PCMSKbit );
                if ( *PCMSKn == 0 ) {
                    PCICR &= ~( 1<<PCICRbit );
                }
                pcIntStruct[PCICRbit][PCMSKbit].enabled = false;
            }
            break;
    }
    
    SREG = oldSREG;
}

bool interruptEnabledCustom( uint8_t pin ) {
    switch ( digitalPinToInterruptActual(pin) ) {
        case 0: return EIMSK & ( 1<<INT0 );
        case 1: return EIMSK & ( 1<<INT1 );
#if defined( __AVR_ATmega2560__ )
        case 2: return EIMSK & ( 1<<INT2 );
        case 3: return EIMSK & ( 1<<INT3 );
        case 4: return EIMSK & ( 1<<INT4 );
        case 5: return EIMSK & ( 1<<INT5 );
        case 6: return EIMSK & ( 1<<INT6 );
        case 7: return EIMSK & ( 1<<INT7 );
#endif
        default:
            if ( digitalPinToPCICR(pin) != nullptr ) {
                uint8_t PCICRbit = digitalPinToPCICRbit( pin );
                uint8_t *PCMSKn  = digitalPinToPCMSK( pin );
                uint8_t PCMSKbit = digitalPinToPCMSKbit( pin );
                return pcIntStruct[PCICRbit][PCMSKbit].enabled;
            } else {
                return false;
            }
    }
}

inline void handlePCInterrupt( uint8_t PCICRbit , uint8_t PCMSKbit ) {
    if ( !pcIntStruct[PCICRbit][PCMSKbit].enabled ) return;
    bool pinState = digitalRead( pcIntStruct[PCICRbit][PCMSKbit].pin );
    if (
        ( pinState != pcIntStruct[PCICRbit][PCMSKbit].prev ) && (
            ( pcIntStruct[PCICRbit][PCMSKbit].mode == CHANGE  ) ||
            ( pcIntStruct[PCICRbit][PCMSKbit].mode == FALLING ) && ( !pinState ) ||
            ( pcIntStruct[PCICRbit][PCMSKbit].mode == RISING  ) && (  pinState )
        )
    ) {
        switch ( pcIntStruct[PCICRbit][PCMSKbit].type ) {
            case INT_NORMAL:   pcIntStruct[PCICRbit][PCMSKbit].func(); break;
            case INT_EDGE:     pcIntStruct[PCICRbit][PCMSKbit].funcEdge( pinState ? RISING : FALLING ); break; 
            case INT_ARG:      pcIntStruct[PCICRbit][PCMSKbit].funcArg( pcIntStruct[PCICRbit][PCMSKbit].arg ); break; 
            case INT_ARG_EDGE: pcIntStruct[PCICRbit][PCMSKbit].funcArgEdge( pcIntStruct[PCICRbit][PCMSKbit].arg , pinState ? RISING : FALLING ); break; 
        }
    }
    pcIntStruct[PCICRbit][PCMSKbit].prev = pinState;
}

ISR( PCINT0_vect ) {
    for ( uint8_t i=0 ; i<8 ; ++i ) {
        handlePCInterrupt( 0 , i );
    }
}

ISR( PCINT1_vect ) {
    for ( uint8_t i=0 ; i<8 ; ++i ) {
        handlePCInterrupt( 1 , i );
    }
}

ISR( PCINT2_vect ) {
    for ( uint8_t i=0 ; i<8 ; ++i ) {
        handlePCInterrupt( 2 , i );
    }
}

ISR( INT0_vect ) {
    switch ( extIntStruct[0].type ) {
        case INT_NORMAL:   extIntStruct[0].func(); break;
        case INT_EDGE:     extIntStruct[0].funcEdge( digitalRead(extIntStruct[0].pin) ? RISING : FALLING ); break; 
        case INT_ARG:      extIntStruct[0].funcArg( extIntStruct[0].arg ); break; 
        case INT_ARG_EDGE: extIntStruct[0].funcArgEdge( extIntStruct[0].arg , digitalRead(extIntStruct[0].pin) ? RISING : FALLING ); break; 
    }
}
ISR( INT1_vect ) {
    switch ( extIntStruct[1].type ) {
        case INT_NORMAL:   extIntStruct[1].func(); break;
        case INT_EDGE:     extIntStruct[1].funcEdge( digitalRead(extIntStruct[1].pin) ? RISING : FALLING ); break; 
        case INT_ARG:      extIntStruct[1].funcArg( extIntStruct[1].arg ); break; 
        case INT_ARG_EDGE: extIntStruct[1].funcArgEdge( extIntStruct[1].arg , digitalRead(extIntStruct[1].pin) ? RISING : FALLING ); break; 
    }
}
#if defined( __AVR_ATmega2560__ )
ISR( INT2_vect ) {
    switch ( extIntStruct[2].type ) {
        case INT_NORMAL:   extIntStruct[2].func(); break;
        case INT_EDGE:     extIntStruct[2].funcEdge( digitalRead(extIntStruct[2].pin) ? RISING : FALLING ); break; 
        case INT_ARG:      extIntStruct[2].funcArg( extIntStruct[2].arg ); break; 
        case INT_ARG_EDGE: extIntStruct[2].funcArgEdge( extIntStruct[2].arg , digitalRead(extIntStruct[2].pin) ? RISING : FALLING ); break; 
    }
}
ISR( INT3_vect ) {
    switch ( extIntStruct[3].type ) {
        case INT_NORMAL:   extIntStruct[3].func(); break;
        case INT_EDGE:     extIntStruct[3].funcEdge( digitalRead(extIntStruct[3].pin) ? RISING : FALLING ); break; 
        case INT_ARG:      extIntStruct[3].funcArg( extIntStruct[3].arg ); break; 
        case INT_ARG_EDGE: extIntStruct[3].funcArgEdge( extIntStruct[3].arg , digitalRead(extIntStruct[3].pin) ? RISING : FALLING ); break; 
    }
}
ISR( INT4_vect ) {
    switch ( extIntStruct[4].type ) {
        case INT_NORMAL:   extIntStruct[4].func(); break;
        case INT_EDGE:     extIntStruct[4].funcEdge( digitalRead(extIntStruct[4].pin) ? RISING : FALLING ); break; 
        case INT_ARG:      extIntStruct[4].funcArg( extIntStruct[4].arg ); break; 
        case INT_ARG_EDGE: extIntStruct[4].funcArgEdge( extIntStruct[4].arg , digitalRead(extIntStruct[4].pin) ? RISING : FALLING ); break; 
    }
}
ISR( INT5_vect ) {
    switch ( extIntStruct[5].type ) {
        case INT_NORMAL:   extIntStruct[5].func(); break;
        case INT_EDGE:     extIntStruct[5].funcEdge( digitalRead(extIntStruct[5].pin) ? RISING : FALLING ); break; 
        case INT_ARG:      extIntStruct[5].funcArg( extIntStruct[5].arg ); break; 
        case INT_ARG_EDGE: extIntStruct[5].funcArgEdge( extIntStruct[5].arg , digitalRead(extIntStruct[5].pin) ? RISING : FALLING ); break; 
    }
}
ISR( INT6_vect ) {
    switch ( extIntStruct[6].type ) {
        case INT_NORMAL:   extIntStruct[6].func(); break;
        case INT_EDGE:     extIntStruct[6].funcEdge( digitalRead(extIntStruct[6].pin) ? RISING : FALLING ); break; 
        case INT_ARG:      extIntStruct[6].funcArg( extIntStruct[6].arg ); break; 
        case INT_ARG_EDGE: extIntStruct[6].funcArgEdge( extIntStruct[6].arg , digitalRead(extIntStruct[6].pin) ? RISING : FALLING ); break; 
    }
}
ISR( INT7_vect ) {
    switch ( extIntStruct[7].type ) {
        case INT_NORMAL:   extIntStruct[7].func(); break;
        case INT_EDGE:     extIntStruct[7].funcEdge( digitalRead(extIntStruct[7].pin) ? RISING : FALLING ); break; 
        case INT_ARG:      extIntStruct[7].funcArg( extIntStruct[7].arg ); break; 
        case INT_ARG_EDGE: extIntStruct[7].funcArgEdge( extIntStruct[7].arg , digitalRead(extIntStruct[7].pin) ? RISING : FALLING ); break; 
    }
}
#endif

struct {
    void (*func)() = emptyFunc_CustomInterrupts;
    void (*funcArg)(void*) = emptyFuncArg_CustomInterrupts;
    volatile uint8_t type = INT_NO_FUNC;
    void *arg = nullptr;
    volatile uint32_t trigTime = 0;
    uint32_t repeat = 0;
} runAfterIntStruct[ MAX_RUN_AFTERS ];

uint8_t runAfter( uint32_t ms , void (*func)() , bool repeat=false ) {
    for ( uint8_t i=0 ; i<MAX_RUN_AFTERS ; ++i ) {
        if ( runAfterIntStruct[i].type == INT_NO_FUNC ) {
            runAfterIntStruct[i].func = func;
            runAfterIntStruct[i].type = INT_NORMAL;
            runAfterIntStruct[i].trigTime = millis() + ms;
            runAfterIntStruct[i].repeat = repeat ? ms : 0;
            return i+1;
        }
    }
    return 0;
}

uint8_t runAfter( uint32_t ms , void (*func)(void*) , void *arg , bool repeat=false ) {
    for ( uint8_t i=0 ; i<MAX_RUN_AFTERS ; ++i ) {
        if ( runAfterIntStruct[i].type == INT_NO_FUNC ) {
            runAfterIntStruct[i].funcArg = func;
            runAfterIntStruct[i].arg = arg;
            runAfterIntStruct[i].type = INT_ARG;
            runAfterIntStruct[i].trigTime = millis() + ms;
            runAfterIntStruct[i].repeat = repeat ? ms : 0;
            return i+1;
        }
    }
    return 0;
}

void runAfterCancel( uint8_t id ) {
    if ( id == 0 ) return;
    if ( id > MAX_RUN_AFTERS ) return;
    runAfterIntStruct[id-1].type = INT_NO_FUNC;
}

ISR( TIMER0_COMPB_vect ) {
    for ( uint8_t i=0 ; i<MAX_RUN_AFTERS ; ++i ) {
        switch ( runAfterIntStruct[i].type ) {
            case INT_NORMAL:
                if ( millis() - runAfterIntStruct[i].trigTime <= 1 ) {
                    if ( runAfterIntStruct[i].repeat > 0 ) {
                        runAfterIntStruct[i].trigTime = millis() + runAfterIntStruct[i].repeat;
                    } else {
                        runAfterIntStruct[i].type = INT_NO_FUNC;
                    }
                    runAfterIntStruct[i].func();
                }
                break;
            case INT_ARG:
                if ( millis() - runAfterIntStruct[i].trigTime <= 1 ) {
                    if ( runAfterIntStruct[i].repeat > 0 ) {
                        runAfterIntStruct[i].trigTime = millis() + runAfterIntStruct[i].repeat;
                    } else {
                        runAfterIntStruct[i].type = INT_NO_FUNC;
                    }
                    runAfterIntStruct[i].funcArg( runAfterIntStruct[i].arg );
                }
                break;
        }
    }
}
