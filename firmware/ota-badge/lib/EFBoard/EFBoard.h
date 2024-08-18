#ifndef EFBOARD_H_
#define EFBOARD_H_

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

#include "EFBoardPowerState.h"

#define EFBOARD_FIRMWARE_VERSION "v1.1.0"
#define EFBOARD_SERIAL_INIT_DELAY_MS 3000  //!< Milliseconds to wait before initializing the serial device (prevents flashing if USBSerial is used!)
#define EFBOARD_SERIAL_DEVICE USBSerial    //!< Serial device to use for logging
#define EFBOARD_SERIAL_BAUD 115200         //!< Baudrate for the serial device

#define EFBOARD_PIN_VBAT 10                //!< Pin the analog voltage divider for V_BAT is connected to
#define EFBOARD_NUM_BATTERIES 3            //!< Number of battery cells used for V_BAT
#define EFBOARD_VBAT_MAX (1.60 * EFBOARD_NUM_BATTERIES) //!< Voltage at which battery cells are considered full
#define EFBOARD_VBAT_MIN (1.16 * EFBOARD_NUM_BATTERIES) //!< Voltage at which battery cells are considered empty

#define EFBOARD_BROWN_OUT_SOFT EFBOARD_VBAT_MIN //!< V_BAT threshold after which a soft brown out is triggered
#define EFBOARD_BROWN_OUT_HARD 3.35             //!< V_BAT threshold after which a hard brown out is triggered


/**
 * @brief Basic related to the EF badge board
 */
class EFBoardClass {

    protected:

        EFBoardPowerState power_state;  //!< Power state of the board during the last check 

    public:

        /**
         * @brief Constructs a new EFBoard instance.
         */
        EFBoardClass();
        
        /**
         * @brief Performs basic initialization of the badge
         */
        void setup();

        /**
         * @brief Returns the number of boots / wakeups since power on
         * 
         * @return Wakeup count since power on
         */
        unsigned int getWakeupCount();

        /**
         * @brief Retrieves the cause for the last wakeup in a human readable form.
         * 
         * @return Wakeup reason as string
         */
        const char* getWakeupReason();

        /**
         * @brief Reads the current battery voltage
         * 
         * @return Current battery voltage
         */
        const float getBatteryVoltage();

        /**
         * @brief Determiens if the badge currently has batteries connected to it
         * 
         * @return True if batterys were detected
         */
        const bool isBatteryPowered();

        /**
         * @brief Approximates current battery capacity level in percent
         * 
         * @return Current approx. battery capacity level in percent (0 - 100)
         */
        const uint8_t getBatteryCapacityPercent();

        /**
         * @brief Updates the power state of the board. If a brown out state was
         * reached once, the board power state does not automatically recover
         * from this.
         * 
         * If you wish to reset the brown out condition use resetPowerState().
         * 
         * @return The detected power state
         */
        const EFBoardPowerState updatePowerState();

        /**
         * @brief Retrieves the last known power state
         * 
         * @return Last detected power state
         */
        const EFBoardPowerState getPowerState();

        /**
         * @brief Resets and updates the power state of the board. This method
         * allows to clear previously set brown out states.
         * 
         * @return The detected power state
         */
        const EFBoardPowerState resetPowerState();

        /**
         * @brief Tries to connect to the given Wifi access point
         * 
         * @param ssid SSID of the Wifi network to connect to
         * @param password WPA2 password for the Wifi network
         * @return True, if the connection was successful
         */
        bool connectToWifi(const char* ssid, const char* password);

        /**
         * @brief Disconnects from any wifi network and disables the radio modem
         * 
         * @return True if the radio modem was sucessfully disabled
         */
        bool disableWifi();
        
        /**
         * @brief Enables OTA update receiver
         * 
         * @param password Optional password to protect the OTA API. If empty, no
         * password is required (DANGER!)
         */
        void enableOTA(const char* password);

        /**
         * @brief Disabled OTA update receiver
         */
        void disableOTA();
        
        /**
         * @brief Writes a credit block to the serial console
         */
        void printCredits();
        

};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EFBOARD)
extern EFBoardClass EFBoard;
#endif

#endif /* EFBOARD_H_ */

