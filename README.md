Code from a lab course on embedded programming using the MSP430. The topic and original lectures are below. I am currently cleaning the lectures to put them into a single lab manual, so the only thing uploaded now is the example code. The course topics each have a number of associated lectures with example code, short "lab" projects, and some topics have larger projects that integrate the topics so far.

__Topic: Reintroduction to C__
#    01 Lecture: binary operators, volatile, pragmas, interrupts, registers

__Topic: Introduction to the MSP430__
#    01 Lecture: Memory spaces, modes, capabilities

%Note combined first two topics, but skipped detailed coverage of memory space

__Topic: Output and basic timing__
#    02 Lecture: Output, power consumption (pull up/down), watchdog timer
%Note: didn't cover power consumption, will cover with output or LPM
#    03 Lab: blink an LED at a desired rate
#    04 Project: Smiley face drawing, spell a word
##                In class we do a lab to display the smiley face
##                As a take home project groups spell out words
##                Class 05 starts with a small quiz on interrupts, the WDT, and digital output

__Topic: Advanced timer controls__
#    05 Lecture: Automatically controlled output, other output pin modes
#    06 Lab: Two LEDs with one clock!

__Topic: Digital to analog conversion__
#    07 Lecture: What is digital to analog conversion? How is it used?
#    08 Lab: PWM output square, sine, and sawtooth signals, build a rectifier circuit
#    09 Lecture: Applications of PWM
#    10 Project: Tone and scale generation

__Topic: Input (Digital and Analog)__
#    11 Lecture: Input, interrupts, pull up and pull down resistors, LPM and wakeup
#    12 Lab: Measuring Current Consumption with VLO or a Crystal Oscillator in LPM3
#    13 Analog to digital conversion
#    14 Lab: Check input power level when using a solar cell

__Topic: Low Power Mode (already introduced this because it is needed for solar cell!)__
#    15 Lab: Wake from low power sleep with button clicks (Currently LPM is in lecture 7)
#    16 Project: morse code clicker (with low power)

__Topic: Wire communiction__
#    17 Lecture: Wire communication, I2C and SPI
#    18 Lab: Simple interrupt and polling communication, turn on LED from different launchpad
#    19 Lab: SPI between two launchpads

__Topic: Building a media player__
#    20 Lecture: Talking to a file system
#    21 Lab: Read data from a filesystem
#    22 Project: read bits from a filesystem, blink the LED
#    23-24 Final Project: read music from a filesystem and play it
