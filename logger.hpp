#pragma once

#include <Print.h>
#include <HardwareSerial.h>
#include <LittleFS.h>

namespace mrjake {

/**
 * Sends characters both to file and to serial
 */

class Logger : public Print {

    static const size_t BUF_SIZE = 1024;            // 1 K -- 512 iterations to copy 0.5MB of logs
    static char cpy_buffer[BUF_SIZE];

    // bytes
    const size_t MAX_UNFLUSHED = 512;
    const size_t TRUNCATE_TO = 512 * 1024;          // 0.5 MB
    const size_t TRUNCATE_TRIGGER = 1024 * 1024;    // 1 MB
    
    File file;
    HardwareSerial& serial;

    char filename[32];
    bool prepend_flag = true;
    int characters_since_flush = 0;

    void prepend_date();
    void truncate();

public:
    Logger(HardwareSerial& serial_) : serial(serial_) { }
    
    void begin(const char* filename);
    void flush();

    size_t write(uint8_t c) override;
};

}; // end namespace