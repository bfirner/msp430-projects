/*
 * main.c
 */

#include <msp430.h>

const char LED1 = BIT0;
const char LED2 = BIT6;

/* The smiley face
 *
 *   0 1 2 3 4
 * 0 +       +
 * 1
 * 2 +       +
 * 3   +   +
 * 4     +
 */

//5 columns for the face in a 5x5 matrix, with two blank buffers
//const unsigned char face[10] = {BIT3, BIT0|BIT4, BIT2|BIT4, BIT0|BIT4, BIT3, 0, 0};
const unsigned char face[10] = {BIT2, BIT0|BIT3, BIT4, BIT0|BIT3, BIT2, 0, 0};

//Start at the first column
int column = 0;
//Timer A0 interrupts for TAIFG and CCR1 and CCR2
#pragma vector=TIMER0_A1_VECTOR
__interrupt void timer0A1(void) {
}

//Timer A0 for CCR0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer0A0(void) {
	P1OUT = face[column];
	column = (column+1)%7;
}

void main(void) {
	//Stop the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	P1DIR = BIT0 | BIT1 | BIT2 | BIT3 | BIT4;
	P1OUT = 0;
	P2DIR = 0;
	P2OUT = 0;

	//Make sure we are running with DCO at 12MHz
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	//Timer A counts to 2083 (60Hz), mode 1 (up), divide SMCLK source by 8
	//TACCR0 interrupts will be used
	TACCR0 = 2083;
	TACCTL0 = 0x10;
	TACTL = MC_1 | ID_3 | TASSEL_2;

	__enable_interrupt();

	while (1)
		;
}
