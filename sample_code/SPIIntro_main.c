/*
 * main.c
 */
#include <msp430.h>
//#include "spi.h"

// SPI Pins
#define SPI_USCIA0_PxSEL  P1SEL
#define SPI_USCIA0_PxSEL2  P1SEL2

#define SPI_USCIA0_PxDIR  P1DIR
#define SPI_USCIA0_PxIN   P1IN
#define SPI_USCIA0_SIMO   BIT2
#define SPI_USCIA0_SOMI   BIT1
#define SPI_USCIA0_UCLK   BIT4

#define CSn_PxOUT         P1OUT
#define CSn_PxDIR         P1DIR
#define CSn_PIN           BIT5

const unsigned char LED = BIT1;

int in_ready = 0;
int in_val = 0;

//When we receive a read do something
#pragma vector=USCIAB0RX_VECTOR
__interrupt void SPIRX(void) {
	in_ready = 1;
	in_val = UCA0RXBUF;
}

//Triggered every time a byte is read, go to the next by in the buffer
#pragma vector=USCIAB0TX_VECTOR
__interrupt void SPITX(void) {
	UCA0TXBUF = 0xBE;
}




//Set up SPI bus as the master
void SPISetupMaster()
{
    //Assert CSn high from master (disable slave)
    CSn_PxOUT |= CSn_PIN;
    CSn_PxDIR |= CSn_PIN;

    //These next five lines are for setting up the state machine
    UCA0CTL1 |= UCSWRST; // **Disable USCI state machine**
    UCA0CTL0 |= UCMST + UCCKPH + UCMSB + UCSYNC; // 3-pin, 8-bit SPI master
    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0BR0 = 0x02; // UCLK/2
    UCA0BR1 = 0;

    //Configure the pins on the MSP430G255. These need the special select since
    //their functions are overloaded.
    SPI_USCIA0_PxSEL |= SPI_USCIA0_SIMO | SPI_USCIA0_SOMI | SPI_USCIA0_UCLK;
    SPI_USCIA0_PxSEL2 |= SPI_USCIA0_SIMO | SPI_USCIA0_SOMI | SPI_USCIA0_UCLK;

    //Set the proper pins as output or input
    SPI_USCIA0_PxDIR |= SPI_USCIA0_SIMO | SPI_USCIA0_UCLK;
    SPI_USCIA0_PxDIR &= ~SPI_USCIA0_SOMI;

    //Make sure that pullup/pulldown is not enabled for the input pin
    //P1REN &= ~(SPI_USCIA0_SOMI);
    P1REN |= SPI_USCIA0_SOMI;
    UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
}

//Set up SPI bus as the slave
void SPISetupSlave()
{
    //First set chip select to the W5100 by setting CSn output
    CSn_PxOUT &= ~CSn_PIN;
    CSn_PxDIR &= ~CSn_PIN;

    //These next five lines are for setting up the state machine
    UCA0CTL1 |= UCSWRST; // **Disable USCI state machine**
    UCA0CTL0 = UCCKPH + UCMSB + UCSYNC; // 3-pin, 8-bit SPI slave
    UCA0CTL1 |= UCSSEL_2; // SMCLK as clock source
    //Baud rate may not matter in slave mode
    UCA0BR0 = 0x02; // UCLK/2
    UCA0BR1 = 0;

    //Configure the pins on the MSP430G255. These need the special select
    //to configure them for SPI operation
    SPI_USCIA0_PxSEL |= SPI_USCIA0_SIMO | SPI_USCIA0_SOMI | SPI_USCIA0_UCLK;
    SPI_USCIA0_PxSEL2 |= SPI_USCIA0_SIMO | SPI_USCIA0_SOMI | SPI_USCIA0_UCLK;

    //Set the proper pins as output or input (P1.0 is connected to MISO)
    SPI_USCIA0_PxDIR &= ~(SPI_USCIA0_SIMO | SPI_USCIA0_UCLK | BIT0);
    SPI_USCIA0_PxDIR |= SPI_USCIA0_SOMI;

    //Make sure that pullup/pulldown is not enabled for the input pins
	//disable pullup/down on P1.0 since it is connect to MISO
	P1REN &= ~(BIT0 | SPI_USCIA0_SIMO | SPI_USCIA0_SOMI | SPI_USCIA0_UCLK);
    UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
}


void main(void) {
	/* Stop the Watchdog Timer (WDT) */
	WDTCTL = WDTPW + WDTHOLD;

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
	//IE1 &= 0xFD; /* Disable UCS interrupt */

	//Set up master SPI
	SPISetupMaster();
	//Set up slave SPI
	//SPISetupSlave();

	//Enable rx and tx SPI interrupts
	IE2 |= UCA0TXIE | UCA0RXIE;

	__enable_interrupt();

	P1DIR |= LED;
	while (1) {
		//Write data into UCA0TXBUF
		UCA0TXBUF = 0xBE;
		//Read data from UCA0RXBUF
		while (0 == in_ready) {
		}
		P1OUT ^= LED;
	}
}
