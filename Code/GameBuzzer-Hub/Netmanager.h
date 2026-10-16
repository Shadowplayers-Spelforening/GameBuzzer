#pragma once
#include <arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "Shared.h"
#include "SerialOut.h"

// Netmanager for server

// Shared data structure
typedef struct struct_message {
	char prefix[4];
	ButtonColor color;
	Command task;
	char args[128];
} struct_message;

class RemoteButton{
	static const uint32_t TIMEOUT = 30000;
	public:
		uint32_t lastPing = 0;
		uint32_t lastPress = 0;
		uint8_t mac[6];
		bool stateConnected = false;
		bool isConnected(){ return lastPing && millis() - lastPing < TIMEOUT; };
		bool isPressed(){ return lastPress && millis() - lastPress < 250; };
};

class Netmanager{

	// Todo: Some kind of hookup mode
	// REPLACE WITH YOUR ESP32-S3 SERVER MAC ADDRESS
	static struct_message myData;
	static struct_message incomingData;
	static constexpr uint8_t BROADCAST_ADDRESS[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	public:
		static const size_t NUM_BUTTONS = 6;
		static RemoteButton buttons[NUM_BUTTONS];		// The index corresponds to a value from ButtonColor enum in Shared.h

		// Callback when data is received from a client
		static void OnDataRecv( const esp_now_recv_info_t* messageInfo, const uint8_t *data, int len );
		// Callback when data is sent
		static void OnDataSent(const wifi_tx_info_t * info, esp_now_send_status_t status) {
			// Serial.print("Last Packet Send Status: ");
			// Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
		}
		Netmanager();

		static void sToggleButton( const ButtonColor targetIndex, const bool on );
		static void sShowText( const ButtonColor targetIndex, const uint16_t backgroundColor, const uint16_t textColor, const uint8_t textLength, const char *text );
		static void sShowKeyboard( const ButtonColor targetIndex, const uint16_t backgroundColor, const uint16_t textColor, const uint8_t maxLength, const uint8_t textLength, const char *text );	
		static void sShowNumpad( const ButtonColor targetIndex, const uint16_t backgroundColor, const uint16_t textColor, const uint32_t maxValue, const uint8_t textLength, const char *text );	
		static void sGetText( const ButtonColor targetIndex );
		static void sPing();

		static void sendTask( const ButtonColor targetIndex, const Command command, const char *args, size_t argsLen, const bool broadcast = false );
		static void loop();
		static void setup();
		
	private:

};

