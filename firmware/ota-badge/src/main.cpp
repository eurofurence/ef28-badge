#include <Arduino.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <WiFi.h>

#include <EFLogging.h>
#include <EFLed.h>
#include <EFPrideFlags.h>
#include <EFTouch.h>

#include "constants.h"
#include "secrets.h"

#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_BUS HSPI

RTC_DATA_ATTR int bootCount = 0;

uint8_t ledFlipper = 0;

volatile uint8_t boopColorIdx = 0;


void setupCpu() {
    LOG_INFO(("Initial CPU frequency: " + String(getCpuFrequencyMhz())).c_str());
    setCpuFrequencyMhz(80);
    LOG_INFO(("Set CPU frequency to: " + String(getCpuFrequencyMhz())).c_str());
}

void connectToWifi(const char* ssid, const char* password) {
    LOG_INFO(("Connecting to WiFi network: " + String(ssid)).c_str());
    EFLed.setDragonNose(CRGB::Red);

    WiFi.begin(ssid, password);
    WiFi.setSleep(true);
    while (WiFi.status() != WL_CONNECTED) {
        LOG_INFO(".");
        delay(200);
    }
    LOG_INFO("Connected!");

    LOG_INFO(("IP address: " + WiFi.localIP().toString()).c_str());
    LOG_INFO(("MAC address: " + WiFi.macAddress()).c_str());

    EFLed.setDragonNose(CRGB::Green);
}

void setupOTA(const char* password) {
    EFLed.setDragonMuzzle(CRGB::Yellow);
    LOG_INFO("Initializing OTA ... ");

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
            LOG_INFO("Start updating " + type);

            // Setup LEDs
            EFLed.clear();
            EFLed.setBrightness(50);
            EFLed.setDragonEye(CRGB::Blue);
        })
        .onEnd([]() {
            LOG_INFO("\nEnd");
            for (uint8_t i = 0; i < 3; i++) {
                EFLed.setDragonEye(CRGB::Green);
                delay(500);
                EFLed.setDragonEye(CRGB::Black);
                delay(500);
            }
            EFLed.clear();
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            uint8_t progresspercent = (progress / (total / 100));
            if (progresspercent % 5 == 0) {
                EFLed.fillEFBarProportionally(progresspercent, CRGB::Red, CRGB::Black);
            }
            LOGF_INFO("Progress: %u%%\r\n", progresspercent);
        })
        .onError([](ota_error_t error) {
            LOGF_ERROR("Error[%u]: ", error);
            EFLed.setDragonNose(CRGB::Red);
            if (error == OTA_AUTH_ERROR) {
                LOG_WARNING("Auth Failed");
                EFLed.setDragonNose(CRGB::Purple);
            } else if (error == OTA_BEGIN_ERROR) {
                EFLed.setDragonNose(CRGB::Green);
                LOG_ERROR("Begin Failed");
            } else if (error == OTA_CONNECT_ERROR) {
                EFLed.setDragonNose(CRGB::Purple);
                LOG_ERROR("Connect Failed");
            } else if (error == OTA_RECEIVE_ERROR) {
                EFLed.setDragonNose(CRGB::Blue);
                LOG_ERROR("Receive Failed");
            } else if (error == OTA_END_ERROR) {
                EFLed.setDragonNose(CRGB::Yellow);
                LOG_ERROR("End Failed");
            }
        });

    ArduinoOTA.begin();

    LOG_INFO("Done.");
    EFLed.setDragonMuzzle(CRGB::Green);
}

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : LOG_DEBUG("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : LOG_DEBUG("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : LOG_DEBUG("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : LOG_DEBUG("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : LOG_DEBUG("Wakeup caused by ULP program"); break;
    default: LOGF_DEBUG("Wakeup was not caused by deep sleep: %d\r\n", wakeup_reason); break;
  }
}

void isr_noseboop() {
    boopColorIdx = (boopColorIdx + 1) % 8;
}

volatile bool logtouched = false;
void ARDUINO_ISR_ATTR isr_touched() {logtouched = true;}

volatile bool loguntouched = false;
void ARDUINO_ISR_ATTR isr_untouched() {loguntouched=true;}

volatile bool logshortpress = false;
void ARDUINO_ISR_ATTR isr_shortpressed() {logshortpress=true; }

volatile bool loglongpress = false;
void ARDUINO_ISR_ATTR isr_longpressed() {loglongpress=true; }

void setup() {
    delay(3000);
    USBSerial.begin(9600);
    delay(3000);
    LOG_INFO("Booting ...");
    LOGF_DEBUG("Boot count: %d\r\n", bootCount++);
    print_wakeup_reason();

    setupCpu();
    EFLed.init(20);
    EFLed.clear();
    connectToWifi(WIFI_SSID, WIFI_PASSWORD);
    setupOTA(OTA_SECRET);

    // Touchy stuffy
    EFTouch.init();

    // Just to make sure that detatching works properly
    // EFTouch.attachInterruptOnTouch(EFTouchZone::Nose, isr_touched);
    // EFTouch.attachInterruptOnRelease(EFTouchZone::Nose, isr_untouched);
    // EFTouch.attachInterruptOnShortpress(EFTouchZone::Nose, isr_shortpressed);
    // EFTouch.attachInterruptOnLongpress(EFTouchZone::Nose, isr_longpressed);
    // EFTouch.detatchInterruptOnTouch(EFTouchZone::Nose);
    // EFTouch.detatchInterruptOnRelease(EFTouchZone::Nose);
    // EFTouch.detatchInterruptOnShortpress(EFTouchZone::Nose);
    // EFTouch.detatchInterruptOnLongpress(EFTouchZone::Nose);

    EFTouch.attachInterruptOnTouch(EFTouchZone::Fingerprint, isr_touched);
    EFTouch.attachInterruptOnRelease(EFTouchZone::Fingerprint, isr_untouched);
    EFTouch.attachInterruptOnShortpress(EFTouchZone::Fingerprint, isr_shortpressed);
    EFTouch.attachInterruptOnLongpress(EFTouchZone::Fingerprint, isr_longpressed);
}

bool blinkled_state = false;
uint8_t flagidx = 0;
uint32_t brightness = 0;

unsigned long task_blinkled = 0;
unsigned long task_flagswitch = 0;
unsigned long task_touchleds = 0;
unsigned long task_brightness = 0;

void loop() {
    // Handler: OTA
    ArduinoOTA.handle();

    // Task: Blink LED
    if (task_blinkled < millis()) {
        EFLed.setDragonEye(blinkled_state ? CRGB::Green : CRGB::Black);
        blinkled_state = !blinkled_state;

        task_blinkled = millis() + 1000;
    }

    // Task: Switch pride flag
    if (task_flagswitch < millis()) {
        LOG_DEBUG(".");

        switch (flagidx) {
            case 0: EFLed.setEFBar(EFPrideFlags::LGBT); break;
            case 1: EFLed.setEFBar(EFPrideFlags::LGBTQI); break;
            case 2: EFLed.setEFBar(EFPrideFlags::Bisexual); break;
            case 3: EFLed.setEFBar(EFPrideFlags::Polyamorous); break;
            case 4: EFLed.setEFBar(EFPrideFlags::Polysexual); break;
            case 5: EFLed.setEFBar(EFPrideFlags::Transgender); break;
            case 6: EFLed.setEFBar(EFPrideFlags::Pansexual); break;
            case 7: EFLed.setEFBar(EFPrideFlags::Asexual); break;
            case 8: EFLed.setEFBar(EFPrideFlags::Genderfluid); break;
            case 9: EFLed.setEFBar(EFPrideFlags::Genderqueer); break;
            case 10: EFLed.setEFBar(EFPrideFlags::Nonbinary); break;
            case 11: EFLed.setEFBar(EFPrideFlags::Intersex); break;
        }
        flagidx = (flagidx + 1) % 12;

        task_flagswitch = millis() + 3000;
    }

    // Task: Touch LEDs
    if (task_touchleds < millis()) {
        uint8_t touchy = EFTouch.readFingerprint();
        if (touchy) {
            EFLed.fillEFBarProportionally(touchy*10, CRGB::Purple, CRGB::Black);
        }

        task_touchleds = millis() + 10;
    }

    // Task: Brightness
    if (task_brightness < millis()) {
        uint8_t newbrightness = (brightness % 202) < 101 ? brightness % 101 : 100 - (brightness % 101);
        EFLed.setBrightness(newbrightness);
        brightness++;

        task_brightness = millis() + 10;
    }

    // Handler: ISR debug logging
    if (logtouched) {
        logtouched = false;
        LOG_DEBUG("v TOUCHED!");
    }
    if (logshortpress) {
        logshortpress = false;
        LOG_DEBUG(" - shortpress");
    }
    if (loglongpress) {
        loglongpress = false;
        LOG_DEBUG(" ----- longpress");
    }
    if (loguntouched) {
        loguntouched = false;
        LOG_DEBUG("^ released.");
    }

    // if (deepsleepcounter == 0) {
    //     USBSerial.println("Putting the ESP into deep sleep for 3 seconds ...");
    //     esp_sleep_enable_timer_wakeup(3000000);
    // 
    //     USBSerial.flush();
    //     esp_deep_sleep_start();
    // }
}
