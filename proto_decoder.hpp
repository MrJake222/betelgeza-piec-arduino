#pragma once

#include <cstdint>
#include <cstddef>
#include <map>
#include <set>

#include <SoftwareSerial.h>
#include "config.h"

namespace mrjake {

class ProtoDecoder {

    // types
    using ParamMap = std::map<uint16_t, uint16_t>;
    using ParamSet = std::set<uint16_t>;

    const static uint16_t _ADDR = ADDR_ETH_MODULE;
    const static uint8_t _ADDR_H = _ADDR >> 8;
    const static uint8_t _ADDR_L = _ADDR;

    // buffer
    const static size_t _BUF_SIZE = 256;
    uint8_t _frame_buf[_BUF_SIZE];
    size_t _bytes_in_buffer = 0;

    // parameter map
    ParamMap _params;
    ParamSet _params_received_now;
    ParamMap _params_to_send;

    // dependencies
    SoftwareSerial& _softSerial;

    // status variables
    String _last_status;
    String _last_good_frame_received;
    unsigned long _ok_frames;
    unsigned long _wrong_frames;

    // functions
    void _print_data(size_t len);
    void _print_buffer(size_t bytes);
    
    uint16_t _crc16_mcrf4xx(size_t len);
    bool _verify_frame();
    void _send_response();
    
public:
    ProtoDecoder(SoftwareSerial& softSerial) :
        _softSerial(softSerial) { }

    static size_t get_buf_size() { return _BUF_SIZE; }
    String get_last_status() { return _last_status; }
    String get_last_good_frame_received() { return _last_good_frame_received; }
    unsigned long get_ok_frames() { return _ok_frames; }
    unsigned long get_wrong_frames() { return _wrong_frames; }

    /* -1 if no param */
    bool has_param(uint16_t param);
    int get_param(uint16_t param);
    void schedule_for_send(uint16_t param, uint16_t value);
    void read_nonblock();

    ParamMap::const_iterator begin() const;
    ParamMap::const_iterator end() const;
};

} // end namespace