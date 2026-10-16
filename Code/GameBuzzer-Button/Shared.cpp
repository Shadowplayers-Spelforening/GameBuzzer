#include "Shared.h"
#include <TFT_eSPI.h>

const char * COMMAND_NAMES[] = {
	"S_TOGGLE_BUTTON", 
	"S_SHOW_TEXT", 
	"S_SHOW_KEYBOARD", 
	"S_GET_TEXT", 
	"C_ANNOUNCE", 
	"C_TEXT", 
	"C_PRESS",
	"S_PING",
	"S_HOOKUP",
	"S_SHOW_NUMPAD"
};

const uint16_t COLOR_HEX[] = {
	TFT_RED, TFT_YELLOW, TFT_GREEN, TFT_BLUE, TFT_WHITE
};

const char * command2name( Command cmd ){
	if( cmd > sizeof(COMMAND_NAMES) - 1 ) 
		return "INVALID COMMAND";
	return COMMAND_NAMES[cmd];
};

const uint16_t colorToU16( ButtonColor color ){
	if( color > sizeof(COLOR_HEX) - 1 ) 
		return 0;
	return COLOR_HEX[color];
};
