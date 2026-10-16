#include <esp_now.h>
#include <WiFi.h>

// Todo: Some kind of hookup mode
// REPLACE WITH YOUR ESP32-S3 SERVER MAC ADDRESS
uint8_t serverAddress[] = {0x34, 0x85, 0x18, 0x42, 0xA4, 0x64};

// Shared data structure
typedef struct struct_message {
	char msg[32];
	int boardId;
} struct_message;

struct_message myData;
struct_message incomingData;

// Callback when data is received from the Server
void OnDataRecv( const uint8_t * mac, const uint8_t *data, int len ){

	memcpy(&incomingData, data, sizeof(incomingData));
	Serial.print("From Server: ");
	Serial.println(incomingData.msg);

}

// Callback when data is sent
void OnDataSent( const wifi_tx_info_t* mac, esp_now_send_status_t status ){
	Serial.print("Send Status: ");
	Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
	Serial.begin(115200);
	
	// Set as Wi-Fi Station
	WiFi.mode(WIFI_STA);

	// Init ESP-NOW
	if (esp_now_init() != ESP_OK) {
		Serial.println("Error initializing ESP-NOW");
		return;
	}

	// Register callbacks
	esp_now_register_send_cb(OnDataSent);
	esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

	// Register the Server as a peer
	esp_now_peer_info_t peerInfo = {};
	memcpy(peerInfo.peer_addr, serverAddress, 6);
	peerInfo.channel = 0;  
	peerInfo.encrypt = false;
	
	if (esp_now_add_peer(&peerInfo) != ESP_OK) {
		Serial.println("Failed to add server peer");
		return;
	}
	
}

void loop() {
	// Set values to send
	strcpy(myData.msg, "Hello Server!");
	myData.boardId = 1; // Give each client a unique ID (1, 2, 3...)

	// Send message to Server
	esp_now_send(serverAddress, (uint8_t *) &myData, sizeof(myData));

	// Wait 5 seconds
	delay(5000);
}