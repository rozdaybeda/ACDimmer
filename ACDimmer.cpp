//15.12.2018
//by Reptiloid software


#include "Arduino.h"
#include "ACDimmer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#define HALF_CYCLE 625   //625 * 16 microseconds = 10 miliseconds

const int default_dimmerM2Pin = 2;
const int default_dimmerM1Pin = 3;

struct DimmerData
{
	bool state = true;

	int pulse = HALF_CYCLE;
	byte power = 100;

	int gatePin = default_dimmerM1Pin; //TRIAC gate
	const int zeroCrossPin = default_dimmerM2Pin; //zero cross detect
};

DimmerData dimmer_data;

//https://playground.arduino.cc/Main/ACPhaseControl
//http://dfe.petrsu.ru/koi/posob/avrlab/mega16tcnt1.html

void zeroCrossingInterrupt() { //zero cross detect   
	TCNT1 = 0;   //reset timer count
	TCCR1B = (1 << CS12) | (0 << CS11) | (0 << CS10); //start timer with divide by 256 input
}

ISR(TIMER1_COMPA_vect) { //comparator match
	if (dimmer_data.state)
		digitalWrite(dimmer_data.gatePin, HIGH);  //set TRIAC gate to high

	TCNT1 = 65535 - dimmer_data.pulse / 2;      //trigger pulse width
}

ISR(TIMER1_OVF_vect) { //timer1 overflow
	digitalWrite(dimmer_data.gatePin, LOW); //turn off TRIAC gate
	TCCR1B = 0x00;          //disable timer stopd unintended triggers
}


Dimmer::Dimmer(const int pin) {
	dimmer_data.power = 100;
	dimmer_data.pulse = HALF_CYCLE;

	dimmer_data.gatePin = pin;

	pinMode(dimmer_data.zeroCrossPin, INPUT);     //zero cross detect
	digitalWrite(dimmer_data.zeroCrossPin, HIGH); //enable pull-up resistor

	pinMode(dimmer_data.gatePin, OUTPUT); //TRIAC gate control
	digitalWrite(dimmer_data.gatePin, LOW); //set TRIAC gate to low
}

void Dimmer::Init()
{
	cli();//stop interrupts

	// set up Timer1 
	TCCR1A = 0; //timer control registers set for
	TCCR1B = 0; //normal operation, timer disabled
	TCNT1 = 0;  //reset timer count

	OCR1A = 0; //initialize the comparator
	TIMSK1 |= (1 << OCIE1A) | (1 << TOIE1); //enable comparator A and overflow interrupts
	TCCR1B = (1 << CS12) | (0 << CS11) | (0 << CS10); //start timer with divide by 256 input

	// set up zero crossing interrupt
	attachInterrupt(0, zeroCrossingInterrupt, CHANGE); // every 10 milli seconds
	//IRQ0 is pin 2. Call zeroCrossingInterrupt on rising signal

	sei();
}

void Dimmer::Power(byte power) {
	dimmer_data.power = power;
	if (power != 0)
		dimmer_data.pulse = (HALF_CYCLE * power) / 100;
	else
		dimmer_data.pulse = HALF_CYCLE;

	OCR1A = (HALF_CYCLE - dimmer_data.pulse) / 2;
}

byte Dimmer::Power() {
	return dimmer_data.power;
}

void Dimmer::State(bool state) {
	dimmer_data.state = state;
}

bool Dimmer::State() {
	return dimmer_data.state;
}
