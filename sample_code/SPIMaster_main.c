/*
 * main.c
 */

#include <msp430.h>

const unsigned char SOMI = BIT1;
const unsigned char SIMO = BIT2;
const unsigned char SCLK = BIT4;

//Set up the port 1 pins
void setMSP430Pins() {
	//Port 1
	P1DIR = SCLK | SIMO; //  All pins to input except SCLK and SIMO
	P1REN = ~(SCLK | SIMO | SOMI); //  Enable all pullup/down resistors except P1.5 (SCLK) and P1.7 (SI)
	P1OUT = 0x00; // Output off, input resistors set to pulldown (including SOMI)
}

void set_UCS() {
	/* DCO Frequency selected */
	/* SCLK=SMCLK  P1.5  */
	/*	Maximum SPI frequency for burst mode is 6.5 MHz	*/
	//Set DCO to 12 MHz calibrated and use DCO/2 for SMCLK
	DCOCTL = CALDCO_12MHZ;		//0x7A  01111010
								//DCOx  011
								//MODx	11010

	/* External Crystal OFF, LFEXT1 is low frequency, ACLK is Divided by 1, RSELx=1110 */
	BCSCTL1 = CALBC1_12MHZ;       // Set range  0x8E  10001110

	/* DCO -> MCLK , MCLK is divided by 2, DCO -> SMCLK, SMCLK is divided by 2, Internal Resistor */
	BCSCTL2 = 0x52;  //  01010010

	/* 0.4 to 1MHz crystal , Using VLO for ACLK, Cap selected 1pF */
	BCSCTL3 = 0x20;
	IE1 &= 0xFD; /* Disable UCS interrupt */

	return;
}

volatile unsigned char input;
//When we receive a read do something
#pragma vector=USCIAB0RX_VECTOR
__interrupt void SPIRX(void) {
	input = UCA0RXBUF;
	//Wake up on successful byte reception
	_bic_SR_register_on_exit(CPUOFF);
}

//Triggered every time a byte is read, go to the next byte in the buffer
#pragma vector=USCIAB0TX_VECTOR
__interrupt void SPITX(void) {
}


void main(void) {
	/* Stop the Watchdog Timer (WDT) */
	WDTCTL = WDTPW + WDTHOLD;

	//Set clock speed and configure pins
	set_UCS();
	setMSP430Pins();

    //These next five lines are for setting up the state machine
    UCA0CTL1 |= UCSWRST; // **Disable USCI state machine**
    UCA0CTL0 |= UCMST + UCCKPH + UCMSB + UCSYNC; // 3-pin, 8-bit SPI master
    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0BR0 = 0x04; // UCLK/4
    UCA0BR1 = 0;
	P1SEL2 = (SCLK | SIMO | SOMI); //  All pins set to I/O function.
	P1SEL = (SCLK | SIMO | SOMI); //  Set P1 to I/O, not special function; Will be changed to SPI later in code

    UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**

    //Only using the receive interrupt
	//IE2 |= UCA0TXIE | UCA0RXIE;
	IE2 |= UCA0RXIE;
	__enable_interrupt();

    while (1) {
    	//Dummy write to initiate read
    	UCA0TXBUF = 0xAA;
		_bis_SR_register(CPUOFF + GIE);

		//The RX interrupt woke us up, read the byte
		volatile unsigned int rxval = input;
		volatile int a = rxval;
    }
}
