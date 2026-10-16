#pragma once

#include "Free_Fonts.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <XPT2046_Touchscreen.h>
#include <Arduino.h>
#include "_Configuration.h"
#include <U8g2_for_TFT_eSPI.h>

const uint16_t BOTTOM = 400;
const uint16_t LEFT = 410;
const uint16_t TOP = 3875;
const uint16_t RIGHT = 3739;

const uint16_t SCREEN_WIDTH = 320;
const uint16_t SCREEN_HEIGHT = 240;


/*
	const char text[20] = "För helvete Vilmer";
	const uint16_t width = u8g2.getUTF8Width(text);
	u8g2.setForegroundColor(TFT_WHITE);
	u8g2.drawUTF8(SCREEN_WIDTH/2-width/2, SCREEN_HEIGHT/2+9, text); 
*/

class Display{

	
	static TFT_eSPI tft; // Invoke custom library
	static XPT2046_Touchscreen ts;
	static U8g2_for_TFT_eSPI u8g2;

	static bool keyboardKeyDown;
	static const char * keyboardDownKey;
	static const char * keyboardHighlightKey;
	static uint32_t keyboardHighlightUntil;
	static bool textDirty;

	static DisplayState state;		// Show text
	static uint16_t backgroundColor;
	static uint16_t textColor;
	static char text[256];
	static uint32_t numpadMaxVal;
	

	static const uint16_t TOUCH_TOP;
	static const uint16_t TOUCH_LEFT;
	static const uint16_t TOUCH_BOTTOM;
	static const uint16_t TOUCH_RIGHT;


	static const size_t keyboardRow1len = 11;
	static const size_t keyboardRow2len = 11;
	static const size_t keyboardRow3len = 11;
	static const size_t keyboardRow4len = 10;
	static const char * keyboardRow1[keyboardRow1len];
	static const char * keyboardRow2[keyboardRow2len];
	static const char * keyboardRow3[keyboardRow3len];
	static const char * keyboardRow4[keyboardRow4len];
	// Width of each character
	static const uint8_t keyboardRow1Units[keyboardRow1len];
	static const uint8_t keyboardRow2Units[keyboardRow2len];
	static const uint8_t keyboardRow3Units[keyboardRow3len];
	static const uint8_t keyboardRow4Units[keyboardRow4len];

	static const size_t numpadRow1len = 5;
	static const size_t numpadRow2len = 6;
	static const char * numpadRow1[numpadRow1len];
	static const char * numpadRow2[numpadRow2len];
	static const uint8_t numpadRow1Units[numpadRow1len];
	static const uint8_t numpadRow2Units[numpadRow2len];

	static void drawKeyboardRow(uint16_t y, const char *keys[], const uint8_t units[], uint8_t count, uint16_t rowHeight);
	static void drawKeyboard();
	static void drawKeyboardKey(const char *key, bool highlighted);
	static const char * getKeyboardKeyAt(int16_t x, int16_t y);
	static bool getKeyboardKeyRect(const char *key, uint16_t &x, uint16_t &y, uint16_t &w, uint16_t &h);
	static void drawNumpadRow(uint16_t y, const char *keys[], const uint8_t units[], uint8_t count, uint16_t rowHeight);
	static void drawNumpad();
	static void drawNumpadEntry();
	static void drawNumpadKey(const char *key, bool highlighted);
	static const char * getNumpadKeyAt(int16_t x, int16_t y);
	static bool getNumpadKeyRect(const char *key, uint16_t &x, uint16_t &y, uint16_t &w, uint16_t &h);
	static void drawTypedBuffer();
	static bool appendTypedKey(const char *key);
	static bool appendNumpadKey(const char *key);
	static void clearTypedBuffer();

	public:
		static constexpr size_t typedBufferLength = 101;
		static char typedBuffer[typedBufferLength];				// Typed on keyboard
		static uint8_t maxLength;				// typedBuffer max length

		static void setup();
		static void draw();

		static void setBackgroundColor( const uint16_t color );
		static void setTextColor( const uint16_t color );


		static void setNumpadMaxVal( const uint32_t val );
		static void setText( const char * inputText, const uint8_t length );
		static void setMaxLength( const uint8_t length );
		static void setStateText();
		static void setStateKeyboard();
		static void setStateNumpad();
		static void loop();
		static void setLedBrightness( const uint8_t brightness );

		static uint8_t getTypedBufferLength(){ return strnlen(typedBuffer, typedBufferLength); };
		static const char * getTypedBuffer(){ return typedBuffer; };

		static bool isButtonReturnState(){ return state == STATE_TEXT; };
		static bool isTextReturnState(){ return state == STATE_KEYBOARD || state == STATE_NUMPAD; };


};


