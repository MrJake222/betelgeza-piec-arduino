#pragma once

#include <ESP8266WebServer.h>
#include "lib/ArduinoJson.h"

#include "proto_decoder.hpp"

namespace mrjake {

class Server {

    static const size_t _BUF_SIZE = 1024;

    short _port;
    ESP8266WebServer _esp_server;

    // buffer for serialization of JSON
    char _buf[_BUF_SIZE];
    DynamicJsonDocument _json_doc;

    void _serialize_and_send_json();

    void _handle_wifi_scan();
    void _handle_wifi_mode();
    void _handle_wifi_ap_config();
    void _handle_wifi_sta_config();
    void _handle_proto_list_params();

    int _last_error = -1;
    bool _should_check_status = false;
    unsigned long _last_check_millis;
    int _led_state = 0;

    ProtoDecoder& _decoder;

public:
    Server(short port, ProtoDecoder& decoder):
        _port(port),
        _json_doc(1024),
        _esp_server(port),
        _decoder(decoder) { }

    short get_port() { return _port; }

    void start();
    void stop();
    void restart() { stop(); start(); }

    void loop();
};

} // end namespace mrjake