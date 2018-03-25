## Getting Started with Embedded Programming

This repository has code from a lab course on embedded programming using the MSP430 that I made for Rutgers. The topic list and original lectures are below. The original course has a number of associated lectures with example code, short "lab" projects, and some topics have larger projects that integrate the topics so far. I will also be creating larger, almost useful projects using the topics in the this outline.

Below is a list of topics and the associated lectures, lab projects, or take-home projects. The numbers denote in which class the lecture, lab, or take-home project appears.

## Topic: Reintroduction to C
### 01 Lecture: binary operators, volatile, pragmas, interrupts, registers

## Topic: Introduction to the MSP430
### 01 Lecture: Memory spaces, modes, capabilities

## Topic: Output and basic timing
###    02 Lecture: Output, power consumption (pull up/down), watchdog timer
###    03 Lab: blink an LED at a desired rate
###    04 Project: Smiley face drawing (or write a word) with persistence of vision
1. In class we do a lab to display the smiley face
2. As a take home project groups spell out words
3. Class 05 starts with a small quiz on interrupts, the WDT, and digital output

## Topic: Advanced timer controls
###    05 Lecture: Hardware controlled output, other output pin modes
###    06 Lab: Two LEDs with one clock!

## Topic: Digital to analog conversion (DAC)
###    07 Lecture: What is digital to analog conversion? How is it used?
###    08 Lab: PWM output square, sine, and sawtooth signals, build a rectifier circuit
###    09 Lecture: Applications of PWM
###    10 Project: Tone and scale generation

## Topic: Input (Digital and Analog)
###    11 Lecture: Input, interrupts, pull up and pull down resistors, LPM and wakeup
###    12 Lab: Measuring Current Consumption with VLO or a Crystal Oscillator in LPM3
###    13 Analog to digital conversion
###    14 Lab: Check input power level when using a solar cell

## Topic: Low Power Mode
   This was already introduced since it is needed when powering the board with a solar cell earlier and when measuring low power consumption.
###    15 Lab: Wake from low power sleep with button clicks (Currently LPM is in lecture 7)
###    16 Project: morse code clicker (with low power)

## Topic: Wire communiction
###    17 Lecture: Wire communication, I2C and SPI
###    18 Lab: Simple interrupt and polling communication, turn on LED from different launchpad
###    19 Lab: SPI between two launchpads

## Topic: Building a (really basic) media player
###    20 Lecture: Talking to a file system
###    21 Lab: Read data from a filesystem
###    22 Project: read bits from a filesystem, blink the LED
###    23-24 Final Project: read music from a filesystem and play it
