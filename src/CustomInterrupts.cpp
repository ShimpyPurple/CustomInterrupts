#include "CustomInterrupts.h"

void emptyFunc_CustomInterrupts() {}
void emptyFuncArg_CustomInterrupts( void *arg ) {}

struct {
    uint8_t pin = 0;
    uint8_t mode = OFF;
    void (*func)() = emptyFunc_CustomInterrupts;
    void (*funcArg)(void*) = emptyFuncArg_CustomInterrupts;
    void *arg = nullptr;
    volatile bool prev = false;
} pcIntStruct[3][8];

struct {
    void (*func)() = emptyFunc_CustomInterrupts;
    void (*funcArg)(void*) = emptyFuncArg_CustomInterrupts;
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
    
    if ( digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT ) {
        uint8_t extIntNum = digitalPinToInterrupt( pin );
        setExtIntRegs( extIntNum , mode );
        extIntStruct[extIntNum].func = func;
        extIntStruct[extIntNum].arg  = nullptr;
    } else if ( digitalPinToPCICR(pin) != nullptr ) {
        uint8_t PCICRbit = digitalPinToPCICRbit( pin );
        uint8_t *PCMSKn  = digitalPinToPCMSK( pin );
        uint8_t PCMSKbit = digitalPinToPCMSKbit( pin );
        PCICR   |= ( 1<<PCICRbit );
        *PCMSKn |= ( 1<<PCMSKbit );
        pcIntStruct[PCICRbit][PCMSKbit].pin  = pin;
        pcIntStruct[PCICRbit][PCMSKbit].mode = mode;
        pcIntStruct[PCICRbit][PCMSKbit].func = func;
        pcIntStruct[PCICRbit][PCMSKbit].arg  = nullptr;
        pcIntStruct[PCICRbit][PCMSKbit].prev = digitalRead( pin );
    }
    
    SREG = oldSREG;
}

void attachInterruptCustom( uint8_t pin , uint8_t mode , void (*func)(void*) , void *arg ) {
    uint8_t oldSREG = SREG;
    cli();
    
    if ( digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT ) {
        uint8_t extIntNum = digitalPinToInterrupt( pin );
        setExtIntRegs( extIntNum , mode );
        extIntStruct[extIntNum].funcArg = func;
        extIntStruct[extIntNum].arg     = arg;
    } else if ( digitalPinToPCICR(pin) != nullptr ) {
        uint8_t PCICRbit = digitalPinToPCICRbit( pin );
        uint8_t *PCMSKn  = digitalPinToPCMSK( pin );
        uint8_t PCMSKbit = digitalPinToPCMSKbit( pin );
        PCICR   |= ( 1<<PCICRbit );
        *PCMSKn |= ( 1<<PCMSKbit );
        pcIntStruct[PCICRbit][PCMSKbit].pin     = pin;
        pcIntStruct[PCICRbit][PCMSKbit].mode    = mode;
        pcIntStruct[PCICRbit][PCMSKbit].funcArg = func;
        pcIntStruct[PCICRbit][PCMSKbit].arg     = arg;
        pcIntStruct[PCICRbit][PCMSKbit].prev    = digitalRead( pin );
    }
    
    SREG = oldSREG;
}

void enableInterruptCustom( uint8_t pin ) {
    uint8_t oldSREG = SREG;
    cli();
    
    if ( digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT ) {
        switch ( digitalPinToInterrupt(pin) ) {
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
            default: return;
        }
    } else if ( digitalPinToPCICR(pin) != nullptr ) {
        uint8_t PCICRbit = digitalPinToPCICRbit( pin );
        uint8_t *PCMSKn  = digitalPinToPCMSK( pin );
        uint8_t PCMSKbit = digitalPinToPCMSKbit( pin );
        PCICR   |= ( 1<<PCICRbit );
        *PCMSKn |= ( 1<<PCMSKbit );
    }
    
    SREG = oldSREG;
}

void disableInterruptCustom( uint8_t pin ) {
    uint8_t oldSREG = SREG;
    cli();
    
    if ( digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT ) {
        switch ( digitalPinToInterrupt(pin) ) {
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
            default: return;
        }
    } else if ( digitalPinToPCICR(pin) != nullptr ) {
        uint8_t PCICRbit = digitalPinToPCICRbit( pin );
        uint8_t *PCMSKn  = digitalPinToPCMSK( pin );
        uint8_t PCMSKbit = digitalPinToPCMSKbit( pin );
        *PCMSKn &= ~( 1<<PCMSKbit );
        if ( *PCMSKn == 0 ) {
            PCICR &= ~( 1<<PCICRbit );
        }
    }
    
    SREG = oldSREG;
}

bool interruptEnabledCustom( uint8_t pin ) {
    if ( digitalPinToInterrupt(pin) != NOT_AN_INTERRUPT ) {
        switch ( digitalPinToInterrupt(pin) ) {
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
            default: return;
        }
    } else if ( digitalPinToPCICR(pin) != nullptr ) {
        uint8_t PCICRbit = digitalPinToPCICRbit( pin );
        uint8_t *PCMSKn  = digitalPinToPCMSK( pin );
        uint8_t PCMSKbit = digitalPinToPCMSKbit( pin );
        return ( PCICR & (1<<PCICRbit) ) && ( *PCMSKn & (1<<PCMSKbit) );
    } else {
        return false;
    }
}

inline void handlePCInterrupt( uint8_t PCICRbit , uint8_t PCMSKbit ) {
    if ( pcIntStruct[PCICRbit][PCMSKbit].mode == OFF ) return;
    bool pinState = digitalRead( pcIntStruct[PCICRbit][PCMSKbit].pin );
    if (
        ( pinState != pcIntStruct[PCICRbit][PCMSKbit].prev ) && (
            ( pcIntStruct[PCICRbit][PCMSKbit].mode == CHANGE  ) ||
            ( pcIntStruct[PCICRbit][PCMSKbit].mode == FALLING ) && ( !pinState ) ||
            ( pcIntStruct[PCICRbit][PCMSKbit].mode == RISING  ) && (  pinState )
        )
    ) {
        if ( pcIntStruct[PCICRbit][PCMSKbit].arg == nullptr ) {
            pcIntStruct[PCICRbit][PCMSKbit].func();
        } else {
            pcIntStruct[PCICRbit][PCMSKbit].funcArg( pcIntStruct[PCICRbit][PCMSKbit].arg );
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

ISR( INT0_vect ) { if (extIntStruct[0].arg == nullptr) extIntStruct[0].func(); else extIntStruct[0].funcArg( extIntStruct[0].arg ); }
ISR( INT1_vect ) { if (extIntStruct[1].arg == nullptr) extIntStruct[1].func(); else extIntStruct[1].funcArg( extIntStruct[1].arg ); }
#if defined( __AVR_ATmega2560__ )
ISR( INT2_vect ) { if (extIntStruct[2].arg == nullptr) extIntStruct[2].func(); else extIntStruct[2].funcArg( extIntStruct[2].arg ); }
ISR( INT3_vect ) { if (extIntStruct[3].arg == nullptr) extIntStruct[3].func(); else extIntStruct[3].funcArg( extIntStruct[3].arg ); }
ISR( INT4_vect ) { if (extIntStruct[4].arg == nullptr) extIntStruct[4].func(); else extIntStruct[4].funcArg( extIntStruct[4].arg ); }
ISR( INT5_vect ) { if (extIntStruct[5].arg == nullptr) extIntStruct[5].func(); else extIntStruct[5].funcArg( extIntStruct[5].arg ); }
ISR( INT6_vect ) { if (extIntStruct[6].arg == nullptr) extIntStruct[6].func(); else extIntStruct[6].funcArg( extIntStruct[6].arg ); }
ISR( INT7_vect ) { if (extIntStruct[7].arg == nullptr) extIntStruct[7].func(); else extIntStruct[7].funcArg( extIntStruct[7].arg ); }
#endif