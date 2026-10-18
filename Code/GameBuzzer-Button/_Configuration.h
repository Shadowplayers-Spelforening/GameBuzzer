#pragma once
#include <arduino.h>
#include "Shared.h"

namespace Configuration{

	//const ButtonColor myColor = RED;
	//const ButtonColor myColor = YELLOW;
	const ButtonColor myColor = GREEN;
	//const ButtonColor myColor = BLUE;
	//const ButtonColor myColor = WHITE;

	//#define SCREEN_DEFAULT

	#ifdef SCREEN_DEFAULT
	const uint16_t TOUCH_TOP = 3746;        // x = bottom to top
	const uint16_t TOUCH_BOTTOM = 400;
	const uint16_t TOUCH_LEFT = 780;
	const uint16_t TOUCH_RIGHT = 3650;      // y = left to right
	#else
	const uint16_t TOUCH_TOP = 3839;        // x = bottom to top
	const uint16_t TOUCH_BOTTOM = 532;
	const uint16_t TOUCH_LEFT = 588;
	const uint16_t TOUCH_RIGHT = 3453;      // y = left to right
	#endif

	const bool INVERT_SCREEN = true;

	const uint8_t PIN_DC = 16;
	const uint8_t PIN_DBG_LED = 17;
	const uint8_t PIN_BTN_LED = 26;
	const uint8_t PIN_DISP_LED = 27;
	const uint8_t PIN_T_CS = 32;
	const uint8_t PIN_CS = 33;
	const uint8_t PIN_BTN = 35;

	

};
