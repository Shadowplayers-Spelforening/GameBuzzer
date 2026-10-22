#pragma once
#include <arduino.h>
#include "Shared.h"

// note: because the screen is mounted upside down, these get flipped 
struct DisplayCalibration{
	uint16_t top;			// high X (bottom in the real world)
	uint16_t bottom;		// low X (top in the real world
	uint16_t left;			// low Y (right in the real world)
	uint16_t right;			// high Y (left in the real world)
};

namespace Configuration{

	const size_t NUM_BUTTONS = 5;

	//const ButtonColor myColor = RED;
	const ButtonColor myColor = YELLOW;
	//const ButtonColor myColor = GREEN;
	//const ButtonColor myColor = BLUE;
	//const ButtonColor myColor = WHITE;

	const DisplayCalibration displayCalibration[NUM_BUTTONS] = {
		{3775,414,691,3604}, // Red
		{3799,291,752,3657}, // yellow
		{3826,433,577,3448}, // green
		{3627,587,343,3770}, // blue
		{3754,532,377,3738}, // white
	};




	const uint16_t TOUCH_TOP = displayCalibration[myColor].top;    
	const uint16_t TOUCH_BOTTOM = displayCalibration[myColor].bottom;
	const uint16_t TOUCH_LEFT = displayCalibration[myColor].left;
	const uint16_t TOUCH_RIGHT = displayCalibration[myColor].right;

	const bool INVERT_SCREEN = true;

	const uint8_t PIN_DC = 16;
	const uint8_t PIN_DBG_LED = 17;
	const uint8_t PIN_BTN_LED = 26;
	const uint8_t PIN_DISP_LED = 27;
	const uint8_t PIN_T_CS = 32;
	const uint8_t PIN_CS = 33;
	const uint8_t PIN_BTN = 35;

	

};
