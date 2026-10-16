#pragma once
#include <arduino.h>
#include "_Configuration.h"

class Button{

	public:
		static bool pressed;
		static bool ledOn;
		static const uint16_t lockoutTime;
		static uint32_t lastPress;

		static void setup();
		static bool loop();
		static void toggleLed( const bool on );
		static bool isLockedOut(){
			return millis() - lastPress < lockoutTime;
		};
		static void lockOut(){ lastPress = millis(); };

		Button(){};


	private:
		static uint32_t lastChange;
		

};
