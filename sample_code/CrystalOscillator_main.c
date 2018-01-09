/*
 * main.c
 */
#include <msp430.h>

const unsigned char led = BIT0;
const unsigned char xin = BIT6;
const unsigned char xout = BIT7;
unsigned char led_on = 0;

#pragma vector=TIMER0_A0_VECTOR
__interrupt void timerA(void) {
	P1OUT ^= led;
/*
	led_on ^= 1;
	if (led_on) {
		P1DIR |= led;
		P1REN &= ~led;
		P1OUT = led;
	}
	else {
		P1DIR &= ~led;
		P1REN |= led;
		P1OUT &= ~led;
	}
*/
	//Don't come out of LPM3
}

void main(void) {
	//Turn off watchdog timer
	WDTCTL = WDTPW | WDTHOLD;
	
	//Set all pins to I/O input, pulldown enabled, except for the led which is output.
	P1DIR = led;
	P1REN = ~led;
	P1OUT = 0;
	P1SEL = 0;
	P1SEL2 = 0;

	P2DIR = xout;
	P2REN = ~(xin | xout);
	P2OUT = 0;
	P2SEL = xin | xout;
	P2SEL2 = 0;

  //Only some platforms (like __MSP430G2553) have a port 3
#ifdef __MSP430G2553
	P3DIR = 0;
	P3REN = 0xFF;
	P3OUT = 0;
	P3SEL = 0;
	P3SEL2 = 0;
#endif

	//ACLK divider is 0
	BCSCTL1 |= DIVA_0;
	//12.5pF cap for the crystal oscillator
	BCSCTL3 |= XCAP_3;
	//Select VLO
	//BCSCTL3 |= LFXT1S_2;

	//Configure ACLK

	//Set up Timer A0 to wake us up
	//Set to mode 1 (count up to TACCR0), divide by 8, input is ACLK, interrupt enabled
	TA0CTL = MC_1 | TASSEL_1 | ID_3;

	//There are 32768 / 8 = 4096 ticks per second for the external oscillator
	//Subtract 1 from 4096 because we start from 0 and need to roll over at the end
	TA0CCR0 = 4095;
	//9.5kHz VLO / 8 = 1187.5 ticks
	//TA0CCR0 = 1186;
	//TA0CCR0 = 1293;

	//Enable timer Timer A0 CCR interrupts
	TA0CCTL0 |= CCIE;

	//Enter LPM 3 and have interrupts enabled
	_BIS_SR(LPM3_bits + GIE);
}
