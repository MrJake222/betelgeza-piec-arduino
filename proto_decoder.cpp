#include "proto_decoder.hpp"

#include <HardwareSerial.h>

#include "helpers.hpp"

namespace mrjake {

void ProtoDecoder::_print_data(size_t bytes_read) {
    Serial.printf("%d bytes read: ", bytes_read);

    for (size_t i=0; i<bytes_read; i++) {
        Serial.printf("%X ", _frame_buf[i]);
    }

    Serial.println();
}

/*
 * frame_len NOT counting stop word and CRC
 */
uint16_t ProtoDecoder::_crc16_mcrf4xx(size_t frame_len) {
    uint16_t crc = 0xFFFF;
    for (size_t i=0; i<frame_len; i++) {
        crc ^= _frame_buf[i];
        for (int i=0; i<8; i++) {
            if (crc & 1) crc = (crc >> 1) ^ 0x8408;
            else crc = (crc >> 1);
        }
    }
    
    return crc;
}

bool ProtoDecoder::_verify_frame(size_t bytes_read) {

    char buf[80];

    if (bytes_read < 6 || bytes_read % 2) {
        _last_status = F("ERR: frame length less than 6 or not even length");
        return false;
    }

    if (_frame_buf[0] != 0x02 || _frame_buf[1] != 0x26) {
        _last_status = F("ERR: frame start wrong");
        return false;
    }

    if (_frame_buf[bytes_read-4] != 0x02 || _frame_buf[bytes_read-3] != 0x18) {
        _last_status = F("ERR: frame end wrong");
        return false;
    }

    if (_frame_buf[2] != 0xFF || _frame_buf[3] != 0xF4) {
        // don't change last status, silently drop
        Serial.printf("WARN: frame destination not matching: is %02X%02X, should be FFF4", _frame_buf[2], _frame_buf[3]);
        return false;
    }

    if ((bytes_read-8) % 4) {
         _last_status = F("ERR: frame data segment not multiple of 4");
        return false;
    }

    uint16_t crc_should_be = ((uint16_t)_frame_buf[bytes_read-2]) << 8 | _frame_buf[bytes_read-1];
    uint16_t crc_is = _crc16_mcrf4xx(bytes_read - 4);
    if (crc_should_be != crc_is) {
        sprintf(buf, "ERR: frame CRC wrong: is %04X, should be %04X", crc_is, crc_should_be);
        _last_status = String(buf);
        return false;
    }

    _last_status = F("INF: frame ok");

    return true;
}

void ProtoDecoder::read_nonblock() {

    if (!_softSerial.available()) {
        return;
    }

    size_t bytes_read = _softSerial.readBytes(_frame_buf, _BUF_SIZE);
    _print_data(bytes_read);

    bool ok = _verify_frame(bytes_read);
    Serial.println(_last_status);
    if (!ok) {
        return;
    }

    _last_good_frame_received = get_date_time();

    size_t data_start = 4;
    size_t data_end = bytes_read - 4;
    size_t data_entries = (data_end - data_start) / 4;

    for (size_t i=0; i<data_entries; i++) {
        size_t param_start = data_start + i*4;
        uint16_t param = ((uint16_t)_frame_buf[param_start]) << 8 | _frame_buf[param_start+1];
        uint16_t value = ((uint16_t)_frame_buf[param_start+2]) << 8 | _frame_buf[param_start+3];
        
        Serial.printf("data entry %d: param %04X value %04X\n", i, param, value);
        _params[param] = value;
    }
}

ProtoDecoder::ParamMap::const_iterator ProtoDecoder::begin() const {
    return _params.begin();
}

ProtoDecoder::ParamMap::const_iterator ProtoDecoder::end() const {
    return _params.end();
}

} // end namespace