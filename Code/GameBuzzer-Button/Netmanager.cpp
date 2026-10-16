#include "Netmanager.h"

uint8_t Netmanager::serverAddress[6];
struct_message Netmanager::myData = {};				// Cache of outgoing data
struct_message Netmanager::incomingData = {};		// Cache of incoming data
uint32_t Netmanager::lastPing = 0;
bool Netmanager::connected = false;
uint32_t Netmanager::lastAck = 0;

Netmanager::Netmanager(){}


// Callback when data is received from the Server or broadcast address
void Netmanager::OnDataRecv( const esp_now_recv_info_t* messageInfo, const uint8_t *data, int len ){

	Serial.print("Received data: ");
	Serial.println((char*)data);
	memcpy(&incomingData, data, sizeof(incomingData));
	if( strcmp(incomingData.prefix, "BUZ") != 0 ){
		Serial.println("Invalid prefix");
		return;
	}

	if( incomingData.task != S_ACK )
		Serial.printf("[%d] From Server (task %d): %s\n", incomingData.color, incomingData.task, incomingData.args);

	bool isForThisButton = incomingData.color == Configuration::myColor || incomingData.color == ButtonColor::ALL;
	if( !isForThisButton ){
		Serial.println("Message not addressed to this button");
		return;
	}

	// Compare mac address. Broadcast is only allowed if connected is false
	if( !connected && incomingData.task != S_HOOKUP && memcmp(messageInfo->src_addr, BROADCAST_ADDRESS, sizeof(BROADCAST_ADDRESS)) != 0 ){
		Serial.println("Broadcast is only allowed if not connected");
		return;
	}



	if( incomingData.task == S_HOOKUP ){

		uint8_t myMac[6];
		WiFi.macAddress(myMac);

		// Print the mac address
		Serial.printf("Received hookup from: %02X:%02X:%02X:%02X:%02X:%02X\n", messageInfo->src_addr[0], messageInfo->src_addr[1], messageInfo->src_addr[2], messageInfo->src_addr[3], messageInfo->src_addr[4], messageInfo->src_addr[5]);
		Serial.printf("myMac: %02X:%02X:%02X:%02X:%02X:%02X\n", myMac[0], myMac[1], myMac[2], myMac[3], myMac[4], myMac[5]);
		Serial.printf("Callback mac: %02X:%02X:%02X:%02X:%02X:%02X\n", incomingData.args[0], incomingData.args[1], incomingData.args[2], incomingData.args[3], incomingData.args[4], incomingData.args[5]);

		// Compare the data with our mac address
		if( memcmp(myMac, incomingData.args, 6) == 0 ){
			// add server to peers

			esp_now_peer_info_t peerInfo = {};
			memcpy(peerInfo.peer_addr, messageInfo->src_addr, 6);
			peerInfo.channel = 0;  // Use current Wi-Fi channel
			peerInfo.encrypt = false;

			if ( esp_now_add_peer(&peerInfo) == ESP_OK ){
				Serial.printf("New Server Peer Added: %02X:%02X:%02X:%02X:%02X:%02X\n", 
							messageInfo->src_addr[0], messageInfo->src_addr[1], messageInfo->src_addr[2], messageInfo->src_addr[3], messageInfo->src_addr[4], messageInfo->src_addr[5]);
			}

			// Save to serverAddress
			memcpy(serverAddress, messageInfo->src_addr, 6);

			Serial.println("Connected to server");
			connected = true;
			lastAck = millis();

		}


	}
	else if( incomingData.task == S_ACK && connected ){
		lastAck = millis();
	}

	else if( incomingData.task == S_PING ){

		Serial.println("Received broadcast S_PING, sending announce");
		taskAnnounce( !connected );
		
	}

	else if( incomingData.task == S_GET_TEXT ){

		uint8_t length = sizeof(Display::typedBuffer);
		if( Display::maxLength < length )
			length = Display::maxLength;

		taskText( length, Display::typedBuffer );

	}

	else if( incomingData.task == S_SHOW_KEYBOARD ){

		Button::toggleLed(true);
		Display::setBackgroundColor((uint16_t)((incomingData.args[0] << 8) | incomingData.args[1]));
		Display::setTextColor((uint16_t)((incomingData.args[2] << 8) | incomingData.args[3]));
		Display::setMaxLength(incomingData.args[4]);
		Serial.printf("Got keyboard with max length %i, text length %i\n", incomingData.args[4], incomingData.args[5]);
		Display::setText( (char*)incomingData.args + 6, incomingData.args[5]+1 ); // The text received doesn't include null character
		Display::setStateKeyboard();
		
	}

	else if( incomingData.task == S_SHOW_TEXT ){

		Display::setBackgroundColor((uint16_t)((incomingData.args[0] << 8) | incomingData.args[1]));
		Display::setTextColor((uint16_t)((incomingData.args[2] << 8) | incomingData.args[3]));
		Display::setText( (char*)incomingData.args + 5, incomingData.args[4] );
		Display::setStateText();

	}

	else if( incomingData.task == S_TOGGLE_BUTTON ){

		const bool on = (bool)incomingData.args[0];
		Serial.printf("Toggling led %s\n", on ? "ON" : "OFF");
		Button::toggleLed( on );

	}
	else if( incomingData.task == S_SHOW_NUMPAD ){

		Button::toggleLed(true);

		Display::setBackgroundColor((uint16_t)((incomingData.args[0] << 8) | incomingData.args[1]));
		Display::setTextColor((uint16_t)((incomingData.args[2] << 8) | incomingData.args[3]));
		// Convert args 4 -> 7 to max val, big endian
		Display::setNumpadMaxVal(((uint32_t)incomingData.args[4] << 24) | ((uint32_t)incomingData.args[5] << 16) | ((uint32_t)incomingData.args[6] << 8) | (uint32_t)incomingData.args[7]);
		Display::setText( (char*)incomingData.args + 9, incomingData.args[8]+1 );
		Display::setStateNumpad();

	}

}

// Callback when data is sent
void Netmanager::OnDataSent( const wifi_tx_info_t* mac, esp_now_send_status_t status ){
	
	const bool success = status == ESP_NOW_SEND_SUCCESS;
	Serial.print("Send Status: ");
	Serial.println(success ? "Success" : "Fail");

}

void Netmanager::taskAnnounce( const bool hookup ){ 
	const char args[2] = { hookup ? 1 : 0, 0 };
	sendTask(C_ANNOUNCE, args, hookup); 
};


void Netmanager::setup(){

	// Set as Wi-Fi Station
	WiFi.mode(WIFI_STA);

	// Init ESP-NOW
	if (esp_now_init() != ESP_OK) {
		Serial.println("Error initializing ESP-NOW");
		return;
	}

	// Register callbacks
	esp_now_register_send_cb(OnDataSent);
	esp_now_register_recv_cb(OnDataRecv);

	// Register the Server as a peer
	esp_now_peer_info_t peerInfo = {};
	memcpy(peerInfo.peer_addr, serverAddress, 6);

	peerInfo.channel = 0;  
	peerInfo.encrypt = false;
	
	if( esp_now_add_peer(&peerInfo) != ESP_OK ){
		Serial.println("Failed to add server peer");
		return;
	}

	// Also register the broadcast address so broadcast tasks can be received reliably
	esp_now_peer_info_t broadcastPeer = {};
	memcpy(broadcastPeer.peer_addr, BROADCAST_ADDRESS, 6);
	broadcastPeer.channel = 0;
	broadcastPeer.encrypt = false;
	if( esp_now_add_peer(&broadcastPeer) != ESP_OK ){
		Serial.println("Failed to add broadcast peer");
	}
	
	myData.color = Configuration::myColor;

}

void Netmanager::taskText( const uint8_t length, const char *text ){

	char args[128];
	if( length > 120 ){
		Serial.println("Error: Trying to send too long text!");
		return;
	}

	args[0] = length;
	strncpy(args + 1, text, length);
	sendTask(C_TEXT, args);

}

void Netmanager::sendTask( Command command, const char *args, bool broadcast ){

	strcpy(myData.prefix, "BUZ");
	myData.task = command;
	memset(myData.args, 0, 128);
	if( args != nullptr )
		strncpy(myData.args, args, 128);
	
	Serial.printf("[sendTask] color %i, task %i, broadcast %d, data, %s\n", myData.color, myData.task, broadcast, myData.args);
	// Send message to Server
	esp_now_send(broadcast ? BROADCAST_ADDRESS : serverAddress, (uint8_t *) &myData, sizeof(myData));
	
};

void Netmanager::loop(){


	const uint32_t ms = millis();
	if( !lastPing || ms - lastPing > PING_FREQ ){
		
		Serial.println("Sending ping");
		taskAnnounce( !connected );
		lastPing = ms;

	}

	if( connected && ms-lastAck > TIMEOUT ){
		connected = false;
		Serial.println("Server timed out, trying to find a new one");
	}

};

