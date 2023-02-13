#include "logger.hpp"
#include "helpers.hpp"

namespace mrjake {

char Logger::cpy_buffer[Logger::BUF_SIZE];

void Logger::prepend_date() {
    file.print(get_date_time());
    serial.print(get_date_time());
    file.print(": ");
    serial.print(": ");
}

void Logger::begin(const char* filename) {
    strcpy(this->filename, filename);

    file = LittleFS.open(filename, "a+"); // append
    if (!file) {
        serial.println("log open failed");
        while (1) delay(1000);
    } else{
        serial.println("log file opened");
    }
}

void Logger::flush() {
    file.flush();
    characters_since_flush = 0;
}

void Logger::truncate() {
    file.seek(TRUNCATE_TO, SeekEnd);
    
    while (file.read() != '\n'); // find first newline
    File file_tmp = LittleFS.open("/tmp", "w");
    
    size_t read;
    do {
        read = file.readBytes(Logger::cpy_buffer, Logger::BUF_SIZE);
        file_tmp.write(Logger::cpy_buffer, read);
    } while (read == Logger::BUF_SIZE);

    file_tmp.close();
    LittleFS.rename("/tmp", filename);

    file = LittleFS.open(filename, "a+");
}

size_t Logger::write(uint8_t c) {
    
    if (prepend_flag) {
        prepend_date();
        prepend_flag = false;
    }

    file.write(c);
    serial.write(c);
    if (c == '\n') {
        prepend_flag = true;
    }
    
    characters_since_flush += 1;
    if (characters_since_flush >= MAX_UNFLUSHED) {
        flush();
        if (file.size() >= TRUNCATE_TRIGGER)
            truncate();
    }

    return 1;
}

}; // end namespace