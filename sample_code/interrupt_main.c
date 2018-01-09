/*
 * main.c
 */

#include <msp430.h>

//Set up variables for the input and LED
volatile const unsigned char led = 0x01;
volatile const unsigned char input = 0x08;

#pragma vector=PORT1_VECTOR
__interrupt void portInterrupt(void) {
	//Toggle the LEDs here
	P1OUT ^= led;
	P1IFG &= ~(input);
}


void main(void) {
	//Turn off the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	//Initialize ports
	P1DIR = 0xFF;
	P1OUT = 0;
	P1SEL = 0;

	//Set up an input pin and
	//an output pin for the LED
	P1DIR |= led;
	P1DIR &= ~(input);

	//Set up a pull-down resistor
	P1REN |= input;

	//Enable interrupts on the input pin
	P1IE |= input;

	//Enable interrupts
	__enable_interrupt();


	while (1);
}
