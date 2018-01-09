/*
 * main.c
 */

#include <msp430.h>

const unsigned char SOMI = BIT1;
const unsigned char SIMO = BIT2;
const unsigned char SCLK = BIT4;
const unsigned char LED = BIT0;

//Set up the port 1 pins
void setMSP430Pins() {
	//Port 1
	P1DIR = SOMI | LED; //  All pins to input except SOMI
	P1REN = ~(SOMI | SCLK | SIMO | LED); //  Enable all pullup/down resistors except SPI pins and LED
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

//When we receive a read do something
#pragma vector=USCIAB0RX_VECTOR
__interrupt void SPIRX(void) {
}

volatile int counter = 0;
//Triggered every time a byte is read, go to the next byte in the buffer
#pragma vector=USCIAB0TX_VECTOR
__interrupt void SPITX(void) {
	UCA0TXBUF = ++counter;
}


void main(void) {
	/* Stop the Watchdog Timer (WDT) */
	WDTCTL = WDTPW + WDTHOLD;
	/*
	//Set DCO to 12 MHz calibrated and use DCO/2 for SMCLK
	DCOCTL = CALDCO_12MHZ;		//0x7A  01111010
								//DCOx  011
								//MODx	11010

	// External Crystal OFF, LFEXT1 is low frequency, ACLK is Divided by 1, RSELx=1110
	BCSCTL1 = CALBC1_12MHZ;       // Set range  0x8E  10001110
	*/

	set_UCS();
	setMSP430Pins();

    //These next five lines are for setting up the state machine
    UCA0CTL1 |= UCSWRST; // **Disable USCI state machine**
    UCA0CTL0 |= UCCKPH + UCMSB + UCSYNC; // 3-pin, 8-bit SPI slave
    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0BR0 = 0x02; // UCLK/2
    UCA0BR1 = 0;
	P1SEL2 = (SCLK | SIMO | SOMI); //  All pins set to I/O function.
	P1SEL = (SCLK | SIMO | SOMI); //  Set P1 to I/O, not special function; Will be changed to SPI later in code

    P2OUT &= ~BIT0;
    P2DIR &= ~BIT0;

    UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**

	//IE2 |= UCA0TXIE | UCA0RXIE;
    UCA0TXBUF = 0xAA;
	IE2 |= UCA0TXIE;
	__enable_interrupt();

    /*
    //Polling-based data write/read
    UCA0TXBUF = 0; // Dummy write so we can read data
    //Wait for interrupt to fire
    while(!(UCA0CTL0 & TXEPT));                 // Wait for TX complete
    x = UCA0RXBUF;
    */
	volatile unsigned char counter = 0;
    while (1) {
    	//Increment a counter and send it to the master
    	//UCA0TXBUF = 0xAA;
    	//P1OUT ^= LED;
		//_bis_SR_register(CPUOFF + GIE);
    }
}
