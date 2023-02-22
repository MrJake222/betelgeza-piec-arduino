#include "server.hpp"

#include <LittleFS.h>
#include "helpers.hpp"

#include "logger.hpp"
extern mrjake::Logger logger;

namespace mrjake {

/**
 * Helpers
 */
String wifi_sta_status_to_string(int status) {
    switch (status) {
        case WL_CONNECTED:
            return "Connected to " + WiFi.SSID();

        case WL_NO_SSID_AVAIL:
            return "No such network";
            
        case WL_CONNECT_FAILED:
            return "Connection failed";

        case WL_WRONG_PASSWORD:
            return "Wrong password";

        case WL_IDLE_STATUS:
            return "Idle";

        case WL_DISCONNECTED:
            return "Disconnected";

        default:
            return "Unknown error, code: " + status;
    }
}

uint16_t get_cu_state(uint16_t cu_state, uint16_t cu_standby, uint16_t cu_fan, uint16_t cu_feed, uint16_t co_current, uint16_t co_target) {
    if (cu_state != 0) {
        return cu_state;
    }

    // cu_state == 0, ambiguous

    if (cu_standby == 0) {
        return 0; // original turned off state
    }

    // cu_standby == 1

    if (cu_fan > 0) {
        // can't be turned on, it's one of the default states (cu_state==2)
        // here also can be turning off (large fan speed)

        if (co_current > co_target) {
            // if large temperature
            return STATE_TURNING_OFF;
        }
        
        // smaller or equal temperature
        return STATE_TURNING_ON;
    }

    // cu_fan == 0

    if (cu_feed == 1) {
        // fan 0, feed working -> initial state of turning on
        return STATE_TURNING_ON;
    }

    // cu_feed == 0

    // no feed, no fan, standby
    return STATE_STANDBY;

    // TODO add turning off decision tree
}

void Server::start() {
    _esp_server.begin();

    _esp_server.on("/wifi_scan", HTTP_GET, std::bind(&mrjake::Server::_handle_wifi_scan, this));
    _esp_server.on("/wifi_mode", HTTP_GET, std::bind(&mrjake::Server::_handle_wifi_mode, this));
    _esp_server.on("/wifi_ap_config", HTTP_POST, std::bind(&mrjake::Server::_handle_wifi_ap_config, this));
    _esp_server.on("/wifi_sta_config", HTTP_POST, std::bind(&mrjake::Server::_handle_wifi_sta_config, this));

    _esp_server.on("/proto_list_params", HTTP_GET, std::bind(&mrjake::Server::_handle_proto_list_params, this));
    _esp_server.on("/proto_get_params", HTTP_GET, std::bind(&mrjake::Server::_handle_proto_get_params, this));
    _esp_server.on("/proto_set_params", HTTP_POST, std::bind(&mrjake::Server::_handle_proto_set_params, this));
    _esp_server.on("/proto_turn_on", HTTP_POST, std::bind(&mrjake::Server::_handle_proto_turn_on, this));
    _esp_server.on("/proto_turn_off", HTTP_POST, std::bind(&mrjake::Server::_handle_proto_turn_off, this));
    
    _esp_server.on("/pump_set_time", HTTP_POST, std::bind(&mrjake::Server::_handle_pump_set_time, this));

    _esp_server.on("/log", HTTP_GET, std::bind(&mrjake::Server::_handle_log, this));

    _esp_server.serveStatic("/", LittleFS, "/static/");

    logger.println("Server listening");
}

void Server::stop() {
    _esp_server.stop();
}

void Server::loop() {
    _esp_server.handleClient();

    if (_should_check_status && ((millis() - _last_check_millis) >= 250)) {
        
        wl_status_t status;
        if ((status = WiFi.status()) == WL_DISCONNECTED) {
            _led_state ^= 1;
            digitalWrite(P_STATUS_LED, _led_state);

            //logger.print(".");
        }

        else {
            //logger.println("");
            //logger.println(wifi_sta_status_to_string(status));
            //logger.println("IP: " + WiFi.localIP().toString());
            _last_error = status;

            if (WiFi.isConnected()) {
                // OK -> LED turned off
                digitalWrite(P_STATUS_LED, HIGH);
            } else {
                // not OK -> solid light
                digitalWrite(P_STATUS_LED, LOW);
            }

            _should_check_status = false;
            _config_changed();
        }

        _last_check_millis = millis();
    }

    if (_should_track_pump && ((millis() - _pump_start) >= _pump_time_to_run)) {
        digitalWrite(P_PUMP, LOW);

        logger.println("pump OFF");
        _should_track_pump = false;
    }
}

void Server::_serialize_and_send_json() {
    size_t contentLength = serializeJson(_json_doc, _buf, _BUF_SIZE);
    _esp_server.send(200, "application/json", _buf, contentLength);
}

void Server::_redirect(String url) {
    _esp_server.sendHeader("Location", url);
    _esp_server.send(301, "text/plain", "Redirection in progress...");
}

void Server::_config_changed() {
    restart();
    _config_changed_callback();
}

/**
 * Routes
 */
void Server::_handle_wifi_scan() {

    _json_doc.clear();

    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect();
    delay(100);
    int n = WiFi.scanNetworks();

    for (int i=0; i<n; i++) {
        _json_doc[i]["ssid"] = WiFi.SSID(i);
        _json_doc[i]["rssi"] = WiFi.RSSI(i);
        _json_doc[i]["bssid"] = WiFi.BSSIDstr(i);
        _json_doc[i]["channel"] = WiFi.channel(i);
        _json_doc[i]["hidden"] = WiFi.isHidden(i);

        uint8_t enc = WiFi.encryptionType(i);
        switch (enc) {
            case ENC_TYPE_NONE: // open
                _json_doc[i]["enc"] = "open";
                break;

            case ENC_TYPE_WEP: // wep
                _json_doc[i]["enc"] = "wep";
                break;

            case ENC_TYPE_TKIP: // wpa
                _json_doc[i]["enc"] = "wpa";
                break;

            case ENC_TYPE_CCMP: // wpa2
            case ENC_TYPE_AUTO: // wpa2
                _json_doc[i]["enc"] = "wpa2";
                break;
            
            default:
                _json_doc[i]["enc"] = "unknown";
        }

        _json_doc[i]["connected"] = WiFi.isConnected() && (WiFi.SSID() == WiFi.SSID(i));
    }

    _serialize_and_send_json();
}

void Server::_handle_wifi_mode() {

    _json_doc.clear();
    bool ap = false;
    bool sta = false;

    // WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP = 2, WIFI_AP_STA = 3
    switch (WiFi.getMode()) {
        case WIFI_OFF:
            _json_doc["mode"] = "off";
            break;

        case WIFI_STA:
            _json_doc["mode"] = "sta";
            sta = true;
            break;

        case WIFI_AP:
            _json_doc["mode"] = "ap";
            ap = true;
            break;

        case WIFI_AP_STA:
            _json_doc["mode"] = "ap_sta";
            ap = true;
            sta = true;
            break;
        
        default:
            _json_doc["mode"] = "unknown";
    }

    if (ap) {
        _json_doc["ap_ssid"] = WiFi.softAPSSID();
        _json_doc["ap_ip"] = WiFi.softAPIP().toString();
    }

    if (sta) {
        _json_doc["sta_status_string"] = wifi_sta_status_to_string(_last_error != -1 ? _last_error : WiFi.status());
        _json_doc["sta_ip"] = WiFi.localIP().toString();
    }

    _serialize_and_send_json();
}

void Server::_handle_wifi_ap_config() {
    String ap = _esp_server.arg("ap");

    if (ap == "on") {
        logger.println("turning on AP mode");
        _redirect("/status/ap_on.html");

        WiFi.persistent(true);
        WiFi.softAP("ESP8266");
    } else {
        logger.println("turning off AP mode");
        _redirect("/status/ap_off.html");

        WiFi.softAPdisconnect(true);
    }

    _config_changed();
}

void Server::_handle_wifi_sta_config() {
    String sta = _esp_server.arg("sta");

    if (sta == "off") {
        logger.println("turning off STA mode");
        _redirect("status/sta_off.html");
        WiFi.disconnect(true);
    }

    else {
        if (!_esp_server.hasArg("ssid")) {
            _redirect("status/sta_e_ssid.html");
            return;
        }

        String ssid = _esp_server.arg("ssid");
        String pass = _esp_server.arg("pass");

        logger.println("turning on STA mode");
        _redirect("/status/sta_on.html");

        logger.println("Trying:");
        logger.println("  SSID: " + ssid);
        logger.println("  pass: " + pass);

        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);
        WiFi.begin(ssid, pass);
        
        logger.println("waiting for STA connection");

        _should_check_status = true;
        _last_check_millis = millis();

        digitalWrite(P_STATUS_LED, LOW);
        _led_state = 0;
    }
}

// Returns parameters as hex strings as numerical (decimal) values
void Server::_handle_proto_list_params() {

    _json_doc.clear();

    _json_doc["last_status"] = _decoder.get_last_status();
    _json_doc["last_good_frame"] = _decoder.get_last_good_frame_received();
    _json_doc["good_frames"] = _decoder.get_ok_frames();
    _json_doc["wrong_frames"] = _decoder.get_wrong_frames();

    // logger.println("Reading parameters to web");

    for (const auto entry : _decoder) {
        // logger.printf("  param %04X is %04X\n", entry.first, entry.second);
        
        _json_doc[String(entry.first, 16)] = entry.second;
    }

    _serialize_and_send_json();
}

// Returns parameters with descriptions as numerical (decimal) values
void Server::_handle_proto_get_params() {

    _json_doc.clear();

    // Module state
    _json_doc["last_status"] = _decoder.get_last_status();
    _json_doc["last_good_frame"] = _decoder.get_last_good_frame_received();
    _json_doc["good_frames"] = _decoder.get_ok_frames();
    _json_doc["wrong_frames"] = _decoder.get_wrong_frames();

    // Heating unit state
    if (_decoder.has_param(F_TIME_r))
        _json_doc["time"] = tech_time_to_string(_decoder.get_param(F_TIME_r));
    if (_decoder.has_param(F_WDAY_r))
        _json_doc["wday"] = _decoder.get_param(F_WDAY_r);

    if (_decoder.has_param(F_CO_CURRENT_r))
        _json_doc["co_current"] = (float)(_decoder.get_param(F_CO_CURRENT_r)) / 10;
    if (_decoder.has_param(F_CO_TARGET_r))
        _json_doc["co_target"] = _decoder.get_param(F_CO_TARGET_r);
    if (_decoder.has_param(F_CO_RANGE_r)) {
        _json_doc["co_min"] = _decoder.get_param(F_CO_RANGE_r) & 0xFF;
        _json_doc["co_max"] = _decoder.get_param(F_CO_RANGE_r) >> 8;
    }

    if (_decoder.has_param(F_CWU_CURRENT_r))
        _json_doc["cwu_current"] = (float)(_decoder.get_param(F_CWU_CURRENT_r)) / 10;
    if (_decoder.has_param(F_CWU_TARGET_r))
        _json_doc["cwu_target"] = _decoder.get_param(F_CWU_TARGET_r);
    if (_decoder.has_param(F_CWU_RANGE_r)) {
        _json_doc["cwu_min"] = _decoder.get_param(F_CWU_RANGE_r) & 0xFF;
        _json_doc["cwu_max"] = _decoder.get_param(F_CWU_RANGE_r) >> 8;
    }

    if (_decoder.has_param(F_VALVE_CURRENT_r))
        _json_doc["valve_current"] = (float)(_decoder.get_param(F_VALVE_CURRENT_r)) / 10;
    if (_decoder.has_param(F_VALVE_TARGET_r))
        _json_doc["valve_target"] =  _decoder.get_param(F_VALVE_TARGET_r);
    if (_decoder.has_param(F_VALVE_RANGE_r)) {
        _json_doc["valve_min"] = _decoder.get_param(F_VALVE_RANGE_r) & 0xFF;
        _json_doc["valve_max"] = _decoder.get_param(F_VALVE_RANGE_r) >> 8;
    }

    if (_decoder.has_param(F_PUMPS_MODE_r))
        _json_doc["pumps_mode"] = _decoder.get_param(F_PUMPS_MODE_r);

    // decision tree for cu_state
    if (_decoder.has_param(F_CU_STATE_r)
        && _decoder.has_param(F_CU_STANDBY_r)
        && _decoder.has_param(F_CU_FAN_r)
        && _decoder.has_param(F_CU_FEED_r)
        && _decoder.has_param(F_CO_CURRENT_r)
        && _decoder.has_param(F_CO_TARGET_r)) {
        
        _json_doc["cu_state"] = get_cu_state(
            _decoder.get_param(F_CU_STATE_r),
            _decoder.get_param(F_CU_STANDBY_r),
            _decoder.get_param(F_CU_FAN_r),
            _decoder.get_param(F_CU_FEED_r),
            _decoder.get_param(F_CO_CURRENT_r) / 10,
            _decoder.get_param(F_CO_TARGET_r));
    }

    if (_should_track_pump)
        _json_doc["circulation_pump_time_left"] = (_pump_time_to_run + _pump_start - millis()) / 1000;

    _json_doc["aux_in"] = !digitalRead(P_IN);

    _serialize_and_send_json();
}

void Server::_handle_proto_set_params() {

    logger.printf("set parameters from %s\n", get_client_ip().c_str());

    if (_esp_server.hasArg("co_target")) {
        int val = _esp_server.arg("co_target").toInt();

        logger.printf("setting co_target to %d\n", val);
        _decoder.schedule_for_send(F_CO_TARGET_W, val);
    }

    if (_esp_server.hasArg("cwu_target")) {
        int val = _esp_server.arg("cwu_target").toInt();

        logger.printf("setting cwu_target to %d\n", val);
        _decoder.schedule_for_send(F_CWU_TARGET_W, val);
    }

    if (_esp_server.hasArg("pumps_mode")) {
        int val = _esp_server.arg("pumps_mode").toInt();

        logger.printf("setting pumps_mode to %d\n", val);
        _decoder.schedule_for_send(F_PUMPS_MODE_W, val);
    }

    if (_esp_server.hasArg("valve_target")) {
        int val = _esp_server.arg("valve_target").toInt();

        logger.printf("setting valve_target to %d\n", val);
        _decoder.schedule_for_send(F_VALVE_TARGET_W, val);
    }

    _redirect("/status/param_set.html");
}

void Server::_handle_proto_turn_on() {
    logger.printf("turn on from %s\n", get_client_ip().c_str());
    _decoder.schedule_for_send(F_TURN_ON_W, 1);
    
    _redirect("/status/turned_on.html");
}

void Server::_handle_proto_turn_off() {
    logger.printf("turn off from %s\n", get_client_ip().c_str());
    _decoder.schedule_for_send(F_TURN_OFF_W, 1);

    _redirect("/status/turned_off.html");
}

void Server::_handle_pump_set_time() {

    digitalWrite(P_PUMP, HIGH);

    _pump_start = millis();
    
    int min = _esp_server.arg("force_time").toInt();
    _pump_time_to_run = min*60*1000;
    _should_track_pump = true;

    logger.printf("pump ON from %s for %d minutes (%u millis)\n", get_client_ip().c_str(), min, _pump_time_to_run);

    _redirect("/status/pump_on.html");
}

void Server::_handle_log() {
    logger.flush();

    File file = LittleFS.open("/logfile", "r");
    _esp_server.streamFile(file, "text/plain");
    file.close();
}

} // end namespace