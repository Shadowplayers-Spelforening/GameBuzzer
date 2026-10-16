#include "Display.h"
#include "Shared.h"

TFT_eSPI Display::tft = TFT_eSPI(); // Invoke custom library
XPT2046_Touchscreen Display::ts(Configuration::PIN_T_CS);
U8g2_for_TFT_eSPI Display::u8g2;
char Display::typedBuffer[typedBufferLength] = "";				// Typed on keyboard
DisplayState Display::state = STATE_TEXT;		// Show text
uint16_t Display::backgroundColor = TFT_BLACK;
uint16_t Display::textColor = TFT_WHITE;
uint8_t Display::maxLength = 60;				// typedBuffer max length
char Display::text[256] = "";
uint32_t Display::numpadMaxVal = 0;
bool Display::keyboardKeyDown = false;
const char * Display::keyboardDownKey = nullptr;
const char * Display::keyboardHighlightKey = nullptr;
uint32_t Display::keyboardHighlightUntil = 0;
bool Display::textDirty = true;

const uint16_t Display::TOUCH_TOP = 3746;        // x = bottom to top
const uint16_t Display::TOUCH_BOTTOM = 400;
const uint16_t Display::TOUCH_LEFT = 780;
const uint16_t Display::TOUCH_RIGHT = 3650;      // y = left to right

int16_t preX = 0;
int16_t preY = 0; 

const char * Display::keyboardRow1[keyboardRow1len] = {"1","2","3","4","5","6","7","8","9","0","<<<"};
const char * Display::keyboardRow2[keyboardRow2len] = {"Q","W","E","R","T","Y","U","I","O","P","Å"};
const char * Display::keyboardRow3[keyboardRow3len] = {"A","S","D","F","G","H","J","K","L","Ö","Ä"};
const char * Display::keyboardRow4[keyboardRow4len] = {"Z","X","C","V","B","N","M",".","-"," "};
// Width of each character
const uint8_t Display::keyboardRow1Units[keyboardRow1len] = {1,1,1,1,1,1,1,1,1,1,2};
const uint8_t Display::keyboardRow2Units[keyboardRow2len] = {1,1,1,1,1,1,1,1,1,1,1};
const uint8_t Display::keyboardRow3Units[keyboardRow3len] = {1,1,1,1,1,1,1,1,1,1,1};
const uint8_t Display::keyboardRow4Units[keyboardRow4len] = {1,1,1,1,1,1,1,1,1,3};

const char * Display::numpadRow1[numpadRow1len] = {"1","2","3","4","<<<"};
const char * Display::numpadRow2[numpadRow2len] = {"5","6","7","8","9","0"};
const uint8_t Display::numpadRow1Units[numpadRow1len] = {1,1,1,1,2};
const uint8_t Display::numpadRow2Units[numpadRow2len] = {1,1,1,1,1,1};

const char * Display::getKeyboardKeyAt(int16_t x, int16_t y){
	const uint16_t keyboardTop = SCREEN_HEIGHT / 2;
	const uint16_t keyboardHeight = SCREEN_HEIGHT - keyboardTop;
	const uint8_t rowCount = 4;
	const uint16_t rowSpacing = 4;
	const uint16_t rowHeight = (keyboardHeight - rowSpacing * (rowCount + 1)) / rowCount;

	if( y < keyboardTop + rowSpacing || y >= SCREEN_HEIGHT - rowSpacing )
		return nullptr;

	for( uint8_t row = 0; row < rowCount; ++row ){
		uint16_t rowY = keyboardTop + rowSpacing * (row + 1) + rowHeight * row;
		if( y < rowY || y >= rowY + rowHeight )
			continue;

		const char **keys = nullptr;
		const uint8_t *units = nullptr;
		size_t count = 0;
		switch( row ){
			case 0: keys = keyboardRow1; units = keyboardRow1Units; count = keyboardRow1len; break;
			case 1: keys = keyboardRow2; units = keyboardRow2Units; count = keyboardRow2len; break;
			case 2: keys = keyboardRow3; units = keyboardRow3Units; count = keyboardRow3len; break;
			case 3: keys = keyboardRow4; units = keyboardRow4Units; count = keyboardRow4len; break;
		}

		const uint16_t keySpacingLocal = 4;
		uint16_t totalUnits = 0;
		for( size_t i = 0; i < count; ++i )
			totalUnits += units[i];

		const uint16_t availableWidth = SCREEN_WIDTH - keySpacingLocal * (count + 1);
		const uint16_t unitWidth = availableWidth / totalUnits;
		uint16_t keyX = keySpacingLocal;

		for( size_t i = 0; i < count; ++i ){
			uint16_t w = unitWidth * units[i];
			if( i == count - 1 )
				w = SCREEN_WIDTH - keyX - keySpacingLocal;

			if( x >= keyX && x < keyX + w )
				return keys[i];

			keyX += w + keySpacingLocal;
		}
	}

	return nullptr;
}



void Display::drawKeyboardRow(uint16_t y, const char *keys[], const uint8_t units[], uint8_t count, uint16_t rowHeight){
	const uint16_t keySpacing = 4;
	uint16_t totalUnits = 0;
	for( uint8_t i = 0; i < count; ++i )
		totalUnits += units[i];

	const uint16_t availableWidth = SCREEN_WIDTH - keySpacing * (count + 1);
	const uint16_t unitWidth = availableWidth / totalUnits;
	uint16_t x = keySpacing;

	for( uint8_t i = 0; i < count; ++i ){
		uint16_t w = unitWidth * units[i];
		if( i == count - 1 )
			w = SCREEN_WIDTH - x - keySpacing;

		tft.fillRoundRect(x, y, w, rowHeight, 4, TFT_DARKGREY);
		tft.drawRoundRect(x, y, w, rowHeight, 4, textColor);

		const bool isHighlighted = (keys[i] == keyboardHighlightKey && millis() < keyboardHighlightUntil);
		u8g2.setFontMode(1);
		u8g2.setForegroundColor(isHighlighted ? TFT_GREEN : textColor);

		const int16_t textWidth = u8g2.getUTF8Width(keys[i]);
		const int16_t textX = x + (w - textWidth) / 2;
		const int16_t textY = y + rowHeight - 6;
		u8g2.drawUTF8(textX, textY, keys[i]);

		x += w + keySpacing;
	}
}

void Display::drawKeyboard(){
	const uint16_t keyboardTop = SCREEN_HEIGHT / 2;
	const uint16_t keyboardHeight = SCREEN_HEIGHT - keyboardTop;
	const uint8_t rowCount = 4;
	const uint16_t rowSpacing = 4;
	const uint16_t rowHeight = (keyboardHeight - rowSpacing * (rowCount + 1)) / rowCount;

	tft.fillRect(0, keyboardTop, SCREEN_WIDTH, keyboardHeight, TFT_BLACK);
	u8g2.setFont(u8g2_font_helvB14_tf);

	drawKeyboardRow(keyboardTop + rowSpacing, keyboardRow1, keyboardRow1Units, keyboardRow1len, rowHeight);
	drawKeyboardRow(keyboardTop + rowSpacing * 2 + rowHeight, keyboardRow2, keyboardRow2Units, keyboardRow2len, rowHeight);
	drawKeyboardRow(keyboardTop + rowSpacing * 3 + rowHeight * 2, keyboardRow3, keyboardRow3Units, keyboardRow3len, rowHeight);
	drawKeyboardRow(keyboardTop + rowSpacing * 4 + rowHeight * 3, keyboardRow4, keyboardRow4Units, keyboardRow4len, rowHeight);
}

void Display::drawNumpadRow(uint16_t y, const char *keys[], const uint8_t units[], uint8_t count, uint16_t rowHeight){
	const uint16_t keySpacing = 4;
	uint16_t totalUnits = 0;
	for( uint8_t i = 0; i < count; ++i )
		totalUnits += units[i];

	const uint16_t availableWidth = SCREEN_WIDTH - keySpacing * (count + 1);
	const uint16_t unitWidth = availableWidth / totalUnits;
	uint16_t x = keySpacing;

	u8g2.setFont(u8g2_font_helvB18_tf);
	for( uint8_t i = 0; i < count; ++i ){
		uint16_t w = unitWidth * units[i];
		if( i == count - 1 )
			w = SCREEN_WIDTH - x - keySpacing;

		tft.fillRoundRect(x, y, w, rowHeight, 4, TFT_DARKGREY);
		tft.drawRoundRect(x, y, w, rowHeight, 4, textColor);

		const bool isHighlighted = (keys[i] == keyboardHighlightKey && millis() < keyboardHighlightUntil);
		u8g2.setFontMode(1);
		u8g2.setForegroundColor(isHighlighted ? TFT_GREEN : textColor);

		const int16_t textWidth = u8g2.getUTF8Width(keys[i]);
		const int16_t textX = x + (w - textWidth) / 2;
		const int16_t textY = y + rowHeight / 2 + 6;
		u8g2.drawUTF8(textX, textY, keys[i]);

		x += w + keySpacing;
	}
}

void Display::drawNumpad(){
	const uint16_t keyboardTop = SCREEN_HEIGHT / 2;
	const uint16_t keyboardHeight = SCREEN_HEIGHT - keyboardTop;
	const uint8_t rowCount = 2;
	const uint16_t rowSpacing = 4;
	const uint16_t rowHeight = (keyboardHeight - rowSpacing * (rowCount + 1)) / rowCount;

	tft.fillRect(0, keyboardTop, SCREEN_WIDTH, keyboardHeight, TFT_BLACK);
	u8g2.setFont(u8g2_font_helvB14_tf);

	drawNumpadRow(keyboardTop + rowSpacing, numpadRow1, numpadRow1Units, numpadRow1len, rowHeight);
	drawNumpadRow(keyboardTop + rowSpacing * 2 + rowHeight, numpadRow2, numpadRow2Units, numpadRow2len, rowHeight);
	drawNumpadEntry();

}

void Display::drawNumpadEntry(){
	const uint16_t entryX = 12;
	const uint16_t entryY = 58;
	const uint16_t entryWidth = SCREEN_WIDTH - entryX * 2;
	const uint16_t entryHeight = 44;

	tft.fillRect(entryX, entryY, entryWidth, entryHeight, backgroundColor);
	tft.drawRoundRect(entryX, entryY, entryWidth, entryHeight, 4, textColor);

	u8g2.setFont(u8g2_font_helvB24_tf);
	u8g2.setFontMode(1);
	u8g2.setForegroundColor(textColor);

	const int16_t textWidth = u8g2.getUTF8Width(typedBuffer);
	const int16_t textX = entryX + (entryWidth - textWidth) / 2;
	const int16_t textY = entryY + entryHeight / 2 + 10;
	u8g2.drawUTF8(textX, textY, typedBuffer);
}

void Display::drawNumpadKey(const char *key, bool highlighted){
	uint16_t x, y, w, h;
	if( !getNumpadKeyRect(key, x, y, w, h) )
		return;

	tft.fillRoundRect(x, y, w, h, 4, TFT_DARKGREY);
	tft.drawRoundRect(x, y, w, h, 4, highlighted ? TFT_GREEN : textColor);

	u8g2.setFont(u8g2_font_helvB18_tf);
	u8g2.setFontMode(1);
	u8g2.setForegroundColor(highlighted ? TFT_GREEN : textColor);

	const int16_t textWidth = u8g2.getUTF8Width(key);
	const int16_t textX = x + (w - textWidth) / 2;
	const int16_t textY = y + h / 2 + 6;
	u8g2.drawUTF8(textX, textY, key);
}

const char * Display::getNumpadKeyAt(int16_t x, int16_t y){
	const uint16_t keyboardTop = SCREEN_HEIGHT / 2;
	const uint16_t keyboardHeight = SCREEN_HEIGHT - keyboardTop;
	const uint8_t rowCount = 2;
	const uint16_t rowSpacing = 4;
	const uint16_t rowHeight = (keyboardHeight - rowSpacing * (rowCount + 1)) / rowCount;

	if( y < keyboardTop + rowSpacing || y >= SCREEN_HEIGHT - rowSpacing )
		return nullptr;

	for( uint8_t row = 0; row < rowCount; ++row ){
		uint16_t rowY = keyboardTop + rowSpacing * (row + 1) + rowHeight * row;
		if( y < rowY || y >= rowY + rowHeight )
			continue;

		const char **keys = nullptr;
		const uint8_t *units = nullptr;
		size_t count = 0;
		switch( row ){
			case 0: keys = numpadRow1; units = numpadRow1Units; count = numpadRow1len; break;
			case 1: keys = numpadRow2; units = numpadRow2Units; count = numpadRow2len; break;
		}

		const uint16_t keySpacingLocal = 4;
		uint16_t totalUnits = 0;
		for( size_t i = 0; i < count; ++i )
			totalUnits += units[i];

		const uint16_t availableWidth = SCREEN_WIDTH - keySpacingLocal * (count + 1);
		const uint16_t unitWidth = availableWidth / totalUnits;
		uint16_t keyX = keySpacingLocal;

		for( size_t i = 0; i < count; ++i ){
			uint16_t w = unitWidth * units[i];
			if( i == count - 1 )
				w = SCREEN_WIDTH - keyX - keySpacingLocal;

			if( x >= keyX && x < keyX + w )
				return keys[i];

			keyX += w + keySpacingLocal;
		}
	}

	return nullptr;
}

bool Display::getNumpadKeyRect(const char *key, uint16_t &outX, uint16_t &outY, uint16_t &outW, uint16_t &outH){
	const uint16_t keyboardTop = SCREEN_HEIGHT / 2;
	const uint16_t keyboardHeight = SCREEN_HEIGHT - keyboardTop;
	const uint8_t rowCount = 2;
	const uint16_t rowSpacing = 4;
	const uint16_t rowHeight = (keyboardHeight - rowSpacing * (rowCount + 1)) / rowCount;

	for( uint8_t row = 0; row < rowCount; ++row ){
		uint16_t rowY = keyboardTop + rowSpacing * (row + 1) + rowHeight * row;

		const char **keys = nullptr;
		const uint8_t *units = nullptr;
		size_t count = 0;
		switch( row ){
			case 0: keys = numpadRow1; units = numpadRow1Units; count = numpadRow1len; break;
			case 1: keys = numpadRow2; units = numpadRow2Units; count = numpadRow2len; break;
		}

		const uint16_t keySpacingLocal = 4;
		uint16_t totalUnits = 0;
		for( size_t i = 0; i < count; ++i )
			totalUnits += units[i];

		const uint16_t availableWidth = SCREEN_WIDTH - keySpacingLocal * (count + 1);
		const uint16_t unitWidth = availableWidth / totalUnits;
		uint16_t keyX = keySpacingLocal;

		for( size_t i = 0; i < count; ++i ){
			uint16_t w = unitWidth * units[i];
			if( i == count - 1 )
				w = SCREEN_WIDTH - keyX - keySpacingLocal;

			if( keys[i] == key ){
				outX = keyX;
				outY = rowY;
				outW = w;
				outH = rowHeight;
				return true;
			}

			keyX += w + keySpacingLocal;
		}
	}

	return false;
}

void Display::drawKeyboardKey(const char *key, bool highlighted){
	uint16_t x, y, w, h;
	if( !getKeyboardKeyRect(key, x, y, w, h) )
		return;

	tft.fillRoundRect(x, y, w, h, 4, TFT_DARKGREY);
	tft.drawRoundRect(x, y, w, h, 4, highlighted ? TFT_GREEN : textColor);

	u8g2.setFont(u8g2_font_helvB14_tf);
	u8g2.setFontMode(1);
	u8g2.setForegroundColor(highlighted ? TFT_GREEN : textColor);

	const int16_t textWidth = u8g2.getUTF8Width(key);
	const int16_t textX = x + (w - textWidth) / 2;
	const int16_t textY = y + h - 6;
	u8g2.drawUTF8(textX, textY, key);
}

bool Display::getKeyboardKeyRect(const char *key, uint16_t &outX, uint16_t &outY, uint16_t &outW, uint16_t &outH){
	const uint16_t keyboardTop = SCREEN_HEIGHT / 2;
	const uint16_t keyboardHeight = SCREEN_HEIGHT - keyboardTop;
	const uint8_t rowCount = 4;
	const uint16_t rowSpacing = 4;
	const uint16_t rowHeight = (keyboardHeight - rowSpacing * (rowCount + 1)) / rowCount;

	// search each row for the key
	for( uint8_t row = 0; row < rowCount; ++row ){
		uint16_t rowY = keyboardTop + rowSpacing * (row + 1) + rowHeight * row;

		const char **keys = nullptr;
		const uint8_t *units = nullptr;
		size_t count = 0;
		switch( row ){
			case 0: keys = keyboardRow1; units = keyboardRow1Units; count = keyboardRow1len; break;
			case 1: keys = keyboardRow2; units = keyboardRow2Units; count = keyboardRow2len; break;
			case 2: keys = keyboardRow3; units = keyboardRow3Units; count = keyboardRow3len; break;
			case 3: keys = keyboardRow4; units = keyboardRow4Units; count = keyboardRow4len; break;
		}

		const uint16_t keySpacingLocal = 4;
		uint16_t totalUnits = 0;
		for( size_t i = 0; i < count; ++i )
			totalUnits += units[i];

		const uint16_t availableWidth = SCREEN_WIDTH - keySpacingLocal * (count + 1);
		const uint16_t unitWidth = availableWidth / totalUnits;
		uint16_t keyX = keySpacingLocal;

		for( size_t i = 0; i < count; ++i ){
			uint16_t w = unitWidth * units[i];
			if( i == count - 1 )
				w = SCREEN_WIDTH - keyX - keySpacingLocal;

			if( keys[i] == key ){
				outX = keyX;
				outY = rowY;
				outW = w;
				outH = rowHeight;
				return true;
			}

			keyX += w + keySpacingLocal;
		}
	}

	return false;
}

void Display::drawTypedBuffer(){
	const uint16_t keyboardTop = SCREEN_HEIGHT / 2;
	const uint16_t typedAreaBottom = keyboardTop - 4;
	const uint16_t typedAreaWidth = SCREEN_WIDTH - 16;
	const uint16_t typedAreaX = 8;

	// 1) Measure top `text` layout height without redrawing it on every key press.
	u8g2.setFont(u8g2_font_helvB14_tf);
	u8g2.setFontMode(1);
	u8g2.setForegroundColor(textColor);

	char topBuf[sizeof(text)];
	strncpy(topBuf, text, sizeof(topBuf));
	topBuf[sizeof(topBuf) - 1] = '\0';

	const uint8_t MAX_LINES = 16;
	uint8_t topLineCount = 0;
	char *topLineStart = topBuf;

	while( *topLineStart && topLineCount < MAX_LINES ){
		while( *topLineStart == ' ' ) topLineStart++;
		if( *topLineStart == '\0' ) break;

		char *lineEnd = topLineStart;
		char *candidate = topLineStart;
		while( *candidate && *candidate != '\n' ){
			char *nextSep = strpbrk(candidate, " \n");
			if( !nextSep ) nextSep = topLineStart + strnlen(topLineStart, 256);

			char saved = *nextSep;
			*nextSep = '\0';
			const uint16_t width = u8g2.getUTF8Width(topLineStart);
			*nextSep = saved;

			if( width > SCREEN_WIDTH ) break;

			lineEnd = nextSep;
			if( saved == '\n' || saved == '\0' ) break;

			candidate = nextSep + 1;
			while( *candidate == ' ' ) candidate++;
		}

		if( lineEnd == topLineStart ){
			char *hardBreak = strpbrk(topLineStart, " \n");
			lineEnd = hardBreak ? hardBreak : topLineStart + strnlen(topLineStart, 256);
		}

		char *nextLine = lineEnd;
		if( *nextLine ) nextLine++;
		*lineEnd = '\0';
		topLineCount++;

		topLineStart = nextLine;
		while( *topLineStart == ' ' || *topLineStart == '\n' ) topLineStart++;
	}

	const uint16_t topLineSpacing = 5;
	const uint16_t topFontHeight = 16 + 4; // matches draw() for non-text
	const uint16_t topLineStep = topFontHeight + topLineSpacing;
	uint16_t topUsedHeight = 0;
	if( topLineCount > 0 ) topUsedHeight = topLineStep * topLineCount - topLineSpacing;

	// 2) Draw the typed buffer area below the top text (bottom-aligned)
	u8g2.setFont(u8g2_font_helvB18_tf);
	u8g2.setFontMode(1);
	u8g2.setForegroundColor(textColor);

	char buffer[sizeof(typedBuffer)];
	strncpy(buffer, typedBuffer, sizeof(buffer));
	buffer[sizeof(buffer) - 1] = '\0';

	const char *lines[MAX_LINES];
	uint8_t lineCount = 0;
	char *lineStart = buffer;

	while( *lineStart && lineCount < MAX_LINES ){
		while( *lineStart == ' ' ) lineStart++;
		if( *lineStart == '\0' ) break;

		char *lineEnd = lineStart;
		char *candidate = lineStart;
		while( *candidate && *candidate != '\n' ){
			char *nextSep = strpbrk(candidate, " \n");
			if( !nextSep ) nextSep = lineStart + strnlen(lineStart, 256);

			char saved = *nextSep;
			*nextSep = '\0';
			const uint16_t width = u8g2.getUTF8Width(lineStart);
			*nextSep = saved;

			if( width > typedAreaWidth ) break;

			lineEnd = nextSep;
			if( saved == '\n' || saved == '\0' ) break;

			candidate = nextSep + 1;
			while( *candidate == ' ' ) candidate++;
		}

		if( lineEnd == lineStart ){
			char *hardBreak = strpbrk(lineStart, " \n");
			lineEnd = hardBreak ? hardBreak : lineStart + strnlen(lineStart, 256);
		}

		char *nextLine = lineEnd;
		if( *nextLine ) nextLine++;
		*lineEnd = '\0';
		lines[lineCount++] = lineStart;

		lineStart = nextLine;
		while( *lineStart == ' ' || *lineStart == '\n' ) lineStart++;
	}

	const uint16_t lineSpacing = 3;
	const uint16_t fontHeight = 18 + 4;
	const uint16_t lineStep = fontHeight + lineSpacing;

	const int16_t typedAreaTop = topUsedHeight > 0 ? (topUsedHeight + 4) : 0;
	const uint8_t maxVisibleLines = (typedAreaBottom - typedAreaTop) / lineStep;
	const uint8_t visibleCount = min(lineCount, maxVisibleLines);

	if( typedAreaTop < typedAreaBottom ){
		tft.fillRect(0, typedAreaTop, SCREEN_WIDTH, typedAreaBottom - typedAreaTop + 1, backgroundColor);
	}

	for( int8_t i = visibleCount - 1; i >= 0; --i ){
		const uint8_t sourceIndex = lineCount - visibleCount + i;
		int16_t y = typedAreaBottom - (visibleCount - 1 - i) * lineStep;
		u8g2.drawUTF8(typedAreaX, y, lines[sourceIndex]);
	}
}

void Display::clearTypedBuffer(){
	typedBuffer[0] = '\0';
	textDirty = true;	
}

void Display::setLedBrightness( const uint8_t brightness ){

	analogWrite(Configuration::PIN_DISP_LED, 255-brightness);

};

void Display::setup(){
	

	pinMode(Configuration::PIN_DISP_LED, OUTPUT);
	setLedBrightness(50);
	Serial.println("STARTING");

	tft.init();
	if( Configuration::INVERT_SCREEN ){
		tft.invertDisplay(true);
		Serial.println("Inverting screen");
	}

	tft.setRotation(3); // 0-3
	tft.fillScreen(colorToU16(Configuration::myColor));

	delay(500);

	ts.begin(tft.getSPIinstance());

	u8g2.begin(tft);
	u8g2.setFont(u8g2_font_helvB18_tf); // Choose a U8g2 font
	u8g2.setFontMode(1); // Transparent background
	u8g2.setForegroundColor(textColor);
	tft.fillScreen(backgroundColor);
	u8g2.drawUTF8(0, 0, "Loading...");

};


void Display::draw(){

	bool deferredKeyboardText = false;
	uint8_t lineCount = 0;
	const uint8_t MAX_LINES = 4;
	const char *lines[MAX_LINES];
	int16_t deferredY = 0;

	if( textDirty ){
		textDirty = false;
	
		tft.fillScreen(backgroundColor);

		if( state == STATE_TEXT ){
			u8g2.setFont(u8g2_font_helvB24_tf); // Choose a U8g2 font
		}
		else{
			u8g2.setFont(u8g2_font_helvB14_tf); // Choose a U8g2 font
		}

		u8g2.setForegroundColor(textColor);

		// Wrap text automatically on spaces and explicit '\n'.
		char buffer[sizeof(text)];
		strncpy(buffer, text, sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0';

		
		
		char *lineStart = buffer;

		while( *lineStart && lineCount < MAX_LINES ){

			while( *lineStart == ' ' )
				lineStart++;
			if( *lineStart == '\0' )
				break;

			char *lineEnd = lineStart;
			char *candidate = lineStart;
			char *lastGoodEnd = lineStart;

			while( *candidate && *candidate != '\n' ){
				char *nextSep = strpbrk(candidate, " \n");
				if( !nextSep ){
					nextSep = lineStart + strnlen(lineStart, 256);
				}

				char saved = *nextSep;
				*nextSep = '\0';
				const uint16_t width = u8g2.getUTF8Width(lineStart);
				*nextSep = saved;

				if( width > SCREEN_WIDTH ){
					break;
				}

				lineEnd = nextSep;
				if( saved == '\n' || saved == '\0' ){
					break;
				}

				lastGoodEnd = nextSep;
				candidate = nextSep + 1;
				while( *candidate == ' ' )
					candidate++;
			}

			if( lineEnd == lineStart ){
				char *hardBreak = strpbrk(lineStart, " \n");
				lineEnd = hardBreak ? hardBreak : lineStart + strnlen(lineStart, 256);
			}

			char *nextLine = lineEnd;
			if( *nextLine ){
				nextLine++;
			}
			*lineEnd = '\0';
			lines[lineCount++] = lineStart;

			lineStart = nextLine;
			while( *lineStart == ' ' || *lineStart == '\n' )
				lineStart++;
		}

		
		if( lineCount > 0 ){
			uint16_t fontHeight = (state == STATE_TEXT ? 24 : 16)+4;

			const uint16_t lineSpacing = 5;
			const uint16_t lineStep = fontHeight + lineSpacing;
			const uint16_t totalHeight = lineStep * lineCount - lineSpacing;
			int16_t y = ( state == STATE_TEXT ) ? (SCREEN_HEIGHT - totalHeight) / 2 + fontHeight : fontHeight;

			if( state == STATE_KEYBOARD ){

				deferredKeyboardText = true;
				deferredY = y;

			}
			else{
				for( uint8_t i = 0; i < lineCount; ++i ){
					const char *line = lines[i];
					const uint16_t width = u8g2.getUTF8Width(line);
					int16_t x = (SCREEN_WIDTH - width) / 2;
					u8g2.drawUTF8(x, y, line);
					y += lineStep;
				}
			}
		}

	}

	if( state == STATE_TEXT ){

	}
	else if( state == STATE_KEYBOARD ){

		drawKeyboard();
		drawTypedBuffer();
		if( deferredKeyboardText ){
			// Redraw the wrapped `text` on top of the typed area so it remains visible
			u8g2.setFont(u8g2_font_helvB14_tf);
			u8g2.setFontMode(1);
			u8g2.setForegroundColor(textColor);

			const uint16_t lineSpacing = 5;
			uint16_t fontHeight = 16 + 4;
			const uint16_t lineStep = fontHeight + lineSpacing;
			for( uint8_t i = 0; i < lineCount; ++i ){

				const char *line = lines[i];
				const uint16_t width = u8g2.getUTF8Width(line);
				int16_t x = (SCREEN_WIDTH - width) / 2;
				u8g2.drawUTF8(x, deferredY, line);
				deferredY += lineStep;

			}

		}

	}
	else if( state == STATE_NUMPAD ){
		drawNumpad();
	}

}

void Display::setTextColor( const uint16_t color ){
	textDirty = true;
	textColor = color;
}

void Display::setBackgroundColor( const uint16_t color ){
	textDirty = true;
	backgroundColor = color;
};

void Display::setText( const char * inputText, const uint8_t length ){
	
	memcpy(text, inputText, length);
	text[length] = '\0';
	Serial.printf("[Display::setText]: Text is now %s\n", text);
	textDirty = true;

}

void Display::setMaxLength( const uint8_t length ){

	maxLength = length;
	if( maxLength > 100 )
		maxLength = 100;

}

void Display::setNumpadMaxVal( const uint32_t val ){ 
	numpadMaxVal = val; 
}

void Display::setStateText(){
	textDirty = true;
	state = STATE_TEXT;
	draw();
}

bool Display::appendTypedKey(const char *key){
	if( strcmp(key, "<<<") == 0 ){
		size_t len = strnlen(typedBuffer, typedBufferLength);
		if( len > 0 ){
			typedBuffer[len - 1] = '\0';
			return true;
		}
		return false;
	}

	size_t currentLen = strnlen(typedBuffer, typedBufferLength);
	size_t keyLen = strnlen(key, 8);
	if( currentLen + keyLen >= sizeof(typedBuffer) )
		return false;

	if( currentLen + keyLen > maxLength )
		return false;

	memcpy(typedBuffer + currentLen, key, keyLen);
	typedBuffer[currentLen + keyLen] = '\0';
	return true;
}

bool Display::appendNumpadKey(const char *key){
	if( strcmp(key, "<<<") == 0 ){
		size_t len = strnlen(typedBuffer, typedBufferLength);
		if( len > 0 ){
			typedBuffer[len - 1] = '\0';
			return true;
		}
		return false;
	}

	if( strnlen(key, 8) != 1 || key[0] < '0' || key[0] > '9' )
		return false;

	size_t currentLen = strnlen(typedBuffer, typedBufferLength);
	if( currentLen + 1 >= sizeof(typedBuffer) )
		return false;

	// Allow leading zeros, but ensure max value is not exceeded.
	char temp[sizeof(typedBuffer)];
	memcpy(temp, typedBuffer, currentLen);
	temp[currentLen] = key[0];
	temp[currentLen + 1] = '\0';

	if( numpadMaxVal > 0 ){
		unsigned long value = strtoul(temp, nullptr, 10);
		if( value > numpadMaxVal )
			return false;
	}

	typedBuffer[currentLen] = key[0];
	typedBuffer[currentLen + 1] = '\0';
	return true;
}

void Display::setStateKeyboard(){
	
	if( state != STATE_KEYBOARD )
		clearTypedBuffer();
	textDirty = true;
	state = STATE_KEYBOARD;
	draw();

}

void Display::setStateNumpad(){
	
	if( state != STATE_NUMPAD )
		clearTypedBuffer();

	textDirty = true;
	state = STATE_NUMPAD;
	draw();

}


void Display::loop(){

	const uint32_t ms = millis();

	if( ts.touched() ){

		TS_Point p = ts.getPoint();
		const int32_t rawX = p.x;
		const int32_t rawY = p.y;
		//Serial.printf("X: %i, Y: %i\n", rawX, rawY);

		const int32_t minRawX = min((int32_t)Display::TOUCH_TOP, (int32_t)Display::TOUCH_BOTTOM);
		const int32_t maxRawX = max((int32_t)Display::TOUCH_TOP, (int32_t)Display::TOUCH_BOTTOM);
		const int32_t minRawY = min((int32_t)Display::TOUCH_LEFT, (int32_t)Display::TOUCH_RIGHT);
		const int32_t maxRawY = max((int32_t)Display::TOUCH_LEFT, (int32_t)Display::TOUCH_RIGHT);

		if( rawX < minRawX || rawX > maxRawX || rawY < minRawY || rawY > maxRawY )
			return;
		
		int16_t xPos = map(rawY, Display::TOUCH_RIGHT, Display::TOUCH_LEFT, 0, SCREEN_WIDTH);
		int16_t yPos = map(rawX, Display::TOUCH_TOP, Display::TOUCH_BOTTOM, SCREEN_HEIGHT, 0);
		xPos = constrain(xPos, 0, SCREEN_WIDTH - 1);
		yPos = constrain(yPos, 0, SCREEN_HEIGHT - 1);

		if( xPos != preX || yPos != preY ){
			//Serial.printf("X: %i, Y: %i\n", xPos, yPos);
			preX = xPos;
			preY = yPos;
		}

		if( state == STATE_KEYBOARD ){

			const char * key = getKeyboardKeyAt(xPos, yPos);
			if( key && !keyboardKeyDown ){
				keyboardKeyDown = true;
				keyboardDownKey = key;
				keyboardHighlightKey = key;
				keyboardHighlightUntil = millis() + 10;
				const bool changed = appendTypedKey(key);
				Serial.printf("KEY PRESSED: %s\n", key);
				drawKeyboardKey(key, true);
				drawTypedBuffer();
				delay(100);
				keyboardHighlightKey = nullptr;
				drawKeyboardKey(key, false);
				if( changed )
					drawTypedBuffer();
			}

		}
		else if( state == STATE_NUMPAD ){
			const char * key = getNumpadKeyAt(xPos, yPos);
			if( key && !keyboardKeyDown ){
				keyboardKeyDown = true;
				keyboardDownKey = key;
				keyboardHighlightKey = key;
				keyboardHighlightUntil = millis() + 10;
				const bool changed = appendNumpadKey(key);
				Serial.printf("NUMPAD PRESSED: %s\n", key);
				drawNumpadKey(key, true);
				drawNumpadEntry();
				delay(100);
				keyboardHighlightKey = nullptr;
				drawNumpadKey(key, false);
				if( changed )
					drawNumpadEntry();
			}
		}
	}
	else {
		keyboardKeyDown = false;
		keyboardDownKey = nullptr;
	}

	


}


