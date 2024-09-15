// MIT License
//
// Copyright 2024 Eurofurence e.V. 
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the “Software”),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @author 32
 */

#include <EFLed.h>
#include <EFLogging.h>
#include "FSMState.h"

#include <algorithm>
#include <WiFi.h>
#include <painlessMesh.h>
#include "mbedtls/base64.h"

//Game variables
#define NUM_HUES 11
#define HUE_WEIGHT 0.5
uint8_t own_hue = 0;
uint8_t hue_consensus[NUM_HUES];

// Mesh credentials
#define MESH_PREFIX "EF28_ESP_MESH"
#define MESH_PASSWORD "********" //<-- Super-secure! Cannot even see it in code.
#define MESH_PORT 7777
Scheduler userScheduler;
painlessMesh mesh;

uint8_t rainbow[] = {1,24,47,72,96,116,140,164,186,210,232};

std::vector<CRGB> bar = {
  CHSV(rainbow[0], 255, 255),
  CHSV(rainbow[1], 255, 255),
  CHSV(rainbow[2], 255, 255),
  CHSV(rainbow[3], 255, 255),
  CHSV(rainbow[4], 255, 255),
  CHSV(rainbow[5], 255, 255),
  CHSV(rainbow[6], 255, 255),
  CHSV(rainbow[7], 255, 255),
  CHSV(rainbow[8], 255, 255),
  CHSV(rainbow[9], 255, 255),
  CHSV(rainbow[10], 255, 255)
};

uint8_t refresh_happen = 0;
uint8_t edit_happen = 0;

uint8_t mesh_status = 0;

void incomingDataCallback(uint32_t from, String &msg) {

	const char* base64Encoded = msg.c_str();

	size_t base64EncodedLength = strlen(base64Encoded);

	size_t decodedLength = 0;
	mbedtls_base64_decode(NULL, 0, &decodedLength, (const uint8_t*)base64Encoded, base64EncodedLength);

	uint8_t decodedArray[decodedLength];

	size_t outputLength = 0;
	if (mbedtls_base64_decode(decodedArray, decodedLength, &outputLength, (const uint8_t*)base64Encoded, base64EncodedLength) == 0) {
		for (size_t i = 0; i < NUM_HUES; i++) {
			hue_consensus[i] = hue_consensus[i] * (HUE_WEIGHT) + decodedArray[i] * (1-HUE_WEIGHT);
		}
	} else {
		Serial.println("Base64 decoding failed.");
	}

	refresh_happen = 0;

}

void newConnectionCallback(uint32_t nodeId) {}

void changedConnectionCallback() {}

void nodeTimeAdjustedCallback(int32_t offset) {}

void gameLoop() {

	int total_sum = 0;
	for (int i = 0; i < NUM_HUES; i++) {
		total_sum += hue_consensus[i];
	}

	int ledIndex = 0;
	for (int i = 0; i < NUM_HUES; i++) {
		int num_leds_for_color = (hue_consensus[i]*11.0f)/total_sum;

		// Fill the bar with the corresponding number of LEDs for this color
		for (int j = 0; j < num_leds_for_color && ledIndex < 11; j++) {
			bar[ledIndex] = CHSV(rainbow[i], 255, 255);
			ledIndex++;
		}
	}
	// Fill any remaining LEDs (if rounding left gaps)
	while (ledIndex < 11) {
		bar[ledIndex] = CHSV(rainbow[10], 255, 255); // Fill with the last color
		ledIndex++;
	}
	
	//Copy and sort hues
	//memcpy(hues_sorted, hues, sizeof(hues));
	//std::sort(hues_sorted, hues_sorted + NUM_HUES);

	// Calculate the length needed for Base64 encoding
	size_t size = sizeof(hue_consensus);
	size_t base64EncodedLength = 0;
	mbedtls_base64_encode(NULL, 0, &base64EncodedLength, hue_consensus, size);

	// Create a buffer for the Base64 encoded string
	uint8_t base64Encoded[base64EncodedLength + 1];  // +1 for null terminator

	// Perform the Base64 encoding
	size_t outputLength = 0;
	mbedtls_base64_encode(base64Encoded, base64EncodedLength, &outputLength, hue_consensus, size);

	// Null-terminate the encoded string
	base64Encoded[outputLength] = '\0';

	// Send the Base64 encoded string as a broadcast
	mesh.sendBroadcast((const char*)base64Encoded);

}
Task taskGameloop( TASK_SECOND * 1.5 , TASK_FOREVER, &gameLoop );

const char* GameHuemesh::getName() {
	return "GameHuemesh";
}

bool GameHuemesh::shouldBeRemembered() {
	return true;
}

void GameHuemesh::entry() {
	this->tick = 0;

	EFLed.clear();

	for (int i = 0; i < NUM_HUES; i++) {
		hue_consensus[i] = 1;
	}

	//We don't need all the power. We are eco friendly! <~<;
	//setCpuFrequencyMhz(10);

	//Setup meshing
	mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);

	// The transmit power can be set from 8 (lowest power ~7dBm) to 84 (highest power 20dBm) (value is in units of 0.25 dBm)
	//String ssid, String password, uint16_t port = 5555, WiFiMode_t connectMode = WIFI_AP_STA, uint8_t channel = 1, uint8_t hidden = 0, uint8_t maxconn = 4
	mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 1, 0, 6);
	mesh.onReceive(&incomingDataCallback);
	mesh.onNewConnection(&newConnectionCallback);
	mesh.onChangedConnections(&changedConnectionCallback);
	mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

	//Add tasks to scheduler
	userScheduler.addTask(taskGameloop);

	//Start tasks
	taskGameloop.enable();

}

void GameHuemesh::exit() {
	EFLed.clear();
}

void GameHuemesh::run() {
	mesh.update();

	std::vector<CRGB> dragon = {
	  CHSV(rainbow[own_hue], 255, 255),
	  CHSV(rainbow[own_hue], 255, 169),
	  CHSV(rainbow[own_hue], 255, 124),
	  CHSV(rainbow[own_hue], 255, 100),
	  CRGB::Black,
	  CRGB::Black
	};
	
	if(edit_happen < 12){
		std::rotate(dragon.begin(), dragon.begin() + this->tick % EFLED_DRAGON_NUM, dragon.end());
		edit_happen++;
	} else {
		dragon = {
			CHSV(rainbow[own_hue], 255, 255),
			CHSV(rainbow[own_hue], 255, 100),
			CHSV(rainbow[own_hue], 255, 255),
			CRGB::Black,
			CRGB::Black,
			CRGB::Black
		};
	}
	
	/*
	if(refresh_happen < 11){
		std::rotate(bar.begin(), bar.begin() + this->tick % 11, bar.end());
		refresh_happen++;
	}
	*/

	dragon.insert(dragon.end(), bar.begin(), bar.end());
	EFLed.setAll(dragon.data());

	this->tick++;
}


std::unique_ptr<FSMState> GameHuemesh::touchEventFingerprintShortpress() {
	if (this->isLocked()) {
		return nullptr;
	}
	return nullptr;
}

std::unique_ptr<FSMState> GameHuemesh::touchEventFingerprintLongpress() {
	return std::make_unique<MenuMain>();
}

std::unique_ptr<FSMState> GameHuemesh::touchEventFingerprintRelease() {
	if (this->isLocked()) {
		return nullptr;
	}

	own_hue = (this->globals->huemeshOwnHue + 1) % NUM_HUES;
	this->globals->huemeshOwnHue = own_hue;
	this->is_globals_dirty = true;
	this->tick = 0;

	return nullptr;
}

std::unique_ptr<FSMState> GameHuemesh::touchEventNoseShortpress() {
	return nullptr;
}

std::unique_ptr<FSMState> GameHuemesh::touchEventNoseLongpress() {
	return nullptr;
}

std::unique_ptr<FSMState> GameHuemesh::touchEventNoseRelease() {
	//Subtract 1 from all others
	for(int i=0; i < NUM_HUES; i++){
		if(hue_consensus[i] > 0 && i != own_hue) hue_consensus[i]--;
	}
	//Increment own color
	if(hue_consensus[own_hue] < 32){
		hue_consensus[own_hue]++;
	}
	
	edit_happen = 0;
	return nullptr;
}


std::unique_ptr<FSMState> GameHuemesh::touchEventAllLongpress() {
	this->toggleLock();
	return nullptr;
}
