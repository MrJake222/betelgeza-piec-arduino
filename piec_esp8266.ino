#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <SoftwareSerial.h>
#include <SimpleFTPServer.h>

#include "helpers.hpp"
#include "server.hpp"
#include "proto_decoder.hpp"

#include "config.h"

#define NTP_SERVER "pl.pool.ntp.org"           
#define TZ "CET-1CEST,M3.5.0,M10.5.0/3" 

SoftwareSerial softSerial;

void serial_init() {
    Serial.begin(115200);
    Serial.println();
}

void soft_serial_init() {
    
    // Invert - 0
    // ISR buf - 0 (auto)
    softSerial.begin(9600, SWSERIAL_8N1, SOFT_RX, SOFT_TX, 0, mrjake::ProtoDecoder::get_buf_size(), 0);

    digitalWrite(SOFT_TX, LOW);

    if (!softSerial) {
        Serial.println("softSerial init failed");
        while (1) delay(1000);
    }

    softSerial.setTimeout(1000);
    Serial.println("softSerial OK");
}

bool fs_init() {
    bool ok = LittleFS.begin();
    if (!ok) {
        Serial.println("failed to setup FS");
        while (1) delay(1000);
        return false;
    }

    Serial.println("FS init ok");
    
    fs::FSInfo i;
    LittleFS.info(i);
    Serial.printf("Used: %d / %d bytes (free %d bytes)\n", i.usedBytes, i.totalBytes, i.totalBytes - i.usedBytes);
    
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
        while (1) delay(1000);
        return false;
    }

    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    return true;
}

FtpServer ftpSrv;

void ftp_srv_init() {
    ftpSrv.begin("norbert", "marcel!!");
}

void config_changed() {
    ftpSrv.end();
    ftp_srv_init();
}

mrjake::ProtoDecoder decoder(softSerial);
mrjake::Server server(80, decoder, config_changed);

void setup() {
    serial_init();
    soft_serial_init();

    fs_init();
    ftp_srv_init();

    // Status led - low turns on
    pinMode(P_STATUS_LED, OUTPUT);
    digitalWrite(P_STATUS_LED, HIGH);

    // Circulation pump - high turns on
    pinMode(P_PUMP, OUTPUT);
    digitalWrite(P_PUMP, LOW);

    // Input (inverted)
    pinMode(P_IN, INPUT);

    WiFi.begin();

    for (int i=0; i<20 && !WiFi.isConnected(); i++) {
        Serial.print("Connection attempt ");
        Serial.print(i+1);
        Serial.println("/20");
        delay(900);
        digitalWrite(P_STATUS_LED, LOW);
        delay(100);
        digitalWrite(P_STATUS_LED, HIGH);
    }

    if (WiFi.isConnected()) {
        Serial.println("Connected to " + WiFi.SSID());
        Serial.println("IP: " + WiFi.localIP().toString());

    } else {
        Serial.println("Can't connect, enabling AP mode");
        digitalWrite(P_STATUS_LED, LOW);
        wifi_ap_init();
    }

    configTime(TZ, NTP_SERVER);

    server.start();
}

long last_print = 0;
long maxm = 0;

void loop() {

    long start = millis();

    server.loop();
    ftpSrv.handleFTP();
    
    decoder.read_nonblock();

    long stop = millis();
    if ((stop - start) > maxm) {
        maxm = stop - start;
    }

    if (millis() - last_print > 10000) {
        Serial.printf("  max loop time: %d ms\n", maxm);
        maxm = 0;
        last_print = millis();
    }
}
