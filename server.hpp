#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include "lib/ArduinoJson.h"

#include "proto_decoder.hpp"

namespace mrjake {

String wifi_sta_status_to_string(int status);

class Server {

    static const size_t _BUF_SIZE = 1024;

    short _port;
    ESP8266WebServer _esp_server;
    ESP8266HTTPUpdateServer _esp_update_server;

    // buffer for serialization of JSON
    char _buf[_BUF_SIZE];
    DynamicJsonDocument _json_doc;

    void _serialize_and_send_json();
    void _redirect(String url);

    void _config_changed();
    void (*_config_changed_callback)(void);

    void _handle_wifi_scan();
    void _handle_wifi_mode();
    void _handle_wifi_ap_config();
    void _handle_wifi_sta_config();
    void _handle_proto_list_params();
    void _handle_proto_get_params();
    void _handle_proto_set_params();
    void _handle_pump_set_time();
    void _handle_log();

    int _last_error = -1;
    bool _should_check_status = false;
    unsigned long _last_check_millis;
    int _led_state = 0;

    bool _should_track_pump = false;
    unsigned long _pump_start;
    unsigned long _pump_time_to_run;

    ProtoDecoder& _decoder;

public:
    Server(short port, ProtoDecoder& decoder, void (*config_changed_callback)(void)):
        _port(port),
        _json_doc(1024),
        _esp_server(port),
        _decoder(decoder),
        _config_changed_callback(config_changed_callback) {
            _esp_update_server.setup(&_esp_server);
        }

    short get_port() { return _port; }

    void start();
    void stop();
    void restart() { stop(); start(); }

    void loop();
};

} // end namespace mrjake