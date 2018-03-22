/*
 * main.c
 */

#include <msp430.h>

/*
 * Programming:
 * msp430-gcc mini-clock.c -mmcu=msp430g2452
 * mspdebug rf2500 "prog a.out"
 */


/*
 * Wiring:
 * 
 * Pins are attached with with the following layout:
 *
 * 7Seg  MSP430 (20 pins)  7Seg
 *       ## Function  ##
 *        1 Vcc  Vss  20
 *        2 1.0  XIN  19
 *        3 1.1  XOUT 18
 * 7      4 1.2  TEST 17
 * 6      5 1.3  RST  16  (7 is wired to the other side)
 * 5      6 1.4  1.7  15   8
 * 4      7 1.5  1.6  14   9
 * 3      8 2.0  2.5  13  10
 * 2      9 2.1  2.4  12  11
 * 1     10 2.2  2.3  11  12
 *
 * This gives the lest number of wires. XIN and XOUT are needed for the crystal
 * and wire 12 from the display is wired around to the other side. This leaves
 * only pins 1.0 and 1.1 for additional I/O.
 *
 * Segment locations:
 *    a
 *    _
 * f |_|  b
 * e |_|. c (g is the middle line)
 *    d  dp
 *
 * 7-segment layout is:
 * segment - pin# - msp port.pin
 * a       - 11   - 2.4
 * b       - 7    - 1.2
 * c       - 4    - 1.5
 * d       - 2    - 2.1
 * e       - 1    - 2.2
 * f       - 10   - 2.5
 * g       - 5    - 1.4
 * dp      - 3    - 2.0
 *
 * Digits go from most significant to least.
 * digit 1 - 12   - 2.3
 * digit 2 -  9   - 1.6
 * digit 3 -  8   - 1.7
 * digit 4 -  6   - 1.3
 *
 * To turn on a digit set its control pin to ground.
 * To turn on a segment set its output to Vcc.
 *
 */

#define digit1on() P2OUT &= ~BIT3
#define digit2on() P1OUT &= ~BIT6
#define digit3on() P1OUT &= ~BIT7
#define digit4on() P1OUT &= ~BIT3
#define digit1off() P2OUT |= BIT3
#define digit2off() P1OUT |= BIT6
#define digit3off() P1OUT |= BIT7
#define digit4off() P1OUT |= BIT3

#define segA  BIT4    //port 2
#define segB  BIT2    //port 1
#define segC  BIT5    //port 1
#define segD  BIT1    //port 2
#define segE  BIT2    //port 2
#define segF  BIT5    //port 2
#define segG  BIT4    //port 1
#define segDP BIT0    //port 2

// Pins for each digit (0-9) split between port 1 and 2
#define digit1p1 (segB | segC)
#define digit1p2 (0)
#define digit2p1 (segB | segG)
#define digit2p2 (segA | segD | segE)
#define digit3p1 (segB | segC | segG)
#define digit3p2 (segA | segD)
#define digit4p1 (segB | segC | segG)
#define digit4p2 (segF)
#define digit5p1 (segC | segG)
#define digit5p2 (segA | segD | segF)
#define digit6p1 (segC | segG)
#define digit6p2 (segA | segD | segE | segF)
#define digit7p1 (segB | segC)
#define digit7p2 (segA)
#define digit8p1 (segB | segC | segG)
#define digit8p2 (segA | segD | segE | segF)
#define digit9p1 (segB | segC | segG)
#define digit9p2 (segA | segD | segF)
#define digit0p1 (segB | segC)
#define digit0p2 (segA | segD | segE | segF)

const unsigned char LED1 = BIT0;
const unsigned char LED2 = BIT6;
const unsigned char xin = BIT6;    // For the crystal oscillator
const unsigned char xout = BIT7;   // Same
const unsigned char button = BIT1; // Input to adjust the time

const unsigned char num_to_pins_1[10] = {digit0p1,
                               digit1p1,
                               digit2p1,
                               digit3p1,
                               digit4p1,
                               digit5p1,
                               digit6p1,
                               digit7p1,
                               digit8p1,
                               digit9p1};
const unsigned char num_to_pins_2[10] = {digit0p2,
                               digit1p2,
                               digit2p2,
                               digit3p2,
                               digit4p2,
                               digit5p2,
                               digit6p2,
                               digit7p2,
                               digit8p2,
                               digit9p2};


unsigned char refresh_rate = 10;
unsigned char hours = 0;
unsigned char minutes = 0;
unsigned char seconds = 0;
// Duty cycle of 1/3 for the four digits so we will count 12 states and have an
// active digit in 4 of them.
const unsigned char max_digit = 4 * 3 - 1;
unsigned char digit = 0;
//Timer A0 interrupts for TAIFG and CCR1 and CCR2
#pragma vector=TIMER0_A1_VECTOR
__interrupt void timer0A1(void) {
	//Check to make sure this is the correct (TACCR1)
  //P1OUT ^= BIT0;
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
    // Find the proper output digit based upon the current digit state
    unsigned char out_digit = 0;
    switch (digit) {
      case 3:
        // Turn on this digit
        digit1off();
        digit4on();
        // Find the number
        out_digit = minutes%10;
        --digit;
        break;
      case 2:
        // Turn on this digit
        digit4off();
        digit3on();
        // Find the number
        out_digit = (minutes/10)%10;
        --digit;
        break;
      case 1:
        // Turn on this digit
        digit3off();
        digit2on();
        // Find the number
        out_digit = hours%10;
        --digit;
        break;
      case 0:
        // Find the number
        out_digit = (hours/10)%10;
        digit = max_digit;
        // Turn on this digit only if the tens hours is not zero
        digit2off();
        if (0 < digit) {
          digit1on();
        }
        break;
      default:
        // Off cycle
        // TODO Should these pins go to ground?
        digit4off();
        digit3off();
        digit2off();
        digit1off();
        --digit;
        break;
    }
    P1OUT |= num_to_pins_1[out_digit];
    P2OUT |= num_to_pins_2[out_digit];
    break;
	case TA0IV_TACCR2:
		break;
	case TA0IV_TAIFG:
		break;
	}
}

// The button will connect P1.1 to P1.3. P1.3 is used as one of the digit
// selects and will be continually changing state to turn that digit on. When
// P1.1 is pulled to ground it must be connected to the digit when it is
// displaying. The exact refresh rate is quite high, so we will disable this
// interrupt and reenable it in the second timer.
char button_count = 0;
char button_pressed = 0;
#pragma vector=PORT1_VECTOR
__interrupt void portInterrupt(void) {
  // Handle button press
  if (P1IFG & button) {
    button_pressed = 1;
    // Clear the flag
    P1IFG &= ~button;
    // Disable the interrupt, it will be reenabled in the timer
    //P1IE &= ~button;
  }
}

// Counting to 24 hours for simplicity
// TODO Check for p1.1 input, increment by 1 minute for the first four seconds and start incrementing by 10 minutes for the next four and 1 hour after that
//Timer A0 for CCR0
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timerA(void) {
  // Increment and modulo by the ranges of the different time units
  seconds = (seconds+1) % 60;
  if (0 == seconds) {
    minutes = (minutes+1) % 60;
    if (0 == minutes) {
      hours = (hours+1)%24;
    }
  }
  // If the button was pressed in this timer interval check if it is still
  // pressed and handle it.
  if (button_pressed) {// && (P1IN & button)) {
    // Clear button press flag
    button_pressed = 0;
    if (button_count < 11) {
      ++button_count;
    }
    char inc = 1;
    if (10 < button_count) {
      inc = 60;
    }
    else if (5 < button_count) {
      // Increment by 5 minutes after 5 ticks
      inc = 5;
    }
    else {
      //++button_state;
    }
    minutes = minutes+inc;
    while (60 < minutes) {
      hours = (hours+1)%24;
      minutes -= 60;
    }
  }
  else {
    // Otherwise clear the consecutive button count
    button_count = 0;
  }
  // Make sure the button interrupt is enabled
  P1IE |= button;
	//Don't come out of LPM3
}

void main(void) {
	//Turn off watchdog timer
	WDTCTL = WDTPW | WDTHOLD;
	
	//Set all pins to I/O input, pulldown enabled, except for the led outputs.
	P1DIR = digit8p1 | segDP | BIT3 | BIT6 | BIT7;
	P1REN = ~P1DIR;
	P1OUT = 0;
	P1SEL = 0;
	P1SEL2 = 0;

	P2DIR = xout | digit8p2 | BIT3;
	P2REN = ~(xin | xout | digit8p2 | BIT3);
	P2OUT = 0;
	P2SEL = xin | xout;
	P2SEL2 = 0;

  digit1off();
  digit2off();
  digit3off();
  digit4off();

  // Enable the button input
	P1DIR &= ~(button);
	//Set up a pull-down resistor, trigger on low to high
  P1OUT &= ~button;
	P1REN |= button;
  P1IES &= ~button;

	//Enable interrupts on the input pin
	P1IE = button;

  //Only some platforms (like __MSP430G2553) have a port 3
//#ifdef __MSP430G2553
#ifdef __MSP430_HAS_PORT3_R__
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
  // Each digit refresh will occur at a rate of refresh_rate / 4096
  // We want to refresh each digit greater than 50 times a second to avoid a
  // flickering effect for human vision
  // We also want about a 33% duty cycle across 4 digits to save energy.
  // Trigger 3 * 4 * 50 times a second = 600 times a second.
  // This gives a value between 6 and 7. Choosing six gives us 56.8Hz.
  refresh_rate = 6;
  TA0CCR1 = refresh_rate;
	TA0CCTL1 |= CCIE;

  /*
  // Debug your wiring by turning on 8s on each segment:
  digit1on();
  digit2on();
  digit3on();
  digit4on();
  P1OUT |= num_to_pins_1[8];
  P2OUT |= num_to_pins_2[8];
  */

	//Enter LPM 3 and have interrupts enabled
	_BIS_SR(LPM3_bits + GIE);
	//Stop the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	while (1)
		;
}
