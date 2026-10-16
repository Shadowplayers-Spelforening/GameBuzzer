#pragma once
#include "_Configuration.h"
#include <arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "Button.h"
#include "Display.h"

// Netmanager for client

// Shared data structure
typedef struct struct_message {
	char prefix[4];
	ButtonColor color;
	Command task;
	char args[128];
} struct_message;


class Netmanager{

	static const uint32_t PING_FREQ = 10000;
	static const uint32_t TIMEOUT = 30000;

	// Todo: Some kind of hookup mode
	// REPLACE WITH YOUR ESP32-S3 SERVER MAC ADDRESS
	static uint8_t serverAddress[6];
	static struct_message myData;
	static struct_message incomingData;
	static constexpr uint8_t BROADCAST_ADDRESS[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	static bool connected;
	static uint32_t lastAck;

	// Callback when data is received from the Server or broadcast address
	static void OnDataRecv( const esp_now_recv_info_t* messageInfo, const uint8_t *data, int len );

	// Callback when data is sent
	static void OnDataSent( const wifi_tx_info_t* mac, esp_now_send_status_t status );


	public:

		Netmanager();

		static void sendTask( Command command, const char *args, bool broadcast = false );
		static void loop();
		static void setup();

		static void taskPressed(){ sendTask(C_PRESS, nullptr); };
		static void taskAnnounce( const bool hookup );
		static void taskText( const uint8_t length, const char *text );
		
	private:
		static uint32_t lastPing;


};
