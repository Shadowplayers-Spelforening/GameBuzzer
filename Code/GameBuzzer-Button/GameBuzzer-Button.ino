#include "_Configuration.h"
#include "Button.h"
#include "Display.h"
#include "Shared.h"
#include <FS.h>
#include <LittleFS.h>
#include "Netmanager.h"

static void handleSerialDisplayCommands(){
	static char serialBuffer[32];
	static size_t serialBufferLen = 0;

	while( Serial.available() > 0 ){

		char c = Serial.read();
		if( c == '\r' )
			continue;
		if( c == '\n' ){

			serialBuffer[serialBufferLen] = '\0';
			if( serialBufferLen > 0 ){

				String cmd = String(serialBuffer);
				cmd.trim();
				if( cmd.equalsIgnoreCase("KEYBOARD") ){
					Display::setStateKeyboard();
					Serial.println("Display state -> KEYBOARD");
				}
				else if( cmd.equalsIgnoreCase("NUMPAD") ){
					Display::setStateNumpad();
					Serial.println("Display state -> NUMPAD");
				}
				else if( cmd.equalsIgnoreCase("TEXT") ){
					Display::setStateText();
					Serial.println("Display state -> TEXT");
				}
				else if( cmd.equalsIgnoreCase("RESET") ){
					ESP.restart();
				}

			}
			serialBufferLen = 0;
			continue;

		}

		if( serialBufferLen < sizeof(serialBuffer) - 1 )
			serialBuffer[serialBufferLen++] = c;

	}
}

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
	
	handleSerialDisplayCommands();
	
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
