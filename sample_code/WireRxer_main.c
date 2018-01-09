/*
 * Reception side of a simple 3 wire serial interface.
 *
 * Bernhard Firner, February 2014
 */
#include <msp430.h>

#define CLK BIT3
#define IOWIRE BIT4
#define SEL BIT5

#define ZERO BIT0
#define ONE  BIT6

//The bytes to send and the buffer to take them from
int bytesToRead = 0;
volatile char* inBuf = 0;
char idle = 1;

//Read a msg of at most max_len characters. Returns the number of bytes read
int getData(char* buffer, int maxLen) {
	//Can't start receiving if the buffer has no space
	//or we aren't idle
	if (!idle || 0 >= maxLen) {
		return 0;
	}
	bytesToRead = maxLen;
	inBuf = buffer;
	//Wait for a message to come (we leave idle) and to finish (we return to idle)
	while (1 == idle);
	while (0 == idle);
	return maxLen - bytesToRead;
}

//Set up a watchdog timer interrupt that toggles the LED
#pragma vector=PORT1_VECTOR
__interrupt void p1_interrupt(void) {
	//Whether or not data is being received
	static char receiving = 0;
	//Bit index of the byte currently being received
	static signed char bit_index = 7;
	//Interrupt is triggered on the rising edge
	//Clear the interrupt flag
	P1IFG &= ~(CLK);
	//Check to see if we are ready to receive bytes,
	//aren't receiving right now, but master indicates
	//it will send data
	//Note that there is a race condition here
	//since the getData function could be called
	//in the middle of a transmission by the master
	if (0 < bytesToRead && !receiving) {
		//SEL low means there is data
		if (!(P1IN & SEL)) {
			receiving = 1;
			bit_index = 7;
			idle = 0;
		}
		else {
			idle = 1;
		}
	}
	//Read a bit if we are receiving
	if (receiving) {
		//Check the next bit
		volatile unsigned char in_bit = 0;
		P1OUT |= ZERO;
		P1OUT &= ~ONE;
		if (P1IN & IOWIRE) {
			in_bit = 1;
			P1OUT |= ONE;
			P1OUT &= ~ZERO;
		}
		*inBuf |= in_bit<<bit_index;
		if (bit_index == 5) {
			bytesToRead = 2;
		}
		//Go to the next byte after receiving 8 bits
		if (0 > --bit_index) {
			bit_index = 7;
			//Need to check wire again between bytes
			++inBuf;
			--bytesToRead;
			//Force idle if we don't have buffer space
			if (0 == bytesToRead) {
				idle = 1;
			}
			//Need to check SEL wire again
			receiving = 0;
		}
	}
}

/*
 * main.c
 */
void main(void) {
	//Stop the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	BCSCTL1 = CALBC1_12MHZ;
	DCOCTL = CALDCO_12MHZ;

	//Set pins to regular I/O, input, turn on pulldown resistors
	P1SEL2 = 0;
	P1SEL = 0;
	P1DIR = 0;
	P1REN = 0xFF;
	P1OUT = 0x0;

	P2SEL2 = 0;
	P2SEL = 0;
	P2DIR = 0;
	P2REN = 0xFF;
	P2OUT = 0x0;

	//Flash the LEDs to indicate incoming bits
	P1DIR |= ZERO | ONE;

	//Enable Clock, In, and Select as input
	P1DIR &= ~(CLK | IOWIRE | SEL);
	//No resistor since high/low logic is driven by other side
	P1REN &= ~(CLK | IOWIRE | SEL);
	//Rising edge interrupt triggering
	P1IES &= ~(CLK);
	P1IE |= CLK;

	//Clear the interrupt flag
	//P1IFG &= ~(CLK);
    __enable_interrupt();

    char input[15] = {0};
    int msg_len = getData(input, 15);
    P1OUT &= ~(ZERO | ONE);
    while (1);
}
