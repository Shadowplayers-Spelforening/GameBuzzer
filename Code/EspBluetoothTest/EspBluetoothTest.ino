#include <esp_now.h>
#include <WiFi.h>

// Shared data structure
typedef struct struct_message {
	char msg[32];
	int boardId;
} struct_message;

struct_message incomingData;
struct_message outgoingData;

// Callback when data is received from a client
void OnDataRecv( const uint8_t * mac, const uint8_t *data, int len ){

	// 1. Check if this client is already in the peer list
	if( !esp_now_is_peer_exist(mac) ){

		esp_now_peer_info_t peerInfo = {};
		memcpy(peerInfo.peer_addr, mac, 6);
		peerInfo.channel = 0;  // Use current Wi-Fi channel
		peerInfo.encrypt = false;
		
		if (esp_now_add_peer(&peerInfo) == ESP_OK) {
			Serial.printf("New Peer Added: %02X:%02X:%02X:%02X:%02X:%02X\n", 
						  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}

	}

	// 2. Process incoming message
	memcpy(&incomingData, data, sizeof(incomingData));
	Serial.printf("[%d] Received: %s\n", incomingData.boardId, incomingData.msg);

	// 3. Optional: Send immediate acknowledgement back to THIS specific client
	strcpy(outgoingData.msg, "ACK from S3");
	outgoingData.boardId = 0;
	esp_now_send(mac, (uint8_t *) &outgoingData, sizeof(outgoingData));

}

// Callback when data is sent
void OnDataSent(const wifi_tx_info_t * info, esp_now_send_status_t status) {
  // Serial.print("Last Packet Send Status: ");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {

	Serial.begin(115200);
	
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
	esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

}

void loop() {
	// This loop stays clean because registration and replies 
	// happen automatically in the OnDataRecv callback.
	delay(1000);
}