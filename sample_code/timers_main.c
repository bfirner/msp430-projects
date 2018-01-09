/*
 * main.c
 */
#include <msp430.h>

int counter = 0;

//Set up a watchdog timer interrupt that toggles the LED
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void) {
    WDTCTL = WDT_MDLY_32;
    if (30 == ++counter) {
    	P1OUT ^= 0x01;
    	counter = 0;
    }
}

void main(void) {
	//Set up the port
	P1OUT = 0;
	P1DIR = 0;
	P1SEL = 0;
	P1REN = 0;

	//Enable LED as output
	P1DIR |= 0x01;

	BCSCTL1 = CALBC1_12MHZ;
	DCOCTL = CALDCO_12MHZ;

	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	//Enable the watchdog timer interrupt and global interrupts
    IE1 |= WDTIE;
    __enable_interrupt();
    //32ms timer from SMCLK
    WDTCTL = WDT_MDLY_32;
    while (1);
}

