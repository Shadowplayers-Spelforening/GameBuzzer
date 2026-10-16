#include "_Configuration.h"
#include "Button.h"
#include "Display.h"
#include "Shared.h"
#include <FS.h>
#include <LittleFS.h>
#include "Netmanager.h"




void setup(){

	Serial.begin(115200);

	pinMode(Configuration::PIN_DBG_LED, OUTPUT);
	

	digitalWrite(Configuration::PIN_DBG_LED, HIGH);
	
	Button::setup();
	Display::setup();
	Serial.println("IT BEGINS!");


	Netmanager::setup();

	Serial.println("Everything initialized");

	Display::setText("", 1);
	Display::setStateText();


}

void loop(){
	
	if( Button::loop() ){

		if( Button::pressed ){
			
			Serial.printf("Button pressed, display %s, lockedout %s, ledOn %s\n", 
				Display::isButtonReturnState() ? "true" : "false",
				Button::isLockedOut() ? "true" : "false",
				Button::ledOn ? "true" : "false"
			);
			if( Display::isButtonReturnState() && !Button::isLockedOut() && Button::ledOn ){
				Netmanager::taskPressed();
				Button::lockOut();
			}
			else if( Display::isTextReturnState() )
				Netmanager::taskText(Display::getTypedBufferLength(), Display::getTypedBuffer());

		}

	}
	
	Display::loop();
	Netmanager::loop();

}
