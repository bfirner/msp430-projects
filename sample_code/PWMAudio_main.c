/*
 * Generate different audio tones using pulse width modulation.
 */

#include <msp430.h>

//Pins used by the crystal oscillator
const unsigned char xin = BIT6;
const unsigned char xout = BIT7;

//The output and ground pin for the PWM output
const unsigned char led = BIT0;
const unsigned char out = BIT6;


const unsigned char POWER = BIT5;
const unsigned char BUTTON = BIT3;


//8 bit depth, 0 to 255
const unsigned char max_amplitude = 255;

//16000 steps per second
//Actually 15625 steps per second

//The spacing of nerves on the basilar membrane (our hearing organ)
//detects logarithmic changes (not spaced absolutely, the ratio of
//frequency detected by one nerve group to the next is the same
//across the organ).
//Double a note's frequency to go up an octave,
//half it to go down an octave
//Since there are 12 half-steps per octave, multiple
//a note's frequency by about 1.06 to go up a note
//There are 7 full steps (A, B, C, D, E, F, G) but
//only 5 half steps (A#/Bb, C#/Db, D#/Eb, F#/Gb, G#/Ab).
//"Concert A" is A4, which is 440Hz in modern music
/*
const unsigned int a4 = 440;
const unsigned int b4 = 494;
const unsigned int c5 = 523;
const unsigned int d5 = 587;
const unsigned int e5 = 659;
const unsigned int f5 = 698;
const unsigned int g5 = 784;
*/
#define rest 3

#define c3 131
#define c3s 139
#define d3 147
#define d3s 156
#define e3 165
#define f3 175
#define f3s 185
#define g3 196
#define g3s 208

#define a3 220
#define a3s 233
#define b3 247
#define c4 262
#define c4s 277
#define d4 294
#define d4s 311
#define e4 330
#define f4 349
#define f4s 370
#define g4 392
#define g4s 415

#define a4 440
#define a4s 466
#define b4 494
#define c5 523
#define c5s 554
#define d5 587
#define d5s 622
#define e5 659
#define f5 698
#define f5s 740
#define g5 784
#define g5s 831
#define a5s 932
#define b5 988

const unsigned int a_major[] = {a4, b4, c5s, d5, e5, f5s, g5s};
const unsigned int twinkle_twinkle[] = {
		a4, a4, e5, e5, f5s, f5s, e5, d5, d5, c5s, c5s, b4, b4, a4,
		f5s, f5s, e5, e5, d5, d5, c5s, f5s, f5s, e5, e5, d5, d5, c5s,
		a4, a4, e5, e5, f5s, f5s, e5, d5, d5, c5s, c5s, b4, f5s, a4
};
const unsigned int twinkle_durations[] = {
		1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2,
		1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2,
		1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2
};
//Total number of notes in the song
//const unsigned int total_notes = 42;
//const unsigned int quarter_note = 4000;


const unsigned int irish_washerwoman[] = {
		b4, g4, g4, d4, g4, g4, b4, g4, b4, d5, c5, b4,
		c5, a4, a4, e4, a4, a4, c5, a4, c5, e5, d5, c5,
		b4, g4, g4, d4, g4, g4, b4, g4, b4, d5, c5, b4,
		c5, b4, c5, a4, d5, c5, b4, g4, g4, g4, //End of part 1
		b4, g4, g4, d4, g4, g4, b4, g4, b4, b4, a4, g4,
		a4, f4, f4, d4, f4, f4, a4, f4, a4, a4, g4, f4,
		e4, g4, g4, d4, g4, g4, c4, g4, g4, b3, g4, g4,
		c5, b4, c5, a4, d5, c5, b4, g4, g4, g4, //End of part 2
		b4, g4, g4, d4, rest, g4, b4, g4, b4, d5, c5, b4,
		c5, a4, a4, e4, rest, a4, c5, a4, c5, e5, d5, c5,
		b4, g4, g4, d4, g4, g4, b4, g4, b4, d5, c5, b4,
		c5, b4, c5, a4, d5, c5, b4, g4, g4, g4, //End of part 1
		b4, g4, g4, d4, g4, g4, b4, g4, b4, b4, a4, g4,
		a4, f4, f4, d4, f4, f4, a4, f4, a4, a4, g4, f4,
		e4, g4, g4, d4, g4, g4, c4, g4, g4, b3, g4, g4,
		c5, b4, c5, a4, d5, c5, b4, g4, g4, g4
};
const unsigned int washerwoman_durations[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 3,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 3,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 3,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 3
};
//Total number of notes in the song
unsigned int total_notes = sizeof(irish_washerwoman) / sizeof(unsigned int);
const unsigned int quarter_note = 1953;

//Will list the zelda theme in 8th notes, it is in 4/4 time
const unsigned int zelda_theme[] = {
		a4s, rest, a4s, a4s, a4s, a4s,
		a4s, g4s, a4s, rest, a4s, a4s, a4s, a4s,
		a4s, g4s, a4s, rest, a4s, a4s, a4s, a4s,
		a4s, g3s, g3s, g3s, g3s, g3s, g3s, g3s, g3s, g3s, g3s,
		a4s, f3, f3, a4s, a4s, c5, d5, d5s,
		f5, rest, f5, f5, f5s, g5s,
		a5s, rest, a5s, a5s, a5s, g5s, f5s
};
const unsigned int zelda_durations[] = {
		36, 16, 8, 8, 8, 8,
		7, 5, 12, 16, 8, 8, 8, 8,
		7, 5, 12, 16, 8, 8, 8, 8,
		8, 5, 5, 8, 5, 5, 8, 5, 5, 9, 9,
		18, 18, 10, 8, 8, 9, 8, 9,
		36, 16, 8, 8, 8, 8,
		36, 8, 8, 8, 8, 8, 8
};
//Total number of notes in the song
//unsigned int total_notes = sizeof(zelda_theme)/sizeof(unsigned int);
//Theme is 150 bpm, so 15625ticks_per_second/(150bpm*4quarter_per_beat/60seconds_per_minute)
//So each quarter note is 1562 in duration, but we need to divide the 4/4 space into
//72 parts to fit all of the different duration notes; 1562.5/72 ~= 22
//const unsigned int quarter_note = 44;





unsigned int cur_note = 0;
unsigned int clock = 0;

//Fill the sound output with an approximated sine wave at this frequency
unsigned char getAmpl(unsigned int freq) {
	unsigned int period = (15625 / freq);
	return (clock % period) * ((max_amplitude/2) / period);
}

//Interrupt for TAR and TACCR1/2
#pragma vector=TIMER0_A1_VECTOR
__interrupt void intA1(void) {
	//Check to make sure this is the correct (TACCR1)
	switch (TA0IV) {
	case TA0IV_NONE:
		break;
	case TA0IV_TACCR1:
		if (rest != irish_washerwoman[cur_note]) {
			//TA0CCR1 = getAmpl(twinkle_twinkle[cur_note]);
			TA0CCR1 = getAmpl(irish_washerwoman[cur_note]);
			//TA0CCR1 = getAmpl(zelda_theme[cur_note]);
		}
		else {
			TA0CCR1 = 0;
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

	//if (clock == twinkle_durations[cur_note]*quarter_note)
	if (clock == washerwoman_durations[cur_note]*quarter_note) {
	//if (clock == zelda_durations[cur_note]*quarter_note) {
		clock = 0;
		cur_note = (cur_note + 1) % total_notes;
	}
}


#pragma vector=PORT1_VECTOR
__interrupt void p1_interrupt(void) {
	P1OUT ^= led;
	//Toggle interrupt edge select
	P1IES ^= BUTTON;
	P1IFG &= ~BUTTON;
}

void main(void) {
	//Turn off the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	//Initialize pins
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

	//Set up the gnd and out pin the pwm
	P1DIR &= ~(BUTTON);
	P1DIR |= out | led;
	//Pull down gnd pin
	P1REN |= BUTTON;
	P1REN &= ~(out | led);
	P1OUT &= ~(out | led);
	P1OUT |= BUTTON;
	//TA0.1 output
	P1SEL |= out;

	P1IE |= BUTTON;
	P1IFG &= ~BUTTON;

	//Set up fasteset speed for DCO
	//Make sure we are running with DCO at 16MHz
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL = CALDCO_16MHZ;

	//Use TACCR0 to specify the pulse period
	//Use TACCR1 to specify pulse durations

	//Set up 16KHz rate from TimerA
	TA0CCR0 = max_amplitude;
	TA0CCTL0 |= CCIE;

	//Now we set up the TACCR1CTL register
	//Set up PWM output, turn high when the counter resets, low when TAR == TACCR1
	//Enable interrupts so that we can change the value of the TACCR1 register
	TA0CCTL1 = OUTMOD_7 | CCIE;
	TA0CCR1 = 0;

	//Up mode (count to TACCR0), divide input by 4 to get 4MHz SMCLK as input
	TA0CTL = MC_1 | ID_2 | TASSEL_2;

	__enable_interrupt();
}
