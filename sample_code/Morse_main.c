/*
 * main.c
 */

#include <msp430.h>

const char LED = BIT0;
const char POWER = BIT5;
const char BUTTON = BIT3;

#pragma vector=PORT1_VECTOR
__interrupt void p1_interrupt(void) {
	/*
	//Stop the timer and check the time
	TA0CTL &= ~(MC_2);
	//False alarm caused by bounce
	if (TAR  < 4 ) {
		TA0CTL |= MC_2;
		P1IFG &= ~BUTTON;
		return;
	}
	*/
	//Rising edge means button was released
	/*
	if (P1IES & BUTTON) {
		P1OUT &= ~LED;
	}
	else {
		P1OUT |= LED;
	}
	*/
	P1OUT ^= LED;
	//Toggle interrupt edge select
	P1IES ^= BUTTON;
	P1IFG &= ~BUTTON;
	/*
	//Start counting if the button went down
	if (0 == (P1IN & BUTTON)) {
		TA0CTL |= MC_2;
	}
	*/
}

void main(void) {
	//Falling edge
	//P1IES |= BUTTON;
	P1DIR &= ~BUTTON;
	//LED and power pin are outputs
	P1DIR |= LED | POWER;
	//Power pin is on, button is pull up
	P1OUT |= BUTTON | POWER;
	P1REN |= BUTTON;
	P1SEL = 0;
	
	P1IE |= BUTTON;
	P1IFG &= ~BUTTON;

	//Set up the timer to start going when a value is in TACCR0
	//Run from ACLK crystal
	//ACLK divider is 1
	BCSCTL1 |= DIVA_0;
	//12.5pF cap for the crystal oscillator
	//BCSCTL3 |= XCAP_3;
	//Select VLO
	BCSCTL3 |= LFXT1S_2;

	//Set to divide by 8, input is ACLK
	TA0CTL = TASSEL_1 | ID_3;

	__enable_interrupt();

}
