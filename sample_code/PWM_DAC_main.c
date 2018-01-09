/*
 * Generate sine wave output
 * See TI document SLAA116 for the general reasoning behind these actions
 */

#include <msp430.h>
#include <math.h>

//Output on P1.2, which will be used for TA0.1
const unsigned char out = BIT6;
const unsigned char led = BIT0;

//Bit 4 of port 2 (P2.4)
const unsigned char sound_out = BIT4;

//8 bit depth, 0 to 255
const unsigned char max_amplitude = 255;
const unsigned char half_amplitude = 127;

unsigned char do_saw = 0;

//16KHz signal, we want to output 440Hz signal
//So 440 times a second we go from 0 to PI
//This is about 36 steps
unsigned int count = 0;
//For 440Hz
//#define period 35
#define period 200
//#define period 31999.0
//#define period 3200
const float step_size = (1.0/period)*2*3.14159;

#define pi 3.14159
#define pi_squared pi*pi
const float four_over_pi = 4/pi;
const float four_over_pi_squared = 4/pi_squared;
float fast_sin(float x) {
	return (four_over_pi)*x - (four_over_pi_squared)*x*abs(x);
}

//0 to 255 in 16000 steps
const float sawtooth_step = 255.0 / 15999.0;
unsigned int sawtooth_count = 0;

unsigned int clock = 0;

//Interrupt for TAR and TACCR1/2
#pragma vector=TIMER0_A1_VECTOR
__interrupt void intA1(void) {
	//Check to make sure this is the correct (TACCR1)
	switch (TA0IV) {
	case TA0IV_NONE:
		break;
	case TA0IV_TACCR1:
		++count;
		if (0 == count%36) {
			TACCR1 = TACCR0;
		}
		else if (1 == count%36) {
			TACCR1 = TACCR0 / 2;
		}
		else if (2 == count%36) {
			TACCR1 = TACCR0 / 3;
		}
		else if (3 == count%36) {
			TACCR1 = TACCR0 / 4;
		}
		else {
			TACCR1 = 0;
		}
		if (do_saw) {
			TA0CCR1 = sawtooth_count * sawtooth_step;
			++sawtooth_count;
			if (sawtooth_count >= 16000) {
				sawtooth_count = 0;
				P1OUT ^= led;
			}
		}
		else {
			//For sine wav output
			//TA0CCR1 = half_amplitude*sin(count * step_size) + half_amplitude;
			TA0CCR1 = half_amplitude*fast_sin(count * step_size) + half_amplitude;
			//TA0CCR1 = half_amplitude*fast_sin((clock%period) * step_size) + half_amplitude;
			if (TA0CCR1 > TA0CCR0) {
				TA0CCR1 = TA0CCR0;
			}
			++count;
			if (count > period) {
				count = 0;
				P1OUT ^= led;
			}
		}
		break;
	case TA0IV_TACCR2:
		break;
	case TA0IV_TAIFG:
		break;
	}
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void intA0(void) {
	++clock;
}

void main(void) {
	//Turn off the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	//If P2.3 and 2.4 are connected then do the saw wave
	P2DIR |= BIT3;
	P2OUT |= BIT3;
	if (P2IN & BIT4) {
		do_saw = 1;
	}

	//Set up the sine wav output
	P1DIR = out | led;
	P1OUT = 0;
	//Set the output pin to be TA0.1
	P1SEL = out;

	//Clock settings
	//We want to generate the sine wave with 8 bit amplitude resolution (2^8 ticks)
	//We want to 16KHz time resolution (roughly 2^14), which is cd quality
	//We want 2^8 bit depth, so that gives us a trigger rate of 2^22, about 4MHz
	//The next highest value for the DCO is 8MHz and we'll divide by 2
	TA0CCR0 = max_amplitude;
	TA0CCTL0 |= CCIE;

	//Make sure we are running with DCO at 16MHz
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL = CALDCO_16MHZ;

	//Now we set up the TACCR1CTL register
	//Set up PWM output, turn high when the counter resets, low when TAR == TACCR1
	//Enable interrupts so that we can change the value of the TACCR1 register
	TA0CCTL1 = OUTMOD_7 | CCIE;
	TA0CCR1 = 0;

	//Set up the TA0CCR2 register
	//TA0CCTL2 |= CCIE;
	//TA0CCR2 = max_amplitude;

	//Up mode (count to TACCR0), divide input by 2, use 8MHz SMCLK as input
	TA0CTL = MC_1 | ID_2 | TASSEL_2;

	__enable_interrupt();
  while(1);
}
