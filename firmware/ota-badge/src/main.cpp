#include <Arduino.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <WiFi.h>

#include "constants.h"
#include "secrets.h"

#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_BUS HSPI

CRGB leds[LED_TOTAL_NUM];

uint8_t ledFlipper = 0;

void resetLeds() {
    fill_solid(leds, LED_TOTAL_NUM, CRGB::Black);
    FastLED.show();
}

void setLed(uint8_t idx, CRGB color) {
    leds[idx] = color;
    FastLED.show();
}

void setupLeds() {
    USBSerial.println("Setting up FastLED.");
    FastLED.addLeds<WS2812B, PIN_LED_DATA, GRB>(leds, LED_TOTAL_NUM);
    FastLED.setBrightness(30);

    // Enable LED
    USBSerial.println("Enabling 5V outputs for LEDs.");
    pinMode(PIN_ENABLE_5VBOOST, OUTPUT);
    digitalWrite(PIN_ENABLE_5VBOOST, HIGH);
    delay(100);

    resetLeds();
}

void setupCpu() {
    USBSerial.print("Initial CPU frequency: ");
    USBSerial.println(getCpuFrequencyMhz());

    setCpuFrequencyMhz(80);
    USBSerial.print("Set CPU frequency to: ");
    USBSerial.println(getCpuFrequencyMhz());
}

void connectToWifi(const char* ssid, const char* password) {
    USBSerial.print("Connecting to WiFi network: ");
    USBSerial.print(ssid);
    USBSerial.print(" ");
    setLed(LED_WIFI_STATUS_IDX, CRGB::Red);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        USBSerial.print(".");
        delay(200);
    }
    USBSerial.println(" Connected!");

    USBSerial.println();
    USBSerial.print("IP address: ");
    USBSerial.println(WiFi.localIP());
    USBSerial.print("MAC address: ");
    USBSerial.println(WiFi.macAddress());
    USBSerial.println();

    setLed(LED_WIFI_STATUS_IDX, CRGB::Green);
}

void setupOTA(const char* password) {
    setLed(LED_OTA_STATUS_IDX, CRGB::Yellow);
    USBSerial.println("Initializing OTA ... ");

    if (password) {
        ArduinoOTA.setPassword(password);
    }

    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH) {
                type = "flash";
            } else {  // U_SPIFFS
                type = "filesystem";
            }

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            USBSerial.println("Start updating " + type);
            setLed(LED_OTA_STATUS_IDX, CRGB::Blue);
        })
        .onEnd([]() {
            USBSerial.println("\nEnd");
            for (uint8_t i = 0; i < 3; i++) {
                setLed(LED_OTA_STATUS_IDX, CRGB::Green);
                delay(500);
                setLed(LED_OTA_STATUS_IDX, CRGB::Black);
                delay(500);
            }
            resetLeds();
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            USBSerial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            USBSerial.printf("Error[%u]: ", error);
            setLed(LED_OTA_STATUS_IDX, CRGB::Red);
            if (error == OTA_AUTH_ERROR) {
                USBSerial.println("Auth Failed");
                setLed(LED_OTA_STATUS_IDX, CRGB::Purple);
            } else if (error == OTA_BEGIN_ERROR) {
                USBSerial.println("Begin Failed");
            } else if (error == OTA_CONNECT_ERROR) {
                USBSerial.println("Connect Failed");
            } else if (error == OTA_RECEIVE_ERROR) {
                USBSerial.println("Receive Failed");
            } else if (error == OTA_END_ERROR) {
                USBSerial.println("End Failed");
            }
        });

    ArduinoOTA.begin();

    USBSerial.println("Done.");
    setLed(LED_OTA_STATUS_IDX, CRGB::Green);
}

void setup() {
    delay(3000);
    USBSerial.begin(9600);
    delay(3000);
    USBSerial.println("Booting ...");

    setupCpu();
    setupLeds();
    connectToWifi(WIFI_SSID, WIFI_PASSWORD);
    setupOTA(OTA_SECRET);
}

void loop() {
    ArduinoOTA.handle();

    USBSerial.print(".");
    setLed(LED_BREATHE_IDX, ledFlipper++ < 127 ? CRGB::Green : CRGB::Black);
    delay(10);
}
