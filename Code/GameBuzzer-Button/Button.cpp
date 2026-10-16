#include "Button.h"

bool Button::pressed = false;
bool Button::ledOn = false;
uint32_t Button::lastChange = 0;
const uint16_t Button::lockoutTime = 500;
uint32_t Button::lastPress = 0;

void Button::setup() {
	
	pinMode( Configuration::PIN_BTN, INPUT );
	pinMode( Configuration::PIN_BTN_LED, OUTPUT ); // Active HIGH
	toggleLed( false );

};

void Button::toggleLed( const bool on ){
	ledOn = on; 
	digitalWrite( Configuration::PIN_BTN_LED, on ); 
};

bool Button::loop(){

	const uint32_t ms = millis();
	if( ms - lastChange < 50 )	// Debounce
		return false;

	const bool state = !digitalRead( Configuration::PIN_BTN );
	const bool changed = state != pressed;
	if( changed ){

		pressed = state;
		lastChange = ms;

	}

	return changed;

};
