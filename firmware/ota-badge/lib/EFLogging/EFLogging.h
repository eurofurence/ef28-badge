#ifndef EFLOGGING_H_
#define EFLOGGING_H_

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
#include <EFBoard.h>

#define LOG_DEV_SERIAL EFBOARD_SERIAL_DEVICE  //!< Serial device to use (Serial or USBSerial)

#define LOG(msg)         { LOG_DEV_SERIAL.println((msg)); }
#define LOG_DEBUG(msg)   { LOG_DEV_SERIAL.printf("%010.4f   [DEBUG] ", millis()/1000.0f); LOG_DEV_SERIAL.println((msg)); }
#define LOG_INFO(msg)    { LOG_DEV_SERIAL.printf("%010.4f    [INFO] ", millis()/1000.0f); LOG_DEV_SERIAL.println((msg)); }
#define LOG_WARNING(msg) { LOG_DEV_SERIAL.printf("%010.4f [WARNING] ", millis()/1000.0f); LOG_DEV_SERIAL.println((msg)); }
#define LOG_ERROR(msg)   { LOG_DEV_SERIAL.printf("%010.4f   [ERROR] ", millis()/1000.0f); LOG_DEV_SERIAL.println((msg)); }
#define LOG_FATAL(msg)   { LOG_DEV_SERIAL.printf("%010.4f   [FATAL] ", millis()/1000.0f); LOG_DEV_SERIAL.println((msg)); }

#define LOGF(msg, format, ...)         { LOG_DEV_SERIAL.printf((msg), (format), ##__VA_ARGS__); }
#define LOGF_DEBUG(msg, format, ...)   { LOG_DEV_SERIAL.printf("%010.4f   [DEBUG] ", millis()/1000.0f); LOG_DEV_SERIAL.printf((msg), (format), ##__VA_ARGS__); }
#define LOGF_INFO(msg, format, ...)    { LOG_DEV_SERIAL.printf("%010.4f    [INFO] ", millis()/1000.0f); LOG_DEV_SERIAL.printf((msg), (format), ##__VA_ARGS__); }
#define LOGF_WARNING(msg, format, ...) { LOG_DEV_SERIAL.printf("%010.4f [WARNING] ", millis()/1000.0f); LOG_DEV_SERIAL.printf((msg), (format), ##__VA_ARGS__); }
#define LOGF_ERROR(msg, format, ...)   { LOG_DEV_SERIAL.printf("%010.4f   [ERROR] ", millis()/1000.0f); LOG_DEV_SERIAL.printf((msg), (format), ##__VA_ARGS__); }
#define LOGF_FATAL(msg, format, ...)   { LOG_DEV_SERIAL.printf("%010.4f   [FATAL] ", millis()/1000.0f); LOG_DEV_SERIAL.printf((msg), (format), ##__VA_ARGS__); }

#endif /* EFLOGGING_H_ */