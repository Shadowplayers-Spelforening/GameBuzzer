#include "Netmanager.h"

struct_message Netmanager::myData;
struct_message Netmanager::incomingData;
RemoteButton Netmanager::buttons[6];

Netmanager::Netmanager(){}

// Handles tasks received from remotes
void Netmanager::OnDataRecv( const esp_now_recv_info_t* messageInfo, const uint8_t *data, int len ){

	// Process incoming message
	memcpy(&incomingData, data, sizeof(incomingData));
	if( strcmp(incomingData.prefix, "BUZ") != 0 ){
		Serial.println("Invalid prefix");
		return;
	}

	const ButtonColor color = incomingData.color;
	if( incomingData.task != C_ANNOUNCE )
		Serial.printf("[%d] Received: (%s) %s\n", color, command2name(incomingData.task), incomingData.args);

	if( color < ButtonColor::RED || color > ButtonColor::WHITE ){
		Serial.println("Invalid color!");
		return;
	}

	// Check if mac matches color on file
	if( buttons[color].isConnected() && memcmp(messageInfo->src_addr, buttons[color].mac, 6) != 0 ){
		Serial.println("Invalid mac address!");
		return;
	}

	const uint8_t task = incomingData.task;

	if( !buttons[color].isConnected() ){
		
		if( task != C_ANNOUNCE ){
			Serial.println("Rejected call, not connected");
			return;
		}

		if( !incomingData.args[0] ){
			Serial.println("Rejected call, not in hookup mode");
			return;
		}

	}

	buttons[color].lastPing = millis();

	// Add peer if needed so we can send to it
	if( !esp_now_is_peer_exist(messageInfo->src_addr) ){

		esp_now_peer_info_t peerInfo = {};
		memcpy(peerInfo.peer_addr, messageInfo->src_addr, 6);
		peerInfo.channel = 0;  // Use current Wi-Fi channel
		peerInfo.encrypt = false;

		// Copy mac to button
		memcpy(buttons[color].mac, messageInfo->src_addr, 6);
		
		if ( esp_now_add_peer(&peerInfo) == ESP_OK ){
			Serial.printf("New Peer Added: %02X:%02X:%02X:%02X:%02X:%02X\n", 
						messageInfo->src_addr[0], messageInfo->src_addr[1], messageInfo->src_addr[2], messageInfo->src_addr[3], messageInfo->src_addr[4], messageInfo->src_addr[5]);
		}

	}

	// Wants to hook up to server, need to reply with a broadcast and the sender's mac address to allow
	if( task == C_ANNOUNCE ){

		if( incomingData.args[0] ){

			// Tell the button it's connected. This reply includes the sender's mac address, and has to be sent globally
			Serial.printf("Replying to incoming mac: %02X:%02X:%02X:%02X:%02X:%02X\n", messageInfo->src_addr[0], messageInfo->src_addr[1], messageInfo->src_addr[2], messageInfo->src_addr[3], messageInfo->src_addr[4], messageInfo->src_addr[5]);
			sendTask( color, S_HOOKUP, (const char *)messageInfo->src_addr, 6, true);
			SerialOut::sendButtonConnectionStatus(color, true);
			buttons[color].stateConnected = true;

		}
		else{
			// If connected, send a simple ack
			sendTask(color, S_ACK, nullptr, 0, false);
		}

	}
	// Button press
	else if( task == C_PRESS ){
		buttons[color].lastPress = millis();
		SerialOut::sendButtonPress(color);
	}
	else if( task == C_TEXT ){

		uint8_t cbLength = incomingData.args[0];
		char cbText[cbLength + 1];
		memcpy(cbText, incomingData.args + 1, cbLength + 1);
		cbText[cbLength] = '\0';
		Serial.printf("Got text: %s\n", cbText);
		SerialOut::sendText(color, cbText);

	}
	

	// 3. Optional: Send immediate acknowledgement back to THIS specific client
	/*
	strcpy(outgoingData.msg, "ACK from S3");
	outgoingData.boardId = 0;
	esp_now_send(mac, (uint8_t *) &outgoingData, sizeof(outgoingData));
	*/

}


void Netmanager::setup(){

	// Set device as a Wi-Fi Station
	WiFi.mode(WIFI_STA);
	while( WiFi.status() == WL_STOPPED )
		delay(10);

	Serial.print("Server MAC: ");
	Serial.println(WiFi.macAddress());

	// Init ESP-NOW
	if( esp_now_init() != ESP_OK ){
		Serial.println("Error initializing ESP-NOW");
		return;
	}

	// Register callbacks
	esp_now_register_send_cb(OnDataSent);
	esp_now_register_recv_cb(OnDataRecv);

	// Add a broadcast peer so server-side broadcast sends work reliably
	esp_now_peer_info_t broadcastPeer = {};
	memcpy(broadcastPeer.peer_addr, BROADCAST_ADDRESS, 6);
	broadcastPeer.channel = 0;
	broadcastPeer.encrypt = false;
	if( esp_now_add_peer(&broadcastPeer) != ESP_OK ){
		Serial.println("Failed to add broadcast peer");
	}

	sPing();
	Serial.println("Sent Ping!");

}

void Netmanager::sendTask( const ButtonColor color, const Command command, const char *args, size_t argsLen, const bool broadcast ){

	strcpy(myData.prefix, "BUZ");
	myData.color = color;
	myData.task = command;
	// Clear buffer then copy binary args up to 128 bytes
	memset(myData.args, 0, 128);
	if( args != nullptr && argsLen ){
		if( argsLen > 128 ) argsLen = 128;
		memcpy(myData.args, args, argsLen);
	}
    Serial.printf("Sending color %i, task %i, data, %s\n", myData.color, myData.task, myData.args);

	if( broadcast )
		esp_now_send(BROADCAST_ADDRESS, (uint8_t *) &myData, sizeof(myData));
	else{
		for( uint8_t i = 0; i < NUM_BUTTONS; ++i ){
			if( i != color && color != ALL )
				continue;
			if( !buttons[i].isConnected() )
				continue;

			// Send message to Server
			esp_err_t status = esp_now_send(buttons[i].mac, (uint8_t *) &myData, sizeof(myData));
			if( status != ESP_OK ) {
				Serial.printf("Broadcast send failed: %d\n", status);
			}
			else{
				Serial.printf("Successful send to mac: %02X:%02X:%02X:%02X:%02X:%02X\n", buttons[i].mac[0], buttons[i].mac[1], buttons[i].mac[2], buttons[i].mac[3], buttons[i].mac[4], buttons[i].mac[5]);
			}

		}
	}
    
};

void Netmanager::sToggleButton( const ButtonColor targetIndex, const bool on ){
	Serial.printf("Toggle button %i %s \n", targetIndex, on ? "ON" : "OFF");
	char args[2];
	args[0] = on ? 1 : 0;
	args[1] = 0;
	sendTask(targetIndex, S_TOGGLE_BUTTON, args, 1);
};

void Netmanager::sShowText( const ButtonColor targetIndex, const uint16_t backgroundColor, const uint16_t textColor, const uint8_t textLength, const char *text ){
	// Pack: uint16_t backgroundColor (big-endian), uint16_t textColor (big-endian), uint8_t textLength, text bytes
	uint8_t args[128];
	const size_t headerSize = 5; // 6
	size_t maxText = 128 - headerSize;
	size_t tlen = (text != nullptr) ? (size_t)textLength : 0;
	if( tlen > maxText ) 
		tlen = maxText;

	// backgroundColor big-endian
	args[0] = (uint8_t)((backgroundColor >> 8) & 0xFF);
	args[1] = (uint8_t)(backgroundColor & 0xFF);
	// textColor big-endian
	args[2] = (uint8_t)((textColor >> 8) & 0xFF);
	args[3] = (uint8_t)(textColor & 0xFF);
	// textLength
	args[4] = (uint8_t)tlen;

	if( tlen )
		memcpy(args + headerSize, text, tlen);

	Serial.printf("Sending text len %i, maxText %i -> %s\n", tlen, maxText, text);

	sendTask(targetIndex, S_SHOW_TEXT, (const char *)args, headerSize + tlen);
};

void Netmanager::sShowKeyboard( const ButtonColor targetIndex, const uint16_t backgroundColor, const uint16_t textColor, const uint8_t maxLength, const uint8_t textLength, const char *text ){
	// Pack: uint8_t maxLength, uint16_t textLength (big-endian), text bytes
	uint8_t args[128];
	const size_t headerSize = 6; 
	size_t maxText = 128 - headerSize;
	size_t tlen = 0;
	if( text != nullptr ) 
		tlen = strnlen(text, maxText);
	if( tlen > maxText ) 
		tlen = maxText;

	// backgroundColor big-endian
	args[0] = (uint8_t)((backgroundColor >> 8) & 0xFF);
	args[1] = (uint8_t)(backgroundColor & 0xFF);
	// textColor big-endian
	args[2] = (uint8_t)((textColor >> 8) & 0xFF);
	args[3] = (uint8_t)(textColor & 0xFF);
	// maxLength
	args[4] = maxLength;
	// textLength
	args[5] = (uint8_t)tlen;
	
	if( tlen ){
		memcpy(args + headerSize, text, tlen);
	}
	sendTask(targetIndex, S_SHOW_KEYBOARD, (const char *)args, headerSize + tlen);
};

void Netmanager::sShowNumpad( const ButtonColor targetIndex, const uint16_t backgroundColor, const uint16_t textColor, const uint32_t maxValue, const uint8_t textLength, const char *text ){
	// Pack: uint8_t maxLength, uint16_t textLength (big-endian), text bytes
	uint8_t args[128];
	const size_t headerSize = 9; 
	size_t maxText = 128 - headerSize;
	size_t tlen = 0;
	if( text != nullptr ) 
		tlen = strnlen(text, maxText);
	if( tlen > maxText ) 
		tlen = maxText;

	// backgroundColor big-endian
	args[0] = (uint8_t)((backgroundColor >> 8) & 0xFF);
	args[1] = (uint8_t)(backgroundColor & 0xFF);
	// textColor big-endian
	args[2] = (uint8_t)((textColor >> 8) & 0xFF);
	args[3] = (uint8_t)(textColor & 0xFF);
	// maxValue (32bit uint)
	args[4] = (uint8_t)((maxValue>>24) & 0xFF);
	args[5] = (uint8_t)((maxValue>>16) & 0xFF);
	args[6] = (uint8_t)((maxValue>>8) & 0xFF);
	args[7] = (uint8_t)(maxValue & 0xFF);
	
	// textLength
	args[8] = (uint8_t)tlen;
	
	if( tlen ){
		memcpy(args + headerSize, text, tlen);
	}
	sendTask(targetIndex, S_SHOW_NUMPAD, (const char *)args, headerSize + tlen);
};

void Netmanager::sGetText( const ButtonColor targetIndex ){
	sendTask(targetIndex, S_GET_TEXT, nullptr, 0);
};

void Netmanager::sPing(){
	sendTask(ButtonColor::ALL, S_PING, nullptr, 0);
}

void Netmanager::loop(){

	// Need to detect button disconnections
	for( uint8_t i = 0; i < NUM_BUTTONS; ++i ){
		
		if( buttons[i].stateConnected && !buttons[i].isConnected() ){
			Serial.printf("[Netmanager] Button %i disconnected\n", i);
			buttons[i].stateConnected = false;
			SerialOut::sendButtonConnectionStatus((ButtonColor)i, false);
		}

	}


};


