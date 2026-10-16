#pragma once
#include "_Configuration.h"
#include "Netmanager.h"


class StatusLED{
	static const uint8_t PIN_ORDER[5];

	public:
		static void setup();
		static void loop();
		// Todo:
	
	private:
		static uint32_t tick;


};
