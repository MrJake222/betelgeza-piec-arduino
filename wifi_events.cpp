#include "wifi_events.hpp"

#include <ESP8266WiFi.h>

#include "logger.hpp"
extern mrjake::Logger logger;

void mac_to_string(char* buf, const unsigned char* mac) {
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void print_disconnect_reason(WiFiDisconnectReason reason) {
    switch(reason) {
        case WIFI_DISCONNECT_REASON_UNSPECIFIED: logger.print(F("UNSPECIFIED")); break;
        case WIFI_DISCONNECT_REASON_AUTH_EXPIRE: logger.print(F("AUTH_EXPIRE")); break;
        case WIFI_DISCONNECT_REASON_AUTH_LEAVE: logger.print(F("AUTH_LEAVE")); break;
        case WIFI_DISCONNECT_REASON_ASSOC_EXPIRE: logger.print(F("ASSOC_EXPIRE")); break;
        case WIFI_DISCONNECT_REASON_ASSOC_TOOMANY: logger.print(F("ASSOC_TOOMANY")); break;
        case WIFI_DISCONNECT_REASON_NOT_AUTHED: logger.print(F("NOT_AUTHED")); break;
        case WIFI_DISCONNECT_REASON_NOT_ASSOCED: logger.print(F("NOT_ASSOCED")); break;
        case WIFI_DISCONNECT_REASON_ASSOC_LEAVE: logger.print(F("ASSOC_LEAVE")); break;
        case WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED: logger.print(F("ASSOC_NOT_AUTHED")); break;
        case WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD: logger.print(F("DISASSOC_PWRCAP_BAD")); break;
        case WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD: logger.print(F("DISASSOC_SUPCHAN_BAD")); break;
        case WIFI_DISCONNECT_REASON_IE_INVALID: logger.print(F("IE_INVALID")); break;
        case WIFI_DISCONNECT_REASON_MIC_FAILURE: logger.print(F("MIC_FAILURE")); break;
        case WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT: logger.print(F("4WAY_HANDSHAKE_TIMEOUT")); break;
        case WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT: logger.print(F("GROUP_KEY_UPDATE_TIMEOUT")); break;
        case WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS: logger.print(F("IE_IN_4WAY_DIFFERS")); break;
        case WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID: logger.print(F("GROUP_CIPHER_INVALID")); break;
        case WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID: logger.print(F("PAIRWISE_CIPHER_INVALID")); break;
        case WIFI_DISCONNECT_REASON_AKMP_INVALID: logger.print(F("AKMP_INVALID")); break;
        case WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION: logger.print(F("UNSUPP_RSN_IE_VERSION")); break;
        case WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP: logger.print(F("INVALID_RSN_IE_CAP")); break;
        case WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED: logger.print(F("802_1X_AUTH_FAILED")); break;
        case WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED: logger.print(F("CIPHER_SUITE_REJECTED")); break;
        case WIFI_DISCONNECT_REASON_BEACON_TIMEOUT: logger.print(F("BEACON_TIMEOUT")); break;
        case WIFI_DISCONNECT_REASON_NO_AP_FOUND: logger.print(F("NO_AP_FOUND")); break;
        case WIFI_DISCONNECT_REASON_AUTH_FAIL: logger.print(F("AUTH_FAIL")); break;
        case WIFI_DISCONNECT_REASON_ASSOC_FAIL: logger.print(F("ASSOC_FAIL")); break;
        case WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT: logger.print(F("HANDSHAKE_TIMEOUT")); break;
        // default: logger.printf("unknown reason, code %d", reason);
    }
}

String mode_to_string(WiFiMode mode) {
    switch(mode) {
        case WIFI_OFF: return String("WIFI_OFF");
        case WIFI_STA: return String("WIFI_STA");
        case WIFI_AP: return String("WIFI_AP");
        case WIFI_AP_STA: return String("WIFI_AP_STA");
    }
}

/* ------------ EVENTS ------------ */

WiFiEventHandler handler_StationModeConnected;
WiFiEventHandler handler_StationModeDisconnected;
WiFiEventHandler handler_StationModeAuthModeChanged;
WiFiEventHandler handler_StationModeGotIP;
WiFiEventHandler handler_StationModeDHCPTimeout;
WiFiEventHandler handler_SoftAPModeStationConnected;
WiFiEventHandler handler_SoftAPModeStationDisconnected;
WiFiEventHandler handler_SoftAPModeProbeRequestReceived;
WiFiEventHandler handler_WiFiModeChange;

void wifi_event_StationModeConnected(const WiFiEventStationModeConnected& data) {
    char buf[20];
    mac_to_string(buf, data.bssid);
    logger.printf("WiFi STA mode connected to %s (%s) channel %d\n", data.ssid, buf, data.channel);
}

void wifi_event_StationModeDisconnected(const WiFiEventStationModeDisconnected& data) {
    char buf[20];
    mac_to_string(buf, data.bssid);
    logger.printf("WiFi STA mode disconnected from %s (%s) reason ", data.ssid, buf);
    print_disconnect_reason(data.reason);
    logger.print("\n");
}

void wifi_event_StationModeAuthModeChanged(const WiFiEventStationModeAuthModeChanged& data) {
    // empty
}

void wifi_event_StationModeGotIP(const WiFiEventStationModeGotIP& data) {
    logger.printf("WiFi STA mode got ip %s mask %s gw %s\n",
        data.ip.toString().c_str(),
        data.mask.toString().c_str(),
        data.gw.toString().c_str());
}

void wifi_event_StationModeDHCPTimeout() {
    logger.println("WiFi STA mode DHCP timeout");
}

void wifi_event_SoftAPModeStationConnected(const WiFiEventSoftAPModeStationConnected& data) {
    char buf[20];
    mac_to_string(buf, data.mac);
    logger.printf("WiFi AP mode station connected mac %s\n", buf);
}

void wifi_event_SoftAPModeStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& data) {
    char buf[20];
    mac_to_string(buf, data.mac);
    logger.printf("WiFi AP mode station disconnected mac %s\n", buf);
}

void wifi_event_SoftAPModeProbeRequestReceived(const WiFiEventSoftAPModeProbeRequestReceived& data) {
    // empty
}

void wifi_event_WiFiModeChange(const WiFiEventModeChange& data) {
    /*logger.printf("WiFi change mode %s -> %s\n",
        mode_to_string(data.oldMode).c_str(),
        mode_to_string(data.newMode).c_str());*/

    // constantly shows WIFI_AP_STA -> WIFI_OFF
}

void wifi_register_events() {
    // need to hold references for handlers to work

    handler_StationModeConnected = WiFi.onStationModeConnected(wifi_event_StationModeConnected);
    handler_StationModeDisconnected = WiFi.onStationModeDisconnected(wifi_event_StationModeDisconnected);
    handler_StationModeAuthModeChanged = WiFi.onStationModeAuthModeChanged(wifi_event_StationModeAuthModeChanged); // empty
    handler_StationModeGotIP = WiFi.onStationModeGotIP(wifi_event_StationModeGotIP);
    handler_StationModeDHCPTimeout = WiFi.onStationModeDHCPTimeout(wifi_event_StationModeDHCPTimeout);
    handler_SoftAPModeStationConnected = WiFi.onSoftAPModeStationConnected(wifi_event_SoftAPModeStationConnected);
    handler_SoftAPModeStationDisconnected = WiFi.onSoftAPModeStationDisconnected(wifi_event_SoftAPModeStationDisconnected);
    handler_SoftAPModeProbeRequestReceived = WiFi.onSoftAPModeProbeRequestReceived(wifi_event_SoftAPModeProbeRequestReceived); // empty
    handler_WiFiModeChange = WiFi.onWiFiModeChange(wifi_event_WiFiModeChange); // empty
}