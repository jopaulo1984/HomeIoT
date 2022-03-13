#ifndef CONNMQTTMANAGER_H
#define CONNMQTTMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include "FS.h"
#include "SPIFFS.h"

#define WAIT_CONFIG 0
#define CONFIG_SAVED 1

class ConnMQTTManager {
private:
    //unsigned int __ap_port = 80;

    WebServer *server;

    String wifi_ssid = "";
    String wifi_pass = "";
    String broker = "";
    String publish = "";
    String subscribe = "";

    int dts_address = 1;
    unsigned int dts_baudrate = 9600;
    int dts_on_relay = 0;
    unsigned int pub_interval = 2;
    unsigned int wmstate = 0;
    unsigned int mqtt_port = 1883;

    uint8_t __reset_pin = 26;

    void load_configs();
    void save_configs();
    void restart();
    void setup_wifi();
    void configs_server();
    void handle_root();
    void handle_save_configs();

    int db_to_percent(int db);

    String get_mng_html();
    String html_message(String msg);

public:
    ConnMQTTManager();
    ConnMQTTManager(int ap_port);
    ConnMQTTManager(int ap_port, int reset_pin);
    ~ConnMQTTManager();

    void autoConnect();

    String getBroker();
    String getPublishTopic();
    String getSubscribeTopic();

    int getBrokerPort();
    unsigned int getPublishInterval();
    int getDTSAddress();
    bool getDTSOnRelay();
    unsigned int getDTSBaudrate();

};

#endif