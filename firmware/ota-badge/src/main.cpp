#include <Arduino.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <WiFi.h>

#include <EFLed.h>
#include <EFPrideFlags.h>
#include <EFTouch.h>

#include "constants.h"
#include "secrets.h"

#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_BUS HSPI

RTC_DATA_ATTR int bootCount = 0;


EFLed leds;
EFTouch touch;

uint8_t ledFlipper = 0;

volatile uint8_t boopColorIdx = 0;


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
    leds.setDragonNose(CRGB::Red);

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

    leds.setDragonNose(CRGB::Green);
}

void setupOTA(const char* password) {
    leds.setDragonMuzzle(CRGB::Yellow);
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
            leds.setDragonMuzzle(CRGB::Blue);
        })
        .onEnd([]() {
            USBSerial.println("\nEnd");
            for (uint8_t i = 0; i < 3; i++) {
                leds.setDragonMuzzle(CRGB::Green);
                delay(500);
                leds.setDragonMuzzle(CRGB::Black);
                delay(500);
            }
            leds.clear();
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            USBSerial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            USBSerial.printf("Error[%u]: ", error);
            leds.setDragonMuzzle(CRGB::Red);
            if (error == OTA_AUTH_ERROR) {
                USBSerial.println("Auth Failed");
                leds.setDragonMuzzle(CRGB::Purple);
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
    leds.setDragonMuzzle(CRGB::Green);
}

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : USBSerial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : USBSerial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : USBSerial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : USBSerial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : USBSerial.println("Wakeup caused by ULP program"); break;
    default: USBSerial.printf("Wakeup was not caused by deep sleep: %d\r\n", wakeup_reason); break;
  }
}

void isr_noseboop() {
    boopColorIdx = (boopColorIdx + 1) % 8;
}

void setup() {
    delay(3000);
    USBSerial.begin(9600);
    delay(3000);
    USBSerial.print("\r\n\r\n\r\n");
    USBSerial.println("Booting ...");
    USBSerial.printf("Boot count: %d\r\n", bootCount++);
    print_wakeup_reason();

    setupCpu();
    leds = EFLed(32);
    leds.clear();
    connectToWifi(WIFI_SSID, WIFI_PASSWORD);
    setupOTA(OTA_SECRET);

    // Test temporary
    touch = EFTouch();
    USBSerial.println("Calibrating EFTouch ...");
    touch.calibrate();
    USBSerial.print("  -> Fingerprint: ");
    USBSerial.println(touch.getFingerprintNoiseLevel());
    USBSerial.print("  -> Nose: ");
    USBSerial.println(touch.getNoseNoiseLevel());

    touch.attachInterruptNose(isr_noseboop);
}

uint8_t deepsleepcounter = 10;
uint8_t flagidx = 0;
uint32_t brightness = 0;

void loop() {
    // Check for OTA
    ArduinoOTA.handle();

    // Blink a LED
    if (ledFlipper == 0) {
        USBSerial.println(".");
        deepsleepcounter--;

        switch (flagidx) {
            case 0: leds.setEFBar(EFPrideFlags::LGBT); break;
            case 1: leds.setEFBar(EFPrideFlags::LGBTQI); break;
            case 2: leds.setEFBar(EFPrideFlags::Bisexual); break;
            case 3: leds.setEFBar(EFPrideFlags::Polyamorous); break;
            case 4: leds.setEFBar(EFPrideFlags::Polysexual); break;
            case 5: leds.setEFBar(EFPrideFlags::Transgender); break;
            case 6: leds.setEFBar(EFPrideFlags::Pansexual); break;
            case 7: leds.setEFBar(EFPrideFlags::Asexual); break;
            case 8: leds.setEFBar(EFPrideFlags::Genderfluid); break;
            case 9: leds.setEFBar(EFPrideFlags::Genderqueer); break;
            case 10: leds.setEFBar(EFPrideFlags::Nonbinary); break;
            case 11: leds.setEFBar(EFPrideFlags::Intersex); break;
        }
        flagidx = (flagidx + 1) % 12;
    }
    leds.setDragonEye(ledFlipper++ < 127 ? CRGB::Green : CRGB::Black);

    // Touch LEDs
    uint8_t touchy = touch.readFingerprint();
    if (touchy) {
        for (uint8_t i = 0; i < EFLED_EFBAR_NUM; i++) {
            if (touchy) {
                leds.setEFBar(i , CRGB::Purple);
                touchy--;
            } else {
                leds.setEFBar(i, CRGB::Black);
            }
        }
    }

    switch (boopColorIdx) {
        case 0: leds.setDragonEarTop(CRGB::Red); break;
        case 1: leds.setDragonEarTop(CRGB::Orange); break;
        case 2: leds.setDragonEarTop(CRGB::Yellow); break;
        case 3: leds.setDragonEarTop(CRGB::Green); break;
        case 4: leds.setDragonEarTop(CRGB::Teal); break;
        case 5: leds.setDragonEarTop(CRGB::Blue); break;
        case 6: leds.setDragonEarTop(CRGB::BlueViolet); break;
        case 7: leds.setDragonEarTop(CRGB::Purple); break;
    }

    uint8_t newbrightness = (brightness % 202) < 101 ? brightness % 101 : 100 - (brightness % 101);
    // USBSerial.print("Set brightness = ");
    // USBSerial.print(newbrightness);
    leds.setBrightness(newbrightness);
    // USBSerial.print("     -> read ");
    // USBSerial.println(leds.getBrightness());
    brightness++;

    // Wait for next iteration
    delay(10);

    // if (deepsleepcounter == 0) {
    //     USBSerial.println("Putting the ESP into deep sleep for 3 seconds ...");
    //     esp_sleep_enable_timer_wakeup(3000000);
    // 
    //     USBSerial.flush();
    //     esp_deep_sleep_start();
    // }
}
