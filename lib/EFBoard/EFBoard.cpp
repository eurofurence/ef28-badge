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
 * @author Honigeintopf
 */

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

#include <EFLed.h>
#include <EFLogging.h>

#include "EFBoard.h"


RTC_DATA_ATTR uint32_t bootCount = 0;

volatile int8_t ota_last_progress = -1;

EFBoardClass::EFBoardClass()
    : power_state(EFBoardPowerState::UNKNOWN) {
    bootCount++;
}

void EFBoardClass::setup() {
    // Setup serial
    // If flashing often fails, you can add a safety-backoff delay before running serial which helps with flashing
    //delay(2000);
    EFBOARD_SERIAL_DEVICE.begin(EFBOARD_SERIAL_BAUD);
    delay(50);

    LOG("\r\n");
    this->printCredits();
    LOG("\r\n");

    // Board initialization process
    LOG_INFO("(EFBoard) Initializing badge ...");
    LOGF_INFO("(EFBoard) Boot #%d - %s\r\n", this->getWakeupCount(), this->getWakeupReason());
    LOGF_INFO("(EFBoard) Firmware version: %s (compiled: %s @ %s)\r\n", EFBOARD_FIRMWARE_VERSION, __DATE__, __TIME__);

    // CPU frequency
    LOGF_DEBUG("(EFBoard) Initial CPU frequency: %d\r\n", getCpuFrequencyMhz());
    setCpuFrequencyMhz(80);
    LOGF_INFO("(EFBoard) Set CPU frequency to: %d\r\n", getCpuFrequencyMhz());

    // Initialize ADC for V_BAT measuring
    analogReadResolution(12);
    LOG_DEBUG("(EFBoard) Set ADC read resolution to: 12 bit");
    pinMode(EFBOARD_PIN_VBAT, INPUT);
    LOG_INFO("(EFBoard) Initialized battery sense ADC")

    // Seed rnd
    randomSeed(analogRead(0));

    // Check power state
    const EFBoardPowerState pwrstate = this->getPowerState();
    if (pwrstate == EFBoardPowerState::BAT_BROWN_OUT_HARD) {
        LOGF_ERROR("(EFBoard) HARD BROWN OUT DETECTED (V_BAT = %.2f V). Panic!\r\n", this->getBatteryVoltage());
        while (1) {
            // TODO: use actual hard brownout code here
            // sleep most of the time.
            esp_sleep_enable_timer_wakeup(5 * 1000000);
            esp_light_sleep_start();
        }
    } else if (pwrstate == EFBoardPowerState::BAT_BROWN_OUT_SOFT) {
        LOGF_WARNING("(EFBoard) Soft brown out detected (V_BAT = %.2f V)\r\n", this->getBatteryVoltage());
    }

    if (this->isBatteryPowered()) {
        LOG_INFO("(EFBoard) Running from battery");
        LOGF_INFO("(EFBoard)   -> Battery voltage: %.2fV\r\n", this->getBatteryVoltage());
    } else {
        LOG_INFO("(EFBoard) Running from USB power");
    }

    // Wifi modem stuff
    this->disableWifi();
    this->disableOTA();

    LOG_INFO("(EFBoard) Initialization complete")
}

unsigned int EFBoardClass::getWakeupCount() {
    return bootCount;
}

const char *EFBoardClass::getWakeupReason() {
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            return "Wakeup caused by external signal using RTC_IO";
        case ESP_SLEEP_WAKEUP_EXT1:
            return "Wakeup caused by external signal using RTC_CNTL";
        case ESP_SLEEP_WAKEUP_TIMER:
            return "Wakeup caused by timer";
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            return "Wakeup caused by touchpad";
        case ESP_SLEEP_WAKEUP_ULP:
            return "Wakeup caused by ULP program";
        case ESP_SLEEP_WAKEUP_GPIO:
            return "Wakeup caused by GPIO";
        case ESP_SLEEP_WAKEUP_UART:
            return "Wakeup caused by UART";
        case ESP_SLEEP_WAKEUP_WIFI:
            return "Wakeup casued by WIFI";
        case ESP_SLEEP_WAKEUP_COCPU:
            return "Wakeup caused by coprocessor interrupt";
        case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:
            return "Wakeup caused by coprocessor CRASH";
        case ESP_SLEEP_WAKEUP_BT:
            return "Wakeup caused by Bluetooth";
        default:
            return "Wakeup was not caused by deep sleep (regular boot)";
    }
}

const float EFBoardClass::getBatteryVoltage() {
    // R11 = 51.1k, R12 = 100k
    constexpr float factor = 1 / (100 / (51.1 + 100.0));
    // According to the datasheet, this should be more like 3.3. But this is off across all boards -> 3.41 seems more accurate!
    constexpr float voltage_range = 3.41f;
    uint16_t rawValue = analogRead(EFBOARD_PIN_VBAT);
    float millivolts = rawValue * (voltage_range  / 4095.0f) * factor;
    LOGF_DEBUG("Battery voltage: %f\r\n", millivolts);
    return millivolts / 1000.0f;
}

const bool EFBoardClass::isBatteryPowered() {
    return this->getBatteryVoltage() > EFBOARD_VBAT_MIN - 0.5;
}

const uint8_t EFBoardClass::getBatteryCapacityPercent() {
    /* Used Varta Longlife AA cells discharge curve as a base.
     *
     * We consider 1.16 V as empty even though the cells are not empty at that
     * point due to the brown out voltage of the DC/DC for the 3.3V rail.
     *
     * Data points used to fit cubic function to:
     *   - 1.60 V = 100 %
     *   - 1.50 V =  94 %
     *   - 1.40 V =  87 %
     *   - 1.30 V =  66 %
     *   - 1.20 V =  23 %
     *   - 1.16 V =   0 %
     */
    // TODO: This is a bit outdated/off
    double vBatCell = this->getBatteryVoltage() / 3.0;
    double percent = (14.9679 * pow(vBatCell, 3) - 68.9823 * pow(vBatCell, 2) + 106.4289 * vBatCell - 54.0063) * 100.0;
    return max(min((int) round(percent), 100), 0);
}

const EFBoardPowerState EFBoardClass::updatePowerState() {
    if (!this->isBatteryPowered()) {
        this->power_state = EFBoardPowerState::USB;
    } else {
        float vbat = this->getBatteryVoltage();

        // if (vbat <= EFBOARD_BROWN_OUT_SOFT) {
        //     // A low-battery state could be a momentary spike that we can handle and do not need to shut off
        //     // We take the highest of three measurements to make sure
        //     delay(10);
        //     float newVbat = this->getBatteryVoltage();
        //     vbat = newVbat > vbat ? newVbat : vbat;
        //     delay(10);
        //     newVbat = this->getBatteryVoltage();
        //     vbat = newVbat > vbat ? newVbat : vbat;
        //     delay(10);
        //     newVbat = this->getBatteryVoltage();
        //     vbat = newVbat > vbat ? newVbat : vbat;
        // }

        if (vbat <= EFBOARD_BROWN_OUT_HARD || this->power_state == EFBoardPowerState::BAT_BROWN_OUT_HARD) {
            this->power_state = EFBoardPowerState::BAT_BROWN_OUT_HARD;
        } else if (vbat <= EFBOARD_BROWN_OUT_SOFT || this->power_state == EFBoardPowerState::BAT_BROWN_OUT_SOFT) {
            this->power_state = EFBoardPowerState::BAT_BROWN_OUT_SOFT;
        } else {
            this->power_state = EFBoardPowerState::BAT_NORMAL;
        }
    }

    return this->power_state;
}

const EFBoardPowerState EFBoardClass::getPowerState() {
    return this->power_state;
}

const EFBoardPowerState EFBoardClass::resetPowerState() {
    this->power_state = EFBoardPowerState::UNKNOWN;
    return this->updatePowerState();
}

bool EFBoardClass::connectToWifi(const char *ssid, const char *password) {
    LOGF_INFO("(EFBoard) Connecting to WiFi network: %s ", ssid);

    // Try to connect to WiFi
    WiFi.begin(ssid, password);
    WiFi.setSleep(true);
    for (int32_t timeout_ms = 10000; timeout_ms >= 0; timeout_ms -= 200) {
        LOGF(".", 0);
        delay(200);

        // Check if connection failed
        if (WiFi.status() == WL_CONNECT_FAILED) {
            LOG(" FAILED!");
            return false;
        }

        // Check for successful connection
        if (WiFi.status() == WL_CONNECTED) {
            LOG(" Connected!")
            break;
        }
    }

    // Catch timeout
    if (WiFi.status() != WL_CONNECTED) {
        LOG(" Timeout.");
        return false;
    }

    // Log assigned MAC address and assigned IP address
    LOGF_INFO("(EFBoard)   -> IP address: %s\r\n", WiFi.localIP().toString().c_str());
    LOGF_INFO("(EFBoard)   -> MAC address: %s\r\n", WiFi.macAddress().c_str());
    return true;
}

bool EFBoardClass::disableWifi() {
    WiFi.disconnect(true, true);
    if (!WiFi.enableSTA(false)) {
        LOG_ERROR("(EFBoard) Failed to disable WiFi");
        return false;
    }

    LOG_INFO("(EFBoard) Disabled WiFi");
    return true;
}

void EFBoardClass::enableOTA(const char *password) {
    LOG_INFO("(EFBoard) Initializing OTA ... ");

    if (password) {
        ArduinoOTA.setPassword(password);
        LOG_INFO("(EFBoard)   -> Enabling password protection");
    } else {
        LOG_WARNING("(EFBoard)   -> Using NO PASSWORD PROTECTION!");
    }

    ArduinoOTA
            .onStart([]() {
                if (ArduinoOTA.getCommand() == U_FLASH) {
                    LOG_INFO("(OTA) Start OTA update of U_FLASH ...");
                } else {
                    LOG_INFO("(OTA) Starting OTA update of U_SPIFFS ...");
                }

                // Reset progress
                ota_last_progress = -1;

                // Setup LEDs
                EFLed.clear();
                EFLed.setBrightnessPercent(50);
                EFLed.setDragonEye(CRGB::Blue);
            })
            .onEnd([]() {
                LOG_INFO("(OTA) Finished! Rebooting ...");
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
                if (ota_last_progress < progresspercent) {
                    ota_last_progress = progresspercent;
                    EFLed.fillEFBarProportionally(progresspercent, CRGB::Red, CRGB::Black);
                    LOGF_INFO("(OTA) Progress: %u%%\r\n", progresspercent);
                }
            })
            .onError([](ota_error_t error) {
                LOGF_ERROR("(OTA) Error[%u]: ", error);
                EFLed.setDragonNose(CRGB::Red);
                if (error == OTA_AUTH_ERROR) {
                    LOG_WARNING("(OTA) Auth Failed");
                    EFLed.setDragonNose(CRGB::Purple);
                } else if (error == OTA_BEGIN_ERROR) {
                    EFLed.setDragonNose(CRGB::Green);
                    LOG_ERROR("(OTA) Begin Failed");
                } else if (error == OTA_CONNECT_ERROR) {
                    EFLed.setDragonNose(CRGB::Purple);
                    LOG_ERROR("(OTA) Connect Failed");
                } else if (error == OTA_RECEIVE_ERROR) {
                    EFLed.setDragonNose(CRGB::Blue);
                    LOG_ERROR("(OTA) Receive Failed");
                } else if (error == OTA_END_ERROR) {
                    EFLed.setDragonNose(CRGB::Yellow);
                    LOG_ERROR("(OTA) End Failed");
                }
            });

    ArduinoOTA.begin();

    LOG_INFO("(EFBoard)   -> Setup OTA listeners");
    LOG_INFO("(EFBoard) Enabled OTA");
}

void EFBoardClass::disableOTA() {
    ArduinoOTA.end();
    LOG_INFO("(EFBoard) Disabled OTA");
}

void EFBoardClass::printCredits() {
    LOG("####################################################################");
    LOG("#                                                                  #");
    LOG("#                 ▄▀▀▄▀▀▀▄  ▄▀▀█▄   ▄▀▀▀█▀▀▄  ▄▀▀▀▀▄               #");
    LOG("#                █   █   █ ▐ ▄▀ ▀▄ █    █  ▐ █ █   ▐               #");
    LOG("#                ▐  █▀▀█▀    █▄▄▄█ ▐   █        ▀▄                 #");
    LOG("#                 ▄▀    █   ▄▀   █    █      ▀▄   █                #");
    LOG("#                █     █ ▄ █   ▄▀ ▄ ▄▀ ▄      █▀▀▀ ▄               #");
    LOG("#                ▐     ▐   ▐   ▐   █          ▐                    #");
    LOG("#                                  ▐                               #");
    LOG("#                                                                  #");
    LOG("#                          Property of the                         #");
    LOG("#                Radical Access Technology Syndicate               #");
    LOG("#                                                                  #");
    LOG("# ---------------------------------------------------------------- #");
    LOG("#                                                                  #");
    LOG("# In your paws, you hold the Eurofurence 28 Cyberpunk badge, a     #");
    LOG("# tangible nexus of your extraordinary support for the furry       #");
    LOG("# community. This badge is not just an accessory; it’s a beacon of #");
    LOG("# your commitment, dedication and generosity that helps us propel  #");
    LOG("# everyone into a neon-lit realm where creativity and community    #");
    LOG("# thrives admist the cybernetic backdrop.                          #");
    LOG("#                                                                  #");
    LOG("# It was crafted with love and dedication, involving many cycles   #");
    LOG("# of coding, soldering and circuit-bending. This is our way of     #");
    LOG("# saying thank you, a cyber-etched tribute to your unwavering      #");
    LOG("# support <3                                                       #");
    LOG("#                                                                  #");
    LOG("# ---------------------------------------------------------------- #");
    LOG("#                                                                  #");
    LOG("# This terminal was crafted by:                                    #");
    LOG("#                                                                  #");
    LOG("#                                                                  #");
    LOG("# ░█▀▄░█▀█░█▀▄░█░█░█▀▄░█▀█░▀█▀                                     #");
    LOG("# ░█░█░█▀█░█▀▄░█▀▄░█▀▄░█▀█░░█░                                     #");
    LOG("# ░▀▀░░▀░▀░▀░▀░▀░▀░▀░▀░▀░▀░░▀░                                     #");
    LOG("#                                                                  #");
    LOG("# ░█░█░█▀█░█▀█░▀█▀░█▀▀░█▀▀░▀█▀░█▀█░▀█▀░█▀█░█▀█░█▀▀                 #");
    LOG("# ░█▀█░█░█░█░█░░█░░█░█░█▀▀░░█░░█░█░░█░░█░█░█▀▀░█▀▀                 #");
    LOG("# ░▀░▀░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀░▀▀▀░▀░▀░░▀░░▀▀▀░▀░░░▀░░                 #");
    LOG("#                                                                  #");
    LOG("# ░▀█▀░█▀▄░█▀█░█░█                                                 #");
    LOG("# ░░█░░█▀▄░█▀█░█▀█                                                 #");
    LOG("# ░▀▀▀░▀░▀░▀░▀░▀░▀                                                 #");
    LOG("#                                                                  #");
    LOG("# ░▀█▀░█▀█░█░█░█▀▀░█▀█                                             #");
    LOG("# ░░█░░█░█░█▀▄░█▀▀░█░█                                             #");
    LOG("# ░░▀░░▀▀▀░▀░▀░▀▀▀░▀░▀                                             #");
    LOG("#                                                                  #");
    LOG("# ---------------------------------------------------------------- #");
    LOG("#                                                                  #");
    LOG("# Visit https://eurofurence.org/EF28/badge for end of transmission #");
    LOG("#                                                                  #");
    LOG("####################################################################");
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EFBOARD)
EFBoardClass EFBoard;
#endif
