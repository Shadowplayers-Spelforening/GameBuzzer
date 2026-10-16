#include "_Configuration.h"
#include "Shared.h"
#include <esp_now.h>
#include <WiFi.h>
#include "Netmanager.h"
#include "StatusLED.h"
#include "SerialOut.h"

void setup(){

	Serial.begin(115200);

	StatusLED::setup();
	Netmanager::setup();
	SerialOut::setup();

	Serial.println("Server initialized!");

}

void loop(){

	SerialOut::loop();
	Netmanager::loop();
	StatusLED::loop();

}
