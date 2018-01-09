/*
 * Uses comparator A+ (CA+) to perform resistance measuring
 * through slope comparison using one resistor of known
 * value and a small capacitor. The known resistor should
 * be connected to pin 1.4 and the unknown resistor to
 * pin 1.5. Input reference of the comparator will be
 * set to 1/4 VCC and input to CA+ will be through P1.1.
 */
#include <msp430.h>

volatile int fired = 0;

//CA+ interrupt vector
//Stop the timer and exit LMP0
#pragma vector=COMPARATORA_VECTOR
__interrupt void CAVect(void) {
	//Turn the CPU back on
	fired = 1;
	if (CACTL1 & CAIFG) { //0 == (CACTL2 & CAOUT)
		//Turn the CPU back on
		_BIC_SR_IRQ(CPUOFF);
	}
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void timerA(void) {
	//Turn the CPU back on
	_BIC_SR_IRQ(CPUOFF);
}

void enableChargingPin(unsigned char pin) {
	//Enable digital functionality and output
	CAPD &= ~(pin);
	P1DIR |= pin;
}

void disableChargingPin(unsigned char pin) {
	//Disable digital functionality and output
	CAPD |= pin;
	P1DIR &= ~pin;
}

//Measure how long it takes to discharge after charging the cap
//using this pin
unsigned char timeDischarge(unsigned char pin) {
	//Keep trying this until the CA+ interrupt doesn't fire
	do {
		//Wait a millisecond for the cap to fully discharge
		//Enable Timer A0 CCR interrupts and count UP mode
		TA0CCTL0 |= TACLR | CCIE;
		TA0CCR0 = 33;
		//Disable CPU but leave interrupts enabled
		_BIS_SR(CPUOFF + GIE);
		//Turn off the interrupt
		TA0CCR0 = 0;
		TA0CCTL0 &= ~CCIE;
		TA0CCTL0 |= TACLR;

		//Now charge the cap for aset time
		P1OUT |= pin;
		//Wait for charging to complete
		//The charge time should be a multiple of tau = R_ref * C
		//5*tau for 1% accuracy, 7*tau for 0.1% accuracy
		//100 Ohm ref * 1uF cap = 0.0001, or 3.2768 crystal cycles
		//7 * 3.2768 = 22.9376
		//5 * 3.2768 = 16.384

		//Clear timer and enable Timer A0 CCR interrupts
		TA0CCTL0 |= TACLR;
		TA0CCR0 = 32;
		TA0CCTL0 |= CCIE;
		//Disable CPU but leave interrupts enabled
		_BIS_SR(CPUOFF + GIE);
		//Turn off the interrupt
		TA0CCR0 = 0;
		TA0CCTL0 &= ~CCIE;
		P1OUT &= ~pin;

		//Now count how long it takes for the CA+ interrupt to fire
		//Restarting counting up to one second
		TA0CCTL0 |= TACLR;
		TA0CCR0 = 32768;
		//Turn on CA+ interrupts
		fired = 0;
		CACTL1 |= CAIE | CAON;
		//Repeats loop if the interrupt already triggered with no delay
	} while (1 == fired);
	_BIS_SR(CPUOFF + GIE);
	//Now turn things back off
	TA0CCR0 = 0;

	//Record the time in clock ticks
	volatile unsigned int time = TAR;
	CACTL1 &= ~(CAIE | CAON);
	TA0CCTL0 |= TACLR;
	return time;
}

void main(void) {
	//Stop the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	//Set up the comparator.
	//1/4VCC reference on - input, falling edge interrupt
	//CA+ is off until we set up each measurement and set the CAON bit.
	CACTL1 = CAREF_1 | CARSEL | CAIE | CAIES;

	//Now set up the + input, and turn on the output filter
	//+ input is CA0, P1.0
	CACTL2 = P2CA0 | CAF;

	//Bit 1 needs to be set for CA+ input (this disables digital functionality)
	//Also set this for the two testing pins to disable them for now
	CAPD = CAPD0 | CAPD4 | CAPD5;

	//Now we need to set up our output pins that will charge through
	//the two resistors
	//According to the family user guide, these two pins should be
	//set to high impedence input with CAPDx set when we aren't
	//measuring from that pinout
	P1OUT = 0;
	P1DIR = 0;
	P1SEL = 0;
	P1SEL2 = 0;

	//Set up ACLK for Timer A but don't start it
	//ACLK divider is 0
	BCSCTL1 |= DIVA_0;
	//12.5pF cap for the crystal oscillator
	BCSCTL3 |= XCAP_3;

	//Configure ACLK
	//Set to mode 1 (count up to TACCR0) input is ACLK
	TA0CTL = TASSEL_1 | MC_1;

	__enable_interrupt();

	while(1) {
		enableChargingPin(BIT4);
		volatile unsigned int meas_time = timeDischarge(BIT4);
		disableChargingPin(BIT4);

		enableChargingPin(BIT5);
		volatile unsigned int ref_time = timeDischarge(BIT5);
		disableChargingPin(BIT5);

		volatile float ratio = 100 * (float)meas_time/ref_time;
	}
}
