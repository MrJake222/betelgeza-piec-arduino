#include "server.hpp"

#include "LittleFS.h"

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

void Server::start() {
    _esp_server.begin();

    _esp_server.on("/wifi_scan", HTTP_GET, std::bind(&mrjake::Server::_handle_wifi_scan, this));
    _esp_server.on("/wifi_mode", HTTP_GET, std::bind(&mrjake::Server::_handle_wifi_mode, this));
    _esp_server.on("/wifi_ap_config", HTTP_POST, std::bind(&mrjake::Server::_handle_wifi_ap_config, this));
    _esp_server.on("/wifi_sta_config", HTTP_POST, std::bind(&mrjake::Server::_handle_wifi_sta_config, this));

    _esp_server.on("/proto_list_params", HTTP_GET, std::bind(&mrjake::Server::_handle_proto_list_params, this));

    _esp_server.serveStatic("/", LittleFS, "/static/");

    Serial.println("Server listening");
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
            digitalWrite(2, _led_state);

            Serial.print(".");
        }

        else {
            Serial.println("");
            Serial.println(wifi_sta_status_to_string(status));
            Serial.println("IP: " + WiFi.localIP().toString());
            _last_error = status;

            if (WiFi.isConnected()) {
                // OK -> LED turned off
                digitalWrite(2, HIGH);
            } else {
                // not OK -> solid light
                digitalWrite(2, LOW);
            }

            _should_check_status = false;
        }

        _last_check_millis = millis();
    }
}

void Server::_serialize_and_send_json() {
    size_t contentLength = serializeJson(_json_doc, _buf, _BUF_SIZE);
    _esp_server.send(200, "application/json", _buf, contentLength);
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

    // WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP = 2, WIFI_AP_STA = 3
    switch (WiFi.getMode()) {
        case WIFI_OFF:
            _json_doc["mode"] = "off";
            break;

        case WIFI_STA:
            _json_doc["mode"] = "sta";
            break;

        case WIFI_AP:
            _json_doc["mode"] = "ap";
            break;

        case WIFI_AP_STA:
            _json_doc["mode"] = "ap_sta";
            break;
        
        default:
            _json_doc["mode"] = "unknown";
    }

    _json_doc["ap_ssid"] = WiFi.softAPSSID();
    _json_doc["ap_ip"] = WiFi.softAPIP().toString();
    _json_doc["sta_status_string"] = wifi_sta_status_to_string(_last_error != -1 ? _last_error : WiFi.status());
    _json_doc["sta_ip"] = WiFi.localIP().toString();

    _serialize_and_send_json();
}

void Server::_handle_wifi_ap_config() {
    String ap = _esp_server.arg("ap");

    if (ap == "on") {
        Serial.println("turning on AP mode");
        _esp_server.send(200, "text/html", "<h1>AP turned ON</h1><a href='/'>Back.</a>");

        WiFi.persistent(true);
        WiFi.softAP("ESP8266");
    } else {
        Serial.println("turning off AP mode");
        _esp_server.send(200, "text/html", "<h1>AP turned OFF</h1><a href='/'>Back.</a>");

        WiFi.softAPdisconnect(true);
    }
}

void Server::_handle_wifi_sta_config() {
    String sta = _esp_server.arg("sta");

    if (sta == "off") {
        Serial.println("turning off STA mode");
        _esp_server.send(200, "text/html", "<h1>STA mode turned OFF</h1><a href='/'>Back.</a>");
        WiFi.disconnect(true);
    }

    else {
        if (!_esp_server.hasArg("ssid")) {
            _esp_server.send(400, "text/html", "No SSID given, no action performed");
            return;
        }

        String ssid = _esp_server.arg("ssid");
        String pass = _esp_server.arg("pass");

        Serial.println("turning on STA mode");
        _esp_server.sendHeader("Location", "/sta_activating.html");
        _esp_server.send(301, "text/plain", "Redirection in progress...");

        Serial.println("Trying:");
        Serial.println("  SSID: " + ssid);
        Serial.println("  pass: " + pass);

        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);
        WiFi.begin(ssid, pass);
        
        Serial.println("waiting for STA connection");

        pinMode(2, OUTPUT);
        // Low turns on

        _should_check_status = true;
        _last_check_millis = millis();

        digitalWrite(2, LOW);
        _led_state = 0;
    }
}

// Returns parameters as hex strings with numerical (decimal) values
void Server::_handle_proto_list_params() {

    _json_doc.clear();

    _json_doc["last_status"] = _decoder.get_last_status();
    _json_doc["last_good_frame"] = _decoder.get_last_good_frame_received();

    Serial.println("Reading parameters");

    for (const auto entry : _decoder) {
        Serial.printf("  param %04X is %04X\n", entry.first, entry.second);
        
        _json_doc[String(entry.first, 16)] = entry.second;
    }

    _serialize_and_send_json();
}

} // end namespace