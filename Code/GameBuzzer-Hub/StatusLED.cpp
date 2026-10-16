#include "StatusLED.h"

// Active LOW
const uint8_t StatusLED::PIN_ORDER[5] = {
	Configuration::PIN_LED_R,
	Configuration::PIN_LED_Y,
	Configuration::PIN_LED_G,
	Configuration::PIN_LED_B,
	Configuration::PIN_LED_W
};

uint32_t StatusLED::tick = 0;

void StatusLED::setup(){
	
	for( uint8_t i = 0; i < 5; i++ )
		pinMode( PIN_ORDER[i], OUTPUT );

	for( uint8_t i = 0; i < 5; i++ ){
		digitalWrite( PIN_ORDER[i], LOW );
		delay( 100 );
		digitalWrite( PIN_ORDER[i], HIGH );
	}
	
}

void StatusLED::loop(){
	
	const uint32_t ms = millis();
	const uint32_t delta = ms - tick;
	if( !tick || delta > 100 ){
		tick = ms;

		// Show connected buttons
		for( uint8_t i = 0; i < Netmanager::NUM_BUTTONS; ++i )
			digitalWrite(PIN_ORDER[i], !(
				Netmanager::buttons[i].isConnected() && 
				!Netmanager::buttons[i].isPressed()
			));

	}

}
