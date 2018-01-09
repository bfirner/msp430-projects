/*
 * Transmission side of a simple 3 wire serial interface.
 *
 * Bernhard Firner, February 2014
 */
#include <msp430.h>

#define CLK BIT0
#define IOWIRE BIT4
#define SEL BIT5
#define DEBUG BIT6

//The bytes to send and the buffer to take them from
volatile int bytesToSend = 0;
volatile char* outBuf = 0;

//Sends a msg of given length. Returns the number of bytes sent
int sendData(char* msg, int length) {
	//Can't start sending new data if we are already sending something
	if (0 != bytesToSend) {
		return 0;
	}
	outBuf = msg;
	bytesToSend = length;
	//Wait for the interrupt to finish sending the data
	while (0 != bytesToSend);
	return length;
}

//Whether or not data is being sent
volatile char sending = 0;
//Bit index of the byte currently being sent
//Start with the MSB
volatile signed char bit_index = 7;
volatile char clock_on = 0;

//Set up a watchdog timer interrupt that toggles the LED
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void) {
	P1OUT ^= CLK;
	//Change data output on falling edge
	if (!(P1OUT & CLK)) {
		if (!sending && 0 < bytesToSend) {
			P1OUT &= ~SEL;
			sending = 1;
		}
		if (sending) {
			//Output high on 1, low on 0
			if ( *outBuf & (0x01 << bit_index)) {
				P1OUT |= IOWIRE | DEBUG;
			}
			else {
				P1OUT &= ~(IOWIRE | DEBUG);
			}
			//Go to the next byte after sending 8 bits
			if (0 > --bit_index) {
				bit_index = 7;
				++outBuf;
				//Put wire high again if we are done
				if (0 == --bytesToSend) {
					P1OUT |= SEL;
					sending = 0;
				}
			}
		}
	}
	WDTCTL = WDT_MDLY_32;
}

/*
 * main.c
 */
void main(void) {
	//Stop the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	//Set pins to regular I/O, input, turn on pulldown resistors
	//P1SEL2 = 0;
	//P1SEL = 0;
	//Enable Clock, Out, and Select as output
	P1DIR = CLK | IOWIRE | SEL | DEBUG;
	P1REN = 0xFF;
	//Select starts high
	P1OUT = SEL;

	//P2SEL2 = 0;
	//P2SEL = 0;
	P2DIR = 0;
	P2REN = 0xFF;
	P2OUT = 0x0;

	BCSCTL1 = CALBC1_12MHZ;
	DCOCTL = CALDCO_12MHZ;

	//BCSCTL1 = CALBC1_1MHZ;
	//DCOCTL = CALDCO_1MHZ;

	//Enable the watchdog timer interrupt and global interrupts
    IE1 |= WDTIE;
    __enable_interrupt();
    //32ms timer from SMCLK
    WDTCTL = WDT_MDLY_32;

    //char* hi = "hello world!";
    //sendData(hi, 12);
    //10110010 is a good test byte
    //char test = 0xB2;
    //sendData(&test, 1);
    char test[2];
    test[0] = 0xB2;
    test[1] = 0xAA;
    sendData(test, 2);
    while(1);
}
