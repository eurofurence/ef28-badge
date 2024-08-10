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

#define EFBOARD_SERIAL_INIT_DELAY_MS 3000  //!< Milliseconds to wait before initializing the serial device (prevents flashing if USBSerial is used!)
#define EFBOARD_SERIAL_DEVICE USBSerial    //!< Serial device to use for logging
#define EFBOARD_SERIAL_BAUD 115200         //!< Baudrate for the serial device


/**
 * @brief Basic related to the EF badge board
 */
class EFBoardClass {

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
        

};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EFBOARD)
extern EFBoardClass EFBoard;
#endif

#endif /* EFBOARD_H_ */

