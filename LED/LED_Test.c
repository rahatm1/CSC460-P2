#include <avr/io.h>
#include "LED_Test.h"
/**
 * \file LED_Test.c
 * \brief Small set of test functions for controlling LEDs on a AT90USBKey
 * 
 * \mainpage Simple set of functions to control the state of the onboard
 *  LEDs on the AT90USBKey. 
 *
 * \author Alexander M. Hoole
 * \date October 2006
 */

void init_LED_B7(void)
{
	DDRB=(1<<DDB7);
	PORTB = 0x00;
}

void init_LED_C7(void)
{
	DDRC = (1<<PORTC7);
	PORTC = 0x00;		//Initialize port to LOW (turn off LEDs)
}

void toggle_LED_B7()
{
	PORTB ^= (1<<PORTB7);
}

void toggle_LED_C7()
{
	PORTC ^= (1<<PC7);
}
