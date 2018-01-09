/*
 * main.c
 */

/*
 * When compiling with msp430-gcc use the -mmcu flag to specify the target.
 * For example: msp430-gcc Blink_main.c -mmcu=msp430g2553
 * or: msp430-gcc Blink_main.c -mmcu=msp430g2452
 * To flash a launchpad device with mspdebug:
 * mspdebug rf2500 "prog filename"
 */
#include <msp430.h>

const char LED = BIT0;

#pragma vector=TIMER0_A0_VECTOR
__interrupt void timerA(void) {
	P1OUT ^= LED;
}

void main(void) {
	WDTCTL = WDTPW + WDTHOLD;

	P1DIR |= LED;
	P1OUT = 0;

	//Mode 1, source SMCLK, div by 8
	TA0CTL = MC_1 | ID_3 | TASSEL_2;
	
	TACCR0 = 65250;
	TACCTL0 = CCIE;

	__enable_interrupt();

	while(1);
}
