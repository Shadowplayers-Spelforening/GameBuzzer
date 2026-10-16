#pragma once
#include <arduino.h>
#include "Shared.h"

namespace Configuration{

	//const ButtonColor myColor = RED;
	const ButtonColor myColor = YELLOW;
	//const ButtonColor myColor = GREEN;
	//const ButtonColor myColor = BLUE;
	//const ButtonColor myColor = WHITE;

	const bool INVERT_SCREEN = true;

	const uint8_t PIN_DC = 16;
	const uint8_t PIN_DBG_LED = 17;
	const uint8_t PIN_BTN_LED = 26;
	const uint8_t PIN_DISP_LED = 27;
	const uint8_t PIN_T_CS = 32;
	const uint8_t PIN_CS = 33;
	const uint8_t PIN_BTN = 35;


};
