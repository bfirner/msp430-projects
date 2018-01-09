/*
 * main.c
 */

#include <msp430.h>

const char LED1 = BIT0;
const char LED2 = BIT6;

int count = 0;
//Timer A0 interrupts for TAIFG and CCR1 and CCR2
#pragma vector=TIMER0_A1_VECTOR
__interrupt void timer0A1(void) {
	P1OUT ^= LED1;
}

//Timer A0 for CCR0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer0A0(void) {
	P1OUT ^= LED2;
}

void main(void) {
	//Stop the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	P1DIR = LED1 | LED2;
	P1OUT = 0;
	P2DIR = 0;
	P2OUT = 0;

	//Make sure we are running with DCO at 12MHz
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	//Mode two, divide by 1, SMCLK clock source, interrupt enabled.
	//TACTL = MC_2 | ID_0 | TASSEL_2 | TAIE;

	//TACCR0 = 12000;					// Count limit (16 bit)
	//TACCTL0 = 0x10;					// Enable counter interrupts, bit 4=1
	//TACTL = TASSEL_2 | MC_1;
	//TACTL = MC_1 | ID_0 | TASSEL_2 | TAIE;

	//Timer A counts to 65250, mode 1 (up), divide SMCLK source by 8
	//TACCR0 interrupts will be used
	TACCR0 = 65250;
	TACCTL0 = 0x10;
	TACTL = MC_1 | ID_3 | TASSEL_2;

	__enable_interrupt();

	while (1)
		;
}
