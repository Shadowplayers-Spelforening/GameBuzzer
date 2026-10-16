#pragma once
#include <Arduino.h>
#include "Netmanager.h"
#include <ArduinoJson.h>

enum SerialInCommand{ // JSON array
	SIN_PING = 0,			// void - Sends SOUT_INIT
	SIN_TOGGLE_BUTTON = 1,	// uint8_t color, "1" or "0"
	SIN_SHOW_TEXT = 2,		// uint8_t color, uint16_t backgroundColor, uint16_t textColor, const char *text
	SIN_SHOW_KEYBOARD = 3,  // uint8_t color, uint16_t backgroundColor, uint16_t textColor, uint8_t maxLength, char *text
	SIN_GET_TEXT = 4,		// uint8_t color
	SIN_SHOW_NUMPAD = 5,	// uint8_t color, uint16_t backgroundColor, uint16_t textColor, uint32_t maxValue, char *text
	SIN_GET_CONNECTED = 6,	// void - Sends SOUT_CONN_BTN for each connected button
};

enum SerialOutCommand{ // JSON array
	SOUT_INIT = 0,				// (str)version
	SOUT_BTN_PRESS = 1,			// (int)color
	SOUT_TEXT = 2,				// (int)color, (str)text
	SOUT_CONN_BTN = 3,			// (int)color
	SOUT_DISC_BTN = 4,			// (int)color
};

// tasks are sent as <SPBUZ>[int task, arg0 (usually color), arg1, arg2...]</SPBUZ>
namespace SerialOut{


	void handleRaw();

	

	void send( const char * jsonString );
	void sendInit();
	void sendButtonPress( const ButtonColor color );
	void sendText( const ButtonColor color, const char * text );
	void sendButtonConnectionStatus( const ButtonColor color, const bool connected );


	void handleCommand( const char * jsonArray, const size_t length );

	void setup();
	void loop();
	
	// Text is received with PREFIX + JSON array + SUFFIX
	// The first element of the array is the task/command.
	void handleRaw();

	
	

};


