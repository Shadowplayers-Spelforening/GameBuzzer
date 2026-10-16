#pragma once
#include <Arduino.h>

enum ButtonColor{
	RED = 0,
	YELLOW = 1,
	GREEN = 2,
	BLUE = 3,
	WHITE = 4,
	ALL = 128
};

enum DisplayState{
	STATE_TEXT = 0,
	STATE_KEYBOARD = 1,
	STATE_NUMPAD = 2
};

// ints are all in big endian
enum Command{
	S_TOGGLE_BUTTON = 0,		// bool on
	S_SHOW_TEXT = 1,			// uint16_t backgroundColor, uint16_t textColor, uint8_t textLength, const char *text
	S_SHOW_KEYBOARD = 2,		// uint16_t backgroundColor, uint16_t textColor, uint8_t maxLength, uint8_t textLength, char *text
	S_GET_TEXT = 3,				// Triggers C_TEXT
	C_ANNOUNCE = 4,				// bool hookup - Sent every 10 sec to keep alive.
	C_TEXT = 5,					// uint8_t textLength, char *text - Sends typed text to the server
	C_PRESS = 6,				// Sent when button is pressed
	S_PING = 7,					// void - Asks all devices to send C_ANNOUNCE
	S_HOOKUP = 8,				// (uint8_t[6])deviceMac - Tells the device that we have hooked it up. Not sent on broadcast address.
	S_SHOW_NUMPAD = 9,			// uint16_t textColor, uint32_t maxValue, uint8_t textLength, char *text
	S_ACK = 10,					// void - Reply from server to device when C_ANNOUNCE is received, if acknowledged
};

const char * command2name( Command cmd);
const uint16_t colorToU16( ButtonColor color);

