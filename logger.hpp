#pragma once

#include <Print.h>
#include <HardwareSerial.h>
#include <LittleFS.h>

namespace mrjake {

/**
 * Sends characters both to file and to serial
 */

class Logger : public Print {

    File file;
    HardwareSerial& serial;

public:
    Logger(HardwareSerial& serial_) : serial(serial_) { }

    void begin(const char* filename) {
        file = LittleFS.open(filename, "a"); // append
        if (!file) {
            Serial.println("log open failed");
            while (1) delay(1000);
        } else{
            Serial.println("log file opened");
        }
    }

    size_t write(uint8_t c) override {
        
        file.write(c);
        file.flush();
        
        return serial.write(c);
    }

    size_t write(const uint8_t *buffer, size_t size) override {
        
        file.write(buffer, size);
        file.flush();
        
        return serial.write(buffer, size);
    }
};

}; // end namespace