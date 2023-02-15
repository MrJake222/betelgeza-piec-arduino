#include "proto_decoder.hpp"

#include <HardwareSerial.h>

#include "helpers.hpp"
#include "config.h"

#include "logger.hpp"
extern mrjake::Logger logger;

namespace mrjake {

void ProtoDecoder::_print_data(size_t bytes_read) {
    Serial.printf("\n%d bytes read: ", bytes_read);

    for (size_t i=0; i<bytes_read; i++) {
        Serial.printf("%02X ", _frame_buf[_bytes_in_buffer + i]);
    }

    Serial.println();
}

void ProtoDecoder::_print_buffer(size_t bytes) {
    Serial.printf("%d bytes in buffer: ", bytes);

    for (size_t i=0; i<bytes; i++) {
        Serial.printf("%02X ", _frame_buf[i]);
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

bool ProtoDecoder::_verify_frame() {

    char buf[80];

    /*if (_bytes_in_buffer < 6 || _bytes_in_buffer % 2) {
        _last_status = F("ERR: frame length less than 6 or not even length");
        return false;
    }*/

    if (_frame_buf[0] != 0x02 || _frame_buf[1] != 0x26) {
        _last_status = F("ERR: frame start wrong");
        return false;
    }

    /*if (_frame_buf[_bytes_in_buffer-4] != 0x02 || _frame_buf[_bytes_in_buffer-3] != 0x18) {
        _last_status = F("ERR: frame end wrong");
        return false;
    }*/

    if (_frame_buf[2] != _ADDR_H || _frame_buf[3] != _ADDR_L) {
        // don't change last status, silently drop
        Serial.printf("WARN: frame destination not matching: is %02X%02X, should be %04X\n", _frame_buf[2], _frame_buf[3], _ADDR);
        return false;
    }

    if ((_bytes_in_buffer-8) % 4) {
         _last_status = F("ERR: frame data segment not multiple of 4");
        return false;
    }

    uint16_t crc_should_be = ((uint16_t)_frame_buf[_bytes_in_buffer-2]) << 8 | _frame_buf[_bytes_in_buffer-1];
    uint16_t crc_is = _crc16_mcrf4xx(_bytes_in_buffer - 4);
    if (crc_should_be != crc_is) {
        sprintf(buf, "ERR: frame CRC wrong: is %04X, should be %04X", crc_is, crc_should_be);
        _last_status = String(buf);
        return false;
    }

    _last_status = F("INF: frame ok");

    return true;
}

void ProtoDecoder::_send_response() {
    
    // _params_to_send[0x01F6] = 50; // co = 50
    // _params_to_send[0x028E] = 51; // cwu = 51
    // _params_to_send[0x0245] = 2; // pompy = 0
    uint16_t new_time = tech_get_time();

    // send time updates if clock set (not default 1 Jan 1970)
    if (get_year() > 1970) {
        if (_params_received_now.count(F_TIME_r)) {
            int diff = tech_time_diff(_params[F_TIME_r], new_time);
            if (diff >= 5) {
                // 5 min difference
                _params_to_send[F_TIME_W] = new_time;
                logger.printf("sending time, was: %s(hex %x), is: %s(hex %x), diff: %d\n",
                    tech_time_to_string(_params[F_TIME_r]).c_str(),
                    _params[F_TIME_r],
                    tech_time_to_string(new_time).c_str(),
                    new_time,
                    diff
                );
            }
        }

        if (_params_received_now.count(F_WDAY_r)) {
            uint16_t new_wday = get_wday();

            /*
            * update before 23:55 and after 0:05
            */
            if (_params[F_WDAY_r] != new_wday && tech_time_diff(new_time, 0) >= 5) {
                // one-day difference
                _params_to_send[F_WDAY_W] = new_wday;
                logger.printf("sending wday, was %d, is %d\n", _params[F_WDAY_r], new_wday);
            }
        } 
    }

    // start is good, address is good
    size_t i = 4;
    for (auto entry : _params_to_send) {
        _frame_buf[i++] = entry.first >> 8;
        _frame_buf[i++] = entry.first;
        _frame_buf[i++] = entry.second >> 8;
        _frame_buf[i++] = entry.second;
    }

    // without end of frame
    uint16_t crc = _crc16_mcrf4xx(i);

    // end of frame + CRC
    _frame_buf[i++] = 0x02;
    _frame_buf[i++] = 0x18;
    _frame_buf[i++] = crc >> 8;
    _frame_buf[i++] = crc;

    Serial.print("sending: ");
    _print_buffer(i);

    delay(5);

    digitalWrite(SOFT_TX, HIGH);
    delay(2);

    for (int k=0; k<i; k++) {
        _softSerial.write(_frame_buf[k]);
        delayMicroseconds(800);
    }

    digitalWrite(SOFT_TX, LOW);
    _params_to_send.clear();
}

bool ProtoDecoder::has_param(uint16_t param) {
    return _params.count(param) > 0;
}

int ProtoDecoder::get_param(uint16_t param) {
    if (!has_param(param)) {
        return -1;
    }
    
    return _params[param];
}

void ProtoDecoder::schedule_for_send(uint16_t param, uint16_t value) {
    _params_to_send[param] = value;
}

void ProtoDecoder::read_nonblock() {

    size_t bytes_currently_read = _softSerial.read(_frame_buf + _bytes_in_buffer, _BUF_SIZE - _bytes_in_buffer);
    // if (bytes_currently_read > 0) {
    //     _print_data(bytes_currently_read);
    // }
    _bytes_in_buffer += bytes_currently_read;

    if (_bytes_in_buffer >= 6 && _frame_buf[_bytes_in_buffer-4] == 0x02 && _frame_buf[_bytes_in_buffer-3] == 0x18) {
        // frame end detected in buffer

        bool ok = _verify_frame();
        Serial.print("Received frame, status: " + _last_status);
        Serial.printf(", bad frames: %d/%d\n", _wrong_frames, (_ok_frames + _wrong_frames));
        if (ok) {
            _last_good_frame_received = get_date_time();

            size_t data_start = 4;
            size_t data_end = _bytes_in_buffer - 4;
            size_t data_entries = (data_end - data_start) / 4;

            Serial.printf("received %d parameters\n", data_entries);
            _params_received_now.clear();

            for (size_t i=0; i<data_entries; i++) {
                size_t param_start = data_start + i*4;
                uint16_t param = ((uint16_t)_frame_buf[param_start]) << 8 | _frame_buf[param_start+1];
                uint16_t value = ((uint16_t)_frame_buf[param_start+2]) << 8 | _frame_buf[param_start+3];
                
                // Serial.printf("  %X,%04X\n", param, value);
                _params[param] = value;
                _params_received_now.insert(param);
            }

            _ok_frames++;
            _send_response();
        }
        else {
            // not ok
            // Serial.printf("received %d bytes: ", _bytes_in_buffer);
            // _print_buffer(_bytes_in_buffer);
            if (_frame_buf[2] == _ADDR_H && _frame_buf[3] == _ADDR_L) {
                // to this module
                _wrong_frames++;
            }
        }

        _bytes_in_buffer = 0;
        // Serial.println("");
    }
    else if (_bytes_in_buffer >= _BUF_SIZE) {
        // buffer full, discard
        _bytes_in_buffer = 0;
    }
}

ProtoDecoder::ParamMap::const_iterator ProtoDecoder::begin() const {
    return _params.begin();
}

ProtoDecoder::ParamMap::const_iterator ProtoDecoder::end() const {
    return _params.end();
}

} // end namespace