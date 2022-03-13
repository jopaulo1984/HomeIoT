#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <ArduinoOTA.h>

#define WAIT_CONFIG 0

char wmstate = 0;

String get_mng_html() {
  String html = "<!DOCTYPE html>\n";
  html += "<html lang=\"pt-br\">\n";
  html += "    <head>\n";
  html += "        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
  html += "        <meta charset=\"utf-8\"/>\n";
  html += "        <title>DTS238-7 Medidor - Configuração</title>\n";
  html += "        <style>\n";
  html += "        form {\n";
  html += "            background-color: #adf;\n";
  html += "            padding: 20px;\n";
  html += "        }\n";
  html += "        form input {\n";
  html += "            border: 1px solid #7f7f7f;\n";
  html += "            padding: 5px;\n";
  html += "            border-radius: 5px;\n";
  html += "            font-size: 1em;\n";
  html += "         }\n";
  html += "         .buttons {text-align: right;}\n";
  html += "        </style>\n";
  html += "    </head>\n";
  html += "    <body>\n";
  html += "        <form method=\"post\" action=\"./save_configs\">\n";
  html += "            <h3><center>DTS238-7 Medidor - Configuração</center></h3>\n";
  html += "            <table>\n";
  html += "                <tr><td>SSID</td><td><input name=\"ssid\" type=\"text\"></td></tr>\n";
  html += "                <tr><td>Senha</td><td><input name=\"pass\" type=\"pass\"></td></tr>\n";
  html += "                <tr><td>Broker</td><td><input name=\"broker\" type=\"text\"></td></tr>\n";
  html += "                <tr><td>Porta</td><td><input name=\"port\" type=\"text\"></td></tr>\n";
  html += "                <tr><td>Publish</td><td><input name=\"publish\" type=\"text\"></td></tr>\n";
  html += "                <tr><td>Subscribe</td><td><input name=\"subscribe\" type=\"text\"></td></tr>\n";
  html += "            </table>\n";
  html += "            <div class=\"buttons\"><input type=\"submit\" value=\"Salvar\"></div>\n";
  html += "        </form>\n";
  html += "    </body>\n";
  html += "</html>";
  Serial.println(html);
  return html;
}

WebServer server(80);

void setup() {

  //Iniciando porta serial padrão
  Serial.begin(115200);

  //Iniciando porta serial modbus
  Serial1.begin(9600);

  //Verificando o estado do gerenciador de conexão

  Serial.println("wmstate: " + String(wmstate));
  
  if (wmstate == WAIT_CONFIG) {

    Serial.println("Não há conexão salva.");

    Serial.println("Iniciando wifi no modo AP...");
    WiFi.softAP("DTS238-7 Medidor", "5a91Ty25");
    Serial.println("Wifi no modo AP");
    Serial.println("IP do ponto de acesso: " + WiFi.softAPIP().toString());

    Serial.println("Iniciando servidor de conexão...");
    server.begin();

    Serial.println("Feito!");

    server.on("/", []() {
      Serial.println("Solicitado ao servidor.");
      server.send(200, "text/html", get_mng_html());
    });

    server.on("/save_configs", []() {
      Serial.println("Configurações salvas.");
      server.send(200, "text/html", "<h1>Ok</h1>");
    });

  } else {
    //Há conexão salva
  }

  ArduinoOTA.begin();

}

void loop() {
  ArduinoOTA.handle();
  if (wmstate == WAIT_CONFIG) {
    server.handleClient();
  }
}
