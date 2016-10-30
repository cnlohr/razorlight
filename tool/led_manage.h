#ifndef _LED_MANAGE_H
#define _LED_MANAGE_H

#include <stdint.h>

void UpdateAlternate( double dTime );
void UpdateLEDs( double dTime );
int DoLEDReturn( int which, char * psrdata, char * retdata, int retsize );


#define MAX_LED_PER 512

//For output LEDs
uint32_t * GetLEDsByIndex( int idx );

//Actual LEDs
extern uint32_t   LeftLEDs[MAX_LED_PER];
extern uint32_t  RightLEDs[MAX_LED_PER];
extern uint32_t    TopLEDs[MAX_LED_PER];
extern uint32_t BottomLEDs[MAX_LED_PER];

//From Display
extern uint32_t   LeftColors[MAX_LED_PER];
extern uint32_t  RightColors[MAX_LED_PER];
extern uint32_t    TopColors[MAX_LED_PER];
extern uint32_t BottomColors[MAX_LED_PER];

//Alterate - you can set this.
extern uint32_t   LeftAlternate[MAX_LED_PER];
extern uint32_t  RightAlternate[MAX_LED_PER];
extern uint32_t    TopAlternate[MAX_LED_PER];
extern uint32_t BottomAlternate[MAX_LED_PER];


#endif

