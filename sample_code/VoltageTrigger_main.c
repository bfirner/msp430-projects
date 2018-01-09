/*
 * Uses comparator A+ (CA+) to trigger an interrupt
 * when voltage above a threshold is applied to P1.0.
 */
#include <msp430.h>

volatile int fired = 0;
const unsigned char xin = BIT6;
const unsigned char xout = BIT7;

const unsigned char LEDOUT = BIT4;
const unsigned char LEDIN = BIT0;


volatile uint8_t ambient_val = 0;

#define OPTICAL_DELAY_DURATION 255
/*
 * Interrupt Service Request (ISR) for Timer0_A1-3
 * Used for ambient light sensing (Timer0_A3)
 */
#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void) {

	uint16_t flags = TA0IV;
	uint16_t temp_val = TA0CCR2;
	// LED discharged "enough" so store the value
	if(TA0CCTL2 & CCIE){
		if(temp_val > 0x0F){
			ambient_val = (OPTICAL_DELAY_DURATION+1-temp_val)&0xF0;
		}else {
			ambient_val = 0x10;
		}
		_BIC_SR_IRQ(LPM3_bits);
	}
//	if (TA0CCTL2 & COV) {
//		ambient_val += 0x01000000;
//	}
//	if (TA0CCTL2 & CCIFG) {
//		ambient_val += 0x100000;
//	}
	// Timer_A has "looped" in continuous up mode
//	if (flags & TA0IV_TACCR2) {
//		sensed_val = TA0CCR0;
//		ambient_val += 0x010000;
//	}
//	sensed_val = flags;

}

uint8_t relativeLightLevel(void) {
// light sensing with Rev 1.1, TPIP-K
// returns a relative level: 0-3

	/*to get SMCLK clock on port P1.4
	 P1DIR |= 0x10;
	 P1SEL |= 0x10;
	 P1SEL2 &= ~0x10;
	 */




	ambient_val = 0;

	/*
	 * http://students.cs.byu.edu/~cs124ta/references/HowTos/HowTo_TimerA.html
	 *
	 * The period of Timer_A, when configured to use SMCLK (TASSEL_2)
	 * and set to "up" mode (MC_1), allows periodic outputs to be produced
	 * completely automatic by hardware, without any software intervention.
	 * Thus, the MSP430 can be left undisturbed in low-power mode (LPM0)
	 * while hardware continues to produce outputs (ie, interrupts or
	 * other port outputs).
	 *
	 * In up mode, Timer_A register TAR counts from 0 up to the value in
	 * register TACCR0. TAR is reset to 0 on the next clock transition.
	 * (Thus the period is TACCR0+1 counts.) This makes the outputs of
	 * Timer_A at the same frequency, which is useful for pulse-width
	 * modulation.
	 */

	P2REN &= ~0x01;				//disable resistors for P2.0
	P2OUT &= ~0x01;				//make low when output
	P2DIR |= 0x01;				//make P2.0 output

	P3SEL2 &= ~0x01;

	TA0CCTL2 = 0;	//CLEAR TIMER
	TA0CTL = TACLR;	// Clear counter
	unsigned int i = 0;
	for (i = 0; i < 5; i++)							//delay
			{
	}

//Start Timer_A (SMCLK, Continuous)
	// TASSEL_1 = ACLK
	// TASSEL_2 = SMCLK
	// MC_0 = STOP
	// MC_1 = Up to TACCR0
	// MC_2 = Continuous up
	// ID_1 = Clock divider 2
	// ID_2 = Clock divider 4
	// ID_3 = Clock divider 8
	// 12kHz VLO/ACLK divide 2 * 512 steps ~100ms
	// or divide 4 * 256 steps
	TA0CTL = TASSEL_1 + MC_1 + ID_2;//ACLK , UP to TACCR. Use ISR to count iterations in sleep
	TA0CCR2 = 0;

//Set up CC for capture

	TA0CCTL2 = CM_2 + CCIS_0 + CAP + SCS;
	TA0CCTL2 &= ~CCIE;

	//Setup Output Pin going to the LED
	P3SEL &= ~0x01;							//Select bit has to be 0 to be high
	P3DIR &= ~0x01;
	P3OUT |= 0x01;
	P3REN |= 0x01;						      //enable Pull up resistor for P3.0

	// Delay to fully charge the LED capacitance
	for (i = 0; i < 5; i++) {
	}

	//Clear the CCIFG interrupt flag
	TA0CCTL2 &= ~CCIFG;

	//change pin P3.0 to input
	P3REN &= ~0x01;						//disable resistors for P3.0

	P3SEL |= 0x01;						//Select bits changed for CC

	// Sleep until wake-up.

	TACCR0 = OPTICAL_DELAY_DURATION;

	//TA0CCTL2 |= TAIFG + CCIE + TAIE;
	TA0CCTL2 |= CCIE;

	__bis_SR_register(LPM3_bits + GIE);

	// Turn off Timer_A0 capture compare
	TA0CCTL2 = 0;

	// Make sure pins are "off" with no leakage
	P2REN |= 0x01;	// enable resistors
	P2OUT &= ~0x01;	// pull-down
	P2DIR &= ~0x01;	// Set input
	P2SEL &= ~0x01; // Set I/O function
	P2SEL2 &= ~0x01; // Set I/O function

	P3REN |= 0x01;	// enable resistors
	P3OUT &= ~0x01;	// pull-down
	P3DIR &= ~0x01;	// Set input
	P3SEL &= ~0x01; // Set I/O function
	P3SEL2 &= ~0x01; // Set I/O function


//	sensed_val |= 0x80;

//	i = 0;
//	while (!(TA0CCTL2 & CCIFG))				//while no flag
//	{
//		i++;
//		if (i == OPTICAL_DELAY_DURATION)// Increasing 'i' increases sensitivity to low light
//		{
//			break;
//
//		}
//	}

//	if (TA0CCR2 > OPTICAL_THRESH1) {
//		sensed_val = 1;
//	} else if (TA0CCR2 > OPTICAL_THRESH2) {
//		sensed_val = 2;
//	} else if (TA0CCR2 > 0x0001) {
//		sensed_val = 3;
//	}

	return ambient_val;
}

void main(void) {
	//Stop the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	//Now we need to set up our output pins that will charge through
	//the two resistors
	//According to the family user guide, these two pins should be
	//set to high impedence input with CAPDx set when we aren't
	//measuring from that pinout
	P1REN = 0xFF;	// enable resistors
	P1OUT = 0;	// pull-down
	P1DIR = 0;	// Set input
	P1SEL = 0; // Set I/O function
	P1SEL2 = 0; // Set I/O function
	P2REN = ~(xin | xout);	// enable resistors
	P2OUT = 0;	// pull-down
	P2DIR = xout;	// Set input
	P2SEL = xin | xout; // Set I/O function
	P2SEL2 = 0; // Set I/O function

	//P2DIR |= LED;
	//P2OUT |= LED;
	//P2REN &= ~LED;
	//while (1);

	//Set up ACLK for Timer A but don't start it
	//ACLK divider is 0
	BCSCTL1 |= DIVA_0;
	//12.5pF cap for the crystal oscillator
	BCSCTL3 |= XCAP_3;

	//Set up Timer A0 to wake us up once a second
	//Set to mode 1 (count up to TACCR0), divide by 8, input is ACLK, interrupt enabled
	TA0CTL = MC_1 | TASSEL_1 | ID_3;

	//There are 32768 / 8 = 4096 ticks per second for the external oscillator
	//Subtract 1 from 4096 because we start from 0 and need to roll over at the end
	TA0CCR0 = 4095;

	//Enable timer Timer A0 CCR interrupts
	TA0CCTL0 |= CCIE;

	//Check the light level over and over
	while (1) {
		//Enter LPM 3 and have interrupts enabled
		_BIS_SR(LPM3_bits + GIE);

		volatile unsigned charlight_level = relativeLightLevel();
		volatile int x = 3;
	}
}
