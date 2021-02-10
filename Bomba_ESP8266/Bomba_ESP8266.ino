#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>

const uint16_t TIMEOUT = 2000;

WiFiManager wifiManager;

ESP8266WebServer server(30128);

void httpSendContent() {
  
  uint32_t respTime = millis();

  String content = "";

  uint32_t ellapsed = 0;

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

  server.send(200, "text/json\r\nAccess-Control-Allow-Origin: *", content);
  
}

void setup() {

  delay(500);

  wifiManager.setDebugOutput(false); //desabilita prints (nao verbose)

  wifiManager.autoConnect("Bomba CS 25", "geTa4592");

  ArduinoOTA.begin();
  
  server.on("/", []() {
    /*if (!server.authenticate(www_username, www_password)) {
      return server.requestAuthentication();
    }*/
    
    if (server.method() != HTTP_POST) {
      server.send(200, "text/json\r\nAccess-Control-Allow-Origin: *", "{\"error\":\"O metodo deve ser POST.\"}");
      return;
    }
    
    String cmd = server.arg("cmd");
    if (cmd.length() == 0) {
      server.send(200, "text/json\r\nAccess-Control-Allow-Origin: *", "{\"error\":\"Necessario a variavel 'cmd'.\"}");
      return;
    }

    Serial.write(cmd.c_str());
    
    httpSendContent();
    
  });
  
  Serial.begin(9600);
  
  server.begin();

  delay(200);
  
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}
