
#ifndef ACDimmer_h
#define ACDimmer_h 

#include "Arduino.h"

struct Dimmer
{
	Dimmer(const int pin);

	void Init();

	void Power(byte power);
	byte Power();

	void State(bool state);
	bool State();
};

#endif