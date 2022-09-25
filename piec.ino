#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <SoftwareSerial.h>

#include "helpers.hpp"
#include "server.hpp"
#include "proto_decoder.hpp"

#define NTP_SERVER "pl.pool.ntp.org"           
#define TZ "CET-1CEST,M3.5.0,M10.5.0/3" 

SoftwareSerial softSerial;

void serial_init() {
    Serial.begin(115200);
    Serial.println();
}

void soft_serial_init() {
    // Rx -- GPIO 12
    // Tx -- GPIO 14
    softSerial.begin(9600, SWSERIAL_8N1, 12, 14);

    if (!softSerial) {
        Serial.println("softSerial init failed");
        while (1) delay(1000);
    }

    softSerial.setTimeout(100);
    Serial.println("softSerial OK");
}

bool fs_init() {
    bool ok = LittleFS.begin();
    if (!ok) {
        Serial.println("failed to setup FS");
        return false;
    }

    Serial.println("FS init ok");
    Serial.println("files present:");
    listAllFilesInDir("/");
    Serial.println("");

    return true;
}

bool wifi_ap_init() {
    Serial.println("setting up AP");
    
    bool ok = WiFi.softAP("ESP8266");
    if (!ok) {
        Serial.println("failed to setup AP");
        return false;
    }

    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    return true;
}

mrjake::ProtoDecoder decoder(softSerial);
mrjake::Server server(80, decoder);



void setup() {
    serial_init();
    soft_serial_init();

    fs_init();

    WiFi.begin();

    for (int i=0; i<20 && !WiFi.isConnected(); i++) {
        Serial.print("Connection attempt ");
        Serial.print(i+1);
        Serial.println("/20");
        delay(1000);
    }

    if (WiFi.isConnected()) {
        Serial.println("Connected to " + WiFi.SSID());
        Serial.println("IP: " + WiFi.localIP().toString());

    } else {
        Serial.println("Can't connect, enabling AP mode");
        wifi_ap_init();
    }

    configTime(TZ, NTP_SERVER);

    server.start();
}


void loop() {
    server.loop();
    
    decoder.read_nonblock();
}
