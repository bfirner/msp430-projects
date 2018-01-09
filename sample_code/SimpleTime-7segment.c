/*
 * main.c
 */

#include <msp430.h>


/*
 * Segment locations:
 *    a
 *    _
 * f |_|  b
 * e |_|. c (g is the middle line)
 *    d  dp
 * 
 * Pins are attached with P1.0-1.5 for segment pins 1-6
 * and P2.0-2.5 for segment pins 7-12
 *
 * 7-segment layout is:
 * segment - pin# - msp port.pin
 * a       - 11   - 2.4
 * b       - 7    - 2.0
 * c       - 4    - 1.3
 * d       - 2    - 1.1
 * e       - 1    - 1.0
 * f       - 10   - 2.3
 * g       - 5    - 1.4
 * dp      - 3    - 1.2
 *
 * digit 1 - 12
 * digit 2 -  9
 * digit 3 -  8
 * digit 4 -  6
 *
 * To turn on a digit set its control pin to ground.
 *
 */

#define digit4on() P2OUT &= ~BIT5
#define digit3on() P2OUT &= ~BIT2
#define digit2on() P2OUT &= ~BIT1
#define digit1on() P1OUT &= ~BIT5
#define digit4off() P2OUT |= BIT5
#define digit3off() P2OUT |= BIT2
#define digit2off() P2OUT |= BIT1
#define digit1off() P1OUT |= BIT5

// Pins for each digit (0-9) split between port 1 and 2
#define digit1p1 (BIT3)
#define digit1p2 (BIT0)
#define digit2p1 (BIT0 | BIT1 | BIT4)
#define digit2p2 (BIT0 | BIT4)
#define digit3p1 (BIT1 | BIT3 | BIT4)
#define digit3p2 (BIT0 | BIT4)
#define digit4p1 (BIT3 | BIT4)
#define digit4p2 (BIT0 | BIT3)
#define digit5p1 (BIT1 | BIT3 | BIT4)
#define digit5p2 (BIT3 | BIT4)
#define digit6p1 (BIT0 | BIT1 | BIT3 | BIT4)
#define digit6p2 (BIT3 | BIT4)
#define digit7p1 (BIT3)
#define digit7p2 (BIT0 | BIT4)
#define digit8p1 (BIT0 | BIT1 | BIT3 | BIT4)
#define digit8p2 (BIT0 | BIT3 | BIT4)
#define digit9p1 (BIT3 | BIT4)
#define digit9p2 (BIT0 | BIT3 | BIT4)
#define digit0p1 (BIT0 | BIT1 | BIT3)
#define digit0p2 (BIT0 | BIT3 | BIT4)

const unsigned char LED1 = BIT0;
const unsigned char LED2 = BIT6;
const unsigned char xin = BIT6;
const unsigned char xout = BIT7;

unsigned char num_to_pins_1[10] = {digit0p1,
                               digit1p1,
                               digit2p1,
                               digit3p1,
                               digit4p1,
                               digit5p1,
                               digit6p1,
                               digit7p1,
                               digit8p1,
                               digit9p1};
unsigned char num_to_pins_2[10] = {digit0p2,
                               digit1p2,
                               digit2p2,
                               digit3p2,
                               digit4p2,
                               digit5p2,
                               digit6p2,
                               digit7p2,
                               digit8p2,
                               digit9p2};

const unsigned char refresh_rate = 10;
int count = 0;
unsigned char digit = 0;
//Timer A0 interrupts for TAIFG and CCR1 and CCR2
#pragma vector=TIMER0_A1_VECTOR
__interrupt void timer0A1(void) {
	//Check to make sure this is the correct (TACCR1)
	switch (TA0IV) {
	case TA0IV_NONE:
		break;
	case TA0IV_TACCR1:
    // Prepare for the next interrupt
    TA0CCR1 = TACCR1 + refresh_rate;
    if (TACCR1 > TACCR0) {
      TACCR1 -= TACCR0;
    }
    // Turn off the last output
    P1OUT &= ~(digit8p1);
    P2OUT &= ~(digit8p2);
    int num = count;
    switch (digit) {
      case 0:
        // Turn on this digit
        digit4off();
        digit1on();
        // Find the number
        num = num%10;
        digit = 1;
        break;
      case 1:
        // Turn on this digit
        digit1off();
        digit2on();
        // Find the number
        num = (num/10)%10;
        digit = 2;
        break;
      case 2:
        // Turn on this digit
        digit2off();
        digit3on();
        // Find the number
        num = (num/100)%10;
        digit = 3;
        break;
      case 3:
        // Turn on this digit
        digit3off();
        digit4on();
        // Find the number
        num = (num/1000)%10;
        digit = 0;
        break;
    }
    P1OUT |= num_to_pins_1[num];
    P2OUT |= num_to_pins_2[num];
    break;
	case TA0IV_TACCR2:
		break;
	case TA0IV_TAIFG:
		break;
	}
}

//Timer A0 for CCR0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timerA(void) {
  // Increment and modulo by 5 digits since this is a 4 digit display
  count = (count+1) % 10000;
	//Don't come out of LPM3
  //P1OUT ^= BIT0;
}

void main(void) {
	//Turn off watchdog timer
	WDTCTL = WDTPW | WDTHOLD;
	
	//Set all pins to I/O input, pulldown enabled, except for the led outputs.
	P1DIR = digit8p1 | BIT5;
	P1REN = ~(digit8p1 | BIT5);
	P1OUT = 0;
	P1SEL = 0;
	P1SEL2 = 0;

	P2DIR = xout | digit8p2 | BIT1 | BIT2 | BIT5;
	P2REN = ~(xin | xout | digit8p2 | BIT1 | BIT2 | BIT5);
	P2OUT = 0;
	P2SEL = xin | xout;
	P2SEL2 = 0;

  digit1off();
  digit2off();
  digit3off();
  digit4off();

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

	//Set up Timer A0 to wake up from LPM
	//Set to mode 1 (count up to TACCR0), divide by 8, input is ACLK, interrupt enabled
	TA0CTL = MC_1 | TASSEL_1 | ID_3;

	//There are 32768 / 8 = 4096 ticks per second for the external oscillator
	//Subtract 1 from 4096 because we start from 0 and need to roll over at the end
	TA0CCR0 = 4095;

	//Enable timer Timer A0 CCR interrupts
	TA0CCTL0 |= CCIE;
  //Also enable CCR1 interrupts for the display at the given refresh rate
  TA0CCR1 = refresh_rate;
	TA0CCTL1 |= CCIE;

	//Enter LPM 3 and have interrupts enabled
	_BIS_SR(LPM3_bits + GIE);
	//Stop the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	while (1)
		;
}
