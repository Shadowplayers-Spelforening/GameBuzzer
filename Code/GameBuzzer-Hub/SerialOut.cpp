#include "SerialOut.h"

// tasks are sent as <SPBUZ>[int task, arg0 (usually color), arg1, arg2...]</SPBUZ>
namespace SerialOut{

	const char PREFIX[] = "<SPBUZ>";
	const char SUFFIX[] = "</SPBUZ>";
	const char VERSION[] = "2026-06-24";

	char inputBuffer[256];
	size_t inputBufferPos;
	uint32_t readStart;

	void handleRaw();
	void send( const char * jsonString ){
		Serial.printf("%s%.*s%s", PREFIX, (int)strnlen(jsonString, 256), jsonString, SUFFIX);
	}

	void sendInit(){

		StaticJsonDocument<64> doc;
		JsonArray arr = doc.to<JsonArray>();
		arr.add((int)SOUT_INIT);
		arr.add(VERSION);

		char buf[64];
		const size_t len = serializeJson(doc, buf, sizeof(buf) - 1);
		buf[len] = '\0';
		send(buf);

	}

	void sendButtonPress( const ButtonColor color ){
		StaticJsonDocument<32> doc;
		JsonArray arr = doc.to<JsonArray>();
		arr.add((int)SOUT_BTN_PRESS);
		arr.add((int)color);
		char buf[32];
		const size_t len = serializeJson(doc, buf, sizeof(buf) - 1);
		buf[len] = '\0';
		send(buf);
	}
	void sendText( const ButtonColor color, const char * text ){
		StaticJsonDocument<256> doc;
		JsonArray arr = doc.to<JsonArray>();
		arr.add((int)SOUT_TEXT);
		arr.add((int)color);
		arr.add(text);
		char buf[256];
		const size_t len = serializeJson(doc, buf, sizeof(buf) - 1);
		buf[len] = '\0';
		send(buf);
	}
	void sendButtonConnectionStatus( const ButtonColor color, const bool connected ){
		
		StaticJsonDocument<32> doc;
		JsonArray arr = doc.to<JsonArray>();
		arr.add(connected ? (int)SOUT_CONN_BTN : (int)SOUT_DISC_BTN);
		arr.add((int)color);
		char buf[32];
		const size_t len = serializeJson(doc, buf, sizeof(buf) - 1);
		buf[len] = '\0';
		send(buf);

	}


	void handleCommand( const char * jsonArray, const size_t length ){
		// Copy to mutable buffer and ensure null-termination
		char buf[sizeof(inputBuffer)];
		size_t copyLen = length < sizeof(buf) - 1 ? length : sizeof(buf) - 1;
		memcpy(buf, jsonArray, copyLen);
		buf[copyLen] = '\0';

		DynamicJsonDocument doc(length + 128);
		auto derr = deserializeJson(doc, buf);
		if( derr ){
			Serial.print("JSON parse error: ");
			Serial.println(derr.c_str());
			return;
		}
		if( !doc.is<JsonArray>() ){
			Serial.println("Expected JSON array");
			return;
		}
		JsonArray arr = doc.as<JsonArray>();
		if( arr.size() < 1 ){
			Serial.println("Invalid command array");
			return;
		}

		int command = arr[0].as<int>();
		Serial.printf("Handling serial command %i\n", command);
		if( command == SIN_PING ){
			sendInit();
		}

		else if( command == SIN_TOGGLE_BUTTON ){
			if( arr.size() < 3 ){
				Serial.println("SIN_TOGGLE_BUTTON: invalid args");
				return;
			}
			int color = arr[1].as<int>();
			bool on = arr[2].as<bool>();
			Netmanager::sToggleButton( (ButtonColor)color, on );
		}
		else if( command == SIN_SHOW_TEXT ){

			if( arr.size() < 4 ){
				Serial.println("SIN_SHOW_TEXT: invalid args");
				return;
			}
			ButtonColor color = (ButtonColor)arr[1].as<int>();
			uint16_t background = (uint16_t)arr[2].as<int>();
			uint16_t textColor = (uint16_t)arr[3].as<int>();
			const char *textPayload = arr[4].as<const char*>();
			Serial.printf("ShowText: %d %d %d %d %s\n", (uint8_t)color, background, textColor, sizeof(textPayload), textPayload);
			Netmanager::sShowText( color, background, textColor, strnlen(textPayload, 128), textPayload );

		}
		else if( command == SIN_SHOW_KEYBOARD ){
			if( arr.size() < 6 ){
				Serial.println("SIN_SHOW_KEYBOARD: invalid args");
				return;
			}
			ButtonColor color = (ButtonColor)arr[1].as<int>();
			uint16_t background = (uint16_t)arr[2].as<int>();
			uint16_t textColor = (uint16_t)arr[3].as<int>();
			uint8_t maxLength = (uint8_t)arr[4].as<int>();
			const char *textPayload = arr[5].as<const char*>();
			Serial.printf("[showKeyboard] %d %d %d %d %d %s\n", (uint8_t)color, background, textColor, maxLength, strnlen(textPayload, 128), textPayload);
			Netmanager::sShowKeyboard( color, background, textColor, maxLength, strnlen(textPayload, 128), textPayload );
		}
		else if( command == SIN_SHOW_NUMPAD ){
			if( arr.size() < 6 ){
				Serial.println("SIN_SHOW_NUMPAD: invalid args");
				return;
			}
			ButtonColor color = (ButtonColor)arr[1].as<int>();
			uint16_t background = (uint16_t)arr[2].as<int>();
			uint16_t textColor = (uint16_t)arr[3].as<int>();
			uint32_t maxValue = (uint32_t)arr[4].as<uint32_t>();
			const char *textPayload = arr[5].as<const char*>();
			Serial.printf("[showNumpad] %d %d %d %d %d %s\n", (uint8_t)color, background, textColor, maxValue, strnlen(textPayload, 128), textPayload);
			Netmanager::sShowNumpad( color, background, textColor, maxValue, strnlen(textPayload, 128), textPayload );


		}
		else if( command == SIN_GET_TEXT ){
			if( arr.size() < 2 ){
				Serial.println("SIN_GET_TEXT: invalid args");
				return;
			}
			Netmanager::sGetText( (ButtonColor)arr[1].as<int>() );
		}
		else if( command == SIN_GET_CONNECTED ){
			for( uint8_t i = 0; i < Netmanager::NUM_BUTTONS; ++i ){

				if( Netmanager::buttons[i].isConnected() )
					sendButtonConnectionStatus( (ButtonColor)i, true );
				
			}
		}
		else{
			Serial.println("Unknown command received");
		}
	}

	void setup(){
		sendInit();
	}

	void loop(){
		
		while( Serial.available() > 0 ){

			if( !readStart )
				readStart = millis();

			inputBuffer[inputBufferPos++] = Serial.read();
			if( inputBufferPos >= sizeof(inputBuffer) - 1 ){
				handleRaw();
			}

		}

		if( millis() - readStart > 10 ){
			handleRaw();
			inputBufferPos = 0;
			readStart = 0;
		}
			
	}
	
	// Text is received with PREFIX + JSON array + SUFFIX
	// The first element of the array is the task/command.
	void handleRaw(){

		// Parse any complete framed messages from inputBuffer.
		const size_t prefixLen = strlen(PREFIX);
		const size_t suffixLen = strlen(SUFFIX);
		if(prefixLen == 0 || suffixLen == 0) return;

		size_t readPos = 0;
		while( readPos + prefixLen + suffixLen <= inputBufferPos ){
			// Find prefix at or after readPos
			size_t p = readPos;
			for( ; p + prefixLen <= inputBufferPos; ++p ){
				if( strncmp(inputBuffer + p, PREFIX, prefixLen) == 0 )
					break;
			}
			if( p + prefixLen + suffixLen > inputBufferPos ){
				break;
			}

			const char *argsStart = inputBuffer + p + prefixLen;
			size_t argsAvailable = inputBufferPos - (p + prefixLen);

			char *suffixPos = nullptr;
			for( size_t j = 0; j + suffixLen <= argsAvailable; ++j ){
				if( strncmp(argsStart + j, SUFFIX, suffixLen) == 0 ){
					suffixPos = (char*)(argsStart + j);
					break;
				}
			}
			if(!suffixPos){
				break;
			}

			size_t argLen = (size_t)(suffixPos - argsStart);
			*suffixPos = '\0';

			handleCommand(argsStart, argLen);

			readPos = (size_t)(suffixPos - inputBuffer) + suffixLen;
		}

		// Compact remaining bytes to the start of the buffer
		if(readPos > 0){

			size_t remaining = inputBufferPos - readPos;
			if(remaining > 0){
				memmove(inputBuffer, inputBuffer + readPos, remaining);
			}
			inputBufferPos = remaining;
			if(inputBufferPos == 0)
				readStart = 0;

		}

	}

	
	

};



