/*
 * main.c
 */

#include <msp430.h>


// ADC10 interrupt service routine so we sleep while waiting for the conversion to finish
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
	//Turn the CPU back on
	_BIC_SR_IRQ(CPUOFF);
}

void main(void) {
	//Stop the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	//Set up Pin 1.5 as analog input
	P1DIR &= ~BIT5;
	P1SEL |= BIT5;
	ADC10AE0 |= BIT5;

	//Set up ADC10; source from VCC and VSS,
	//turn on the ADC core, and enable the interrupt
	ADC10CTL0 = SREF_0 | ADC10ON | ADC10IE;
	//Input on channel 5 (PIN 1.5). Divide the clock by 8.
	ADC10CTL1 = INCH_5 + ADC10DIV_3;

	//Ready the interrupts
	__enable_interrupt();

	//Enable sampling/conversion and turn on encoding to begin
	ADC10CTL0 |= ENC | ADC10SC;
	//Stop the CPU and wait for conversion to finish (LPM0)
	_BIS_SR(CPUOFF + GIE);
	//Read the result
	volatile int result = ADC10MEM;
	while (1);
}
