#pragma once

#include <cstdint>
#include <cstddef>
#include <map>

#include <SoftwareSerial.h>

namespace mrjake {

class ProtoDecoder {

    // types
    using ParamMap = std::map<uint16_t, uint16_t>;

    // buffer
    const static size_t _BUF_SIZE = 1024;
    uint8_t _frame_buf[_BUF_SIZE];

    // parameter map
    ParamMap _params;

    // dependencies
    SoftwareSerial& _softSerial;

    // status variables
    String _last_status;
    String _last_good_frame_received;

    // functions
    void _print_data(size_t len);
    
    uint16_t _crc16_mcrf4xx(size_t len);
    bool _verify_frame(size_t bytes_read);
    
public:
    ProtoDecoder(SoftwareSerial& softSerial) :
        _softSerial(softSerial) { }

    String get_last_status() { return _last_status; }
    String get_last_good_frame_received() { return _last_good_frame_received; }

    void read_nonblock();
    ParamMap::const_iterator begin() const;
    ParamMap::const_iterator end() const;
};

} // end namespace