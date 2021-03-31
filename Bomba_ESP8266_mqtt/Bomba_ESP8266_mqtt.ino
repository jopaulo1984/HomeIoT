/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
//#include "FS.h"

const unsigned int TIMEOUT = 2000;

const char* outTopic = "geTa6187/getAbast";
const char* inTopic = "geTa6187/sendAbast";

String mqtt_server = "broker.mqtt-dashboard.com";
int mqtt_port = 1883;

unsigned long lastMsg = 0;

WiFiManager wifiManager;
WiFiClient espclient;
PubSubClient mqttclient(espclient);
ESP8266WebServer server(30128);

String abastSendCommand(const char* cmd) {
  uint32_t respTime = millis();
  String content = "";
  uint32_t ellapsed = 0;
  
  Serial.println(cmd);

  while (1==1) {

    if (ellapsed > TIMEOUT) {
      content = "{\"error\":\"Tempo de resposta esgotado.\"}";
      break;
    }
    
    if (Serial.available() > 0) {

      content += Serial.readString();

      if (content[content.length() - 1] == '\n') {
        break;
      }

      respTime = millis();
      
    }

    ellapsed = millis() - respTime;
    
  }

  return content;
  
}

void sendValues() {
  String response = abastSendCommand("GET INFO");
  mqttclient.publish(outTopic, response.c_str());
}

void callback(char* topic, byte* payload, unsigned int length) {  
  String cmd; // = new char[length];
  String response;
  
  for (int i = 0; i < length; i++) {
    cmd += (char)payload[i];
  }

  response = abastSendCommand(cmd.c_str());
  
  mqttclient.publish(outTopic, response.c_str());
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttclient.connected()) {
    // Create a random mqttclient ID
    String clientId = "ESP8266Client_geTa6187";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttclient.connect(clientId.c_str())) {
      // Once connected, publish an announcement...
      sendValues();
      // ... and resubscribe
      mqttclient.subscribe(inTopic);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

boolean isNumeric(String str) {
    unsigned int stringLength = str.length();
 
    if (stringLength == 0) {
        return false;
    }
 
    boolean seenDecimal = false;
 
    for(unsigned int i = 0; i < stringLength; ++i) {
        if (isDigit(str.charAt(i))) {
            continue;
        }
 
        if (str.charAt(i) == '.') {
            if (seenDecimal) {
                return false;
            }
            seenDecimal = true;
            continue;
        }
        return false;
    }
    return true;
}

void setup() {

    //carregando broker do arquivo
    String dados = "";
    /*File fbrkr = SPIFFS.open("broker.txt", "r");
    if (fbrkr) {
      while (fbrkr.available()) {
        dados += (char)fbrkr.read();
      }
    }
    fbrkr.close();*/

    mqtt_server = "";
    mqtt_port = 0;

    //parser
    String sport = "";
    int estado = 0;
    for (unsigned int i; i < dados.length(); i++) {
        char c = dados.charAt(i);
        switch (estado) {
            case 0:
                if (c == ':') {
                    estado = 1;
                } else {
                    mqtt_server += c;
                }
                break;
            case 1:
                if (isDigit(c)) {
                    sport += c;
                } else {
                    estado = -1;
                }
            default:
                i = dados.length();
        }
    }
    mqtt_port = sport.toInt();

    delay(500);

    wifiManager.setDebugOutput(false); //desabilita prints (nao verbose)

    wifiManager.autoConnect("Bomba CS 25", "geTa4592");

    ArduinoOTA.begin();

    server.on("/", []() {
        
        if (server.method() != HTTP_POST) {
          server.send(200, "text/json\r\nAccess-Control-Allow-Origin: *", "{\"error\":\"O metodo deve ser POST.\"}");
          return;
        }

        String cmd = server.arg("cmd");
        if (cmd.length() == 0) {
          server.send(200, "text/json\r\nAccess-Control-Allow-Origin: *", "{\"error\":\"Falha no comando.\"}");
          return;
        }
        
        String response = abastSendCommand(cmd.c_str());

        server.send(200, "text/json\r\nAccess-Control-Allow-Origin: *", response);
        
    });

    server.on("/mqttbroker", []() {
        /*if (!server.authenticate(www_username, www_password)) {
        return server.requestAuthentication();
        }*/
        
        if (server.method() != HTTP_POST) {
            server.send(200, "text/json\r\nAccess-Control-Allow-Origin: *", "{\"error\":\"O metodo deve ser POST.\"}");
            return;
        }
        
        String oper = server.arg("operation");
        if (oper.length() == 0) {
            server.send(200, "text/json\r\nAccess-Control-Allow-Origin: *", "{\"error\":\"Falha no comando.\"}");
            return;
        } else if (oper == "get") {
            server.send(200, "text/json\r\nAccess-Control-Allow-Origin: *", "{\"address\":\"" + mqtt_server + "\",\"port\":" + mqtt_port + "}");
        } else if (oper == "set") {
            String addr = server.arg("address");
            int port = server.arg("port").toInt();
            if (addr.length() == 0 || port == 0) {
                server.send(200, "text/json\r\nAccess-Control-Allow-Origin: *", "{\"error\":\"Dados inválidos.\"}");
                return;
            }
            mqtt_server = addr;
            mqtt_port = port;

            //salvando na eeprom
            /*File fbrkr = SPIFFS.open("broker.txt", "w");
            fbrkr.write(addr + ":" + port);
            fbrkr.close();*/

            //ESP.restart();

            server.send(200, "text/json\r\nAccess-Control-Allow-Origin: *", "null");

            mqttclient.setServer(mqtt_server.c_str(), mqtt_port);

        }
        
    });

    Serial.begin(115200);

    server.begin();

    if (mqtt_server.length() > 0 && mqtt_port > 0) {
      mqttclient.setServer(mqtt_server.c_str(), mqtt_port);      
      mqttclient.setCallback(callback);
    }

    delay(200);

}

void loop() {
  if (mqtt_server.length() > 0 && mqtt_port > 0) {
    if (!mqttclient.connected()) {
      reconnect();
    }
    mqttclient.loop();
  
    //publica a cada 2s
    unsigned long now = millis();
    if ((now - lastMsg) > 2000) {
      lastMsg = now;
      sendValues();
    }
  }
}
