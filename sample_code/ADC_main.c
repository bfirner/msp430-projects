/*
 * main.c
 */

#include <msp430.h>

/* Example memory segment.
 * 1.Debug
 * 2.Suspend (pause button)
 * 3.Window->Show View->Memory Browser
 * 4.View 0x10DA
 * Label          Word Address (hex)   0x10DA Offset (hex) Value Device 1  Value Device 2

               10DA                 0                   0x1010          0x1010

               10DC                 2                   0x7FE7          0x800D

               10DE                 4                   0x0000          0x0000

               10E0                 6                   0x80DF          0x8149

CAL_ADC_15T30  10E2                 8                   0x02F8          0x02EC

CAL_ADC_15T85  10E4                 A                   0x037B          0x0000

               10E6                 C                   0x8147          0x81D2

CAL_ADC_25T30  10E8                 E                   0x01C7          0x01BF

CAL_ADC_25T85  10EA                 10                  0x0213          0x0000
 */
/*
#define CAL_ADC_15T30 0x02F2
#define CAL_ADC_15T85 0x037B
#define CAL_ADC_25T30 0x01C3
#define CAL_ADC_25T85 0x0215
*/

#define CAL_ADC_25T30 ((unsigned short*)(0x10E8))[0]
#define CAL_ADC_25T85 ((unsigned short*)(0x10EA))[0]
#define CAL_ADC_15T30 ((unsigned short*)(0x10E2))[0]
#define CAL_ADC_15T85 ((unsigned short*)(0x10E4))[0]

/*
 * Raw ADC value for temperature.
 */
int adc10_value;

// ADC10 interrupt service routine so we sleep while waiting for the conversion to finish
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
	adc10_value = ADC10MEM; //temporarily hold output value while shutting down adc.
	_BIC_SR_IRQ(CPUOFF);
}

//Convert the raw adc temperature value into using the
//calibrated values as described in section 24.2.2.1
float convertTemp1_5Ref(int rawADC) {
	return (rawADC - CAL_ADC_15T30)*(85-30)/(CAL_ADC_15T85-CAL_ADC_15T30) + 30;
}
float convertTemp2_5Ref(int rawADC) {
	return (rawADC - CAL_ADC_25T30)*(85-30)/(CAL_ADC_25T85-CAL_ADC_25T30) + 30;
}

/*
 * Configure the ADC for temperature sensing and sense
 */
float getTemperature() {
	/*
	 * Comment Format:		//Bits modified	//Bits' function				//Bits modified to...
	 */

	ADC10CTL0 = SREF_1		//Bits 15-13	//Select Reference				//R+ = VREF+ and VR- = VSS
	+ REFON 				//Bit 5			//Reference generator on		//Reference on
			+ ADC10ON 		//Bit 4			//ADC10 on						//ADC10 on
			+ ADC10SHT_3 	//Bits 12-11	//ADC10 sample-and-hold time	//64 x ADC10CLKs
			+ ADC10IE; 		//Enable interrupts when the ADC10MEM register gets a conversion result

	ADC10CTL1 = INCH_10 	//Bits 15-12	//Input channel select			//Temperature sensor
	+ ADC10DIV_3; 			//Bits 7-5		//ADC10 clock divider			///4

	__delay_cycles(100); //wait for Vref to settle.  30 microseconds needed.

	ADC10CTL0 |= ENC	//Bit 1		//Enable conversion				//ADC10 enabled
			+ ADC10SC;	//Bit 0		//Start conversion				//Start sample-and-conversion

	_BIS_SR(CPUOFF + GIE);
	// LPM0, ADC12_ISR will force exit

	ADC10CTL0 &= ~ENC; //Bit 1			//Enable conversion bit--ADC10 disabled

	ADC10CTL0 &= ~(REFON | ADC10ON); //turn off Reference and ADC.  Needed for low sleep power

	//Shift up by two since these calibrated values are for 12 bit results
	return convertTemp1_5Ref(adc10_value);
	//return temperatureSlope * adc10_value + temperatureOffset;
	//return(int) ((adc10_value * 27069L - 18169625L) >> 16);
}

int counter = 0;
char sample_temp = 0;

//Set up a watchdog timer interrupt that toggles the LED
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void) {
    WDTCTL = WDT_MDLY_32;
    if (30 == ++counter) {
    	P1OUT ^= 0x01;
    	counter = 0;
    	sample_temp = 1;
    }
}

void main(void) {
	//Stop the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

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

	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;

	//Enable the watchdog timer interrupt and global interrupts
    IE1 |= WDTIE;
    __enable_interrupt();
    //32ms timer from SMCLK
    WDTCTL = WDT_MDLY_32;
    while (1) {
    	//Sample the temperature once a second
    	if (sample_temp) {
    		volatile float temp = getTemperature();
    		sample_temp = 0;
    	}
    };

}
