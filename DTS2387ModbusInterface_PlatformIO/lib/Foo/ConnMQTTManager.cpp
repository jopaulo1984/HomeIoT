
#include "ConnMQTTManager.h"

//  privates methods and functions //
void ConnMQTTManager::load_configs()
{

    Serial.println("carregando o arquivo de configuração");

    File f = SPIFFS.open("/configs.txt", "r");

    if (!f)
    {
        Serial.println("Erro ao iniciar o arquivo de configuração.");
        while (1 == 1)
            ; //loop infinito. necessário reset manual
    }

    wmstate = f.readStringUntil('\n').toInt();
    wifi_ssid = f.readStringUntil('\n');
    wifi_pass = f.readStringUntil('\n');
    broker = f.readStringUntil('\n');
    mqtt_port = f.readStringUntil('\n').toInt();
    publish = f.readStringUntil('\n');
    subscribe = f.readStringUntil('\n');
    pub_interval = f.readStringUntil('\n').toInt();
    dts_address = f.readStringUntil('\n').toInt();
    dts_baudrate = f.readStringUntil('\n').toInt();
    dts_on_relay = f.readStringUntil('\n').toInt();

    f.close();

    //removendo espaços no começo e fim da string
    wifi_ssid.trim();
    wifi_pass.trim();
    broker.trim();
    publish.trim();
    subscribe.trim();

    Serial.println("wmstate: " + String(wmstate));
    Serial.println("wifi_ssid: " + wifi_ssid);
    Serial.println("wifi_pass: " + wifi_pass);
    Serial.println("broker: " + broker);
    Serial.println("mqtt_port: " + String(mqtt_port));
    Serial.println("publish: " + publish);
    Serial.println("subscribe: " + subscribe);
    Serial.println("pub_interval: " + String(pub_interval));
    Serial.println("dts_address: " + String(dts_address));
    Serial.println("dts_baudrate: " + String(dts_baudrate));
    Serial.println("dts_on_relay: " + String(dts_on_relay > 0));
    Serial.println("Feito!");
    
}

void ConnMQTTManager::save_configs()
{
    Serial.println("Salvando configurações...");
    File f = SPIFFS.open("/configs.txt", "w");
    if (!f)
    {
        Serial.println("Erro ao salvar o arquivo de configuração.");
        return; //loop infinito
    }
    f.println(wmstate);
    f.println(wifi_ssid);
    f.println(wifi_pass);
    f.println(broker);
    f.println(mqtt_port);
    f.println(publish);
    f.println(subscribe);
    f.println(pub_interval);
    f.println(dts_address);
    f.println(dts_baudrate);
    f.println(dts_on_relay);
    f.close();
    Serial.println("Configurações salvas.");
}

void ConnMQTTManager::restart()
{
    WiFi.disconnect();
    Serial.println("Reiniciando...");
    delay(2000);
    ESP.restart();
    while (1 == 1);
}

void ConnMQTTManager::setup_wifi()
{

    Serial.println("Iniciando o wifi...");
    Serial.print("Conectando");

    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());

    unsigned long lastTime = millis();
    while (1 == 1)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            break;
        }
        else if (WiFi.status() == WL_CONNECT_FAILED)
        {
            Serial.println("Ocorreu um erro ao tentar conectar.");
            restart();
        }
        else if (millis() - lastTime > 60000)
        {
            Serial.println("Tempo esgotado.");
            restart();
        }
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());
    Serial.println("\nConectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void ConnMQTTManager::configs_server()
{

    Serial.println("Não há conexão salva.");

    Serial.println("Iniciando wifi no modo AP...");
    WiFi.softAP("DTS238-7 Medidor", "5a91Ty25");
    Serial.println("Wifi no modo AP");
    Serial.println("IP do ponto de acesso: " + WiFi.softAPIP().toString());

    Serial.println("Iniciando servidor de conexão...");
    server->begin();

    Serial.println("Feito!");

    server->on("/", std::bind(&ConnMQTTManager::handle_root, this));

    server->on("/save_configs", std::bind(&ConnMQTTManager::handle_save_configs, this));

    ArduinoOTA.begin();

    while (1 == 1)
    {
        ArduinoOTA.handle();
        server->handleClient();
    }
}

void ConnMQTTManager::handle_root()
{
    Serial.println("Solicitado ao servidor.");
    server->send(200, "text/html", get_mng_html());
}

void ConnMQTTManager::handle_save_configs()
{
    if (server->method() != HTTP_POST)
    {
        server->send(200, "text/json\r\nAccess-Control-Allow-Origin: *", html_message("O metodo deve ser POST."));
        return;
    }

    wifi_ssid = server->arg("ssid");
    if (wifi_ssid.length() == 0)
    {
        server->send(200, "text/json\r\nAccess-Control-Allow-Origin: *", html_message("Necessário informar o SSID do WIFI."));
        return;
    }

    wifi_pass = server->arg("pass");
    if (wifi_pass.length() == 0)
    {
        server->send(200, "text/json\r\nAccess-Control-Allow-Origin: *", html_message("Necessário informar a senha do WIFI."));
        return;
    }

    broker = server->arg("broker");
    if (broker.length() == 0)
    {
        server->send(200, "text/json\r\nAccess-Control-Allow-Origin: *", html_message("Necessário informar o broker."));
        return;
    }

    String smqtt_port = server->arg("mqtt_port");
    if (smqtt_port.length() == 0)
    {
        server->send(200, "text/json\r\nAccess-Control-Allow-Origin: *", html_message("Necessário informar a senha a porta MQTT."));
        return;
    }

    publish = server->arg("publish");
    if (publish.length() == 0)
    {
        server->send(200, "text/json\r\nAccess-Control-Allow-Origin: *", html_message("Necessário informar o campo 'Publish'."));
        return;
    }

    subscribe = server->arg("subscribe");
    if (subscribe.length() == 0)
    {
        server->send(200, "text/json\r\nAccess-Control-Allow-Origin: *", html_message("Necessário informar o campo 'Subscribe'."));
        return;
    }

    String spub_interval = server->arg("pub_interval");
    if (spub_interval.length() == 0)
    {
        spub_interval = "2";
    }

    String sdts_address = server->arg("dts_address");
    if (sdts_address.length() == 0)
    {
        sdts_address = "1";
    }

    String sdts_baudrate = server->arg("dts_baudrate");
    if (sdts_baudrate.length() == 0)
    {
        sdts_baudrate = "9600";
    }

    String sdts_on_relay = server->arg("dts_on_relay");
    if (sdts_on_relay.length() == 0)
    {
        sdts_on_relay = "0";
    }

    mqtt_port = smqtt_port.toInt();
    pub_interval = spub_interval.toInt();
    dts_address = sdts_address.toInt();
    dts_baudrate = sdts_baudrate.toInt();
    dts_on_relay = sdts_on_relay.toInt();

    wmstate = CONFIG_SAVED;

    save_configs();

    server->send(200, "text/html", "<h1>Salvo com sucesso!<br />Reniniciando sistema...</h1>");

    restart();
}

int ConnMQTTManager::db_to_percent(int db)
{
    if (db < -100)
    {
        return 0;
    }
    if (db >= -50)
    {
        return 100;
    }
    return 2 * (db + 100);
}

String ConnMQTTManager::get_mng_html()
{
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
    html += "        input, select {\n";
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
    html += "                <tr><td colspan=\"2\"><center>-----  WiFi -----</center></td></tr>";
    html += "                <tr>\n";
    html += "                    <td>SSID</td>\n";
    html += "                    <td>\n";
    html += "                        <select name=\"ssid\" value=\"" + wifi_ssid + "\" required>";

    int ssids_count = WiFi.scanNetworks();
    for (int i = 0; i < ssids_count; i++)
    {
        String __ssid = WiFi.SSID(i);
        int __sign = WiFi.RSSI(i);
        html += "                            <option value=\"" + __ssid + "\">" + __ssid + " | " + String(db_to_percent(__sign)) + "%</option>\n";
    }

    html += "                        </select>\n";
    html += "                    </td>\n";
    html += "                </tr>\n";
    html += "                <tr><td>Senha</td><td><input name=\"pass\" type=\"password\" value=\"" + wifi_pass + "\" required></td></tr>\n";
    html += "                <tr><td colspan=\"2\"><center>-----  MQTT -----</center></td></tr>";
    html += "                <tr><td>Broker</td><td><input name=\"broker\" type=\"text\" value=\"" + broker + "\" required></td></tr>\n";
    html += "                <tr><td>Porta</td><td><input name=\"mqtt_port\" type=\"text\" value=\"" + String(mqtt_port) + "\" required pattern=\"\\d+\"></td></tr>\n";
    html += "                <tr><td>Publish</td><td><input name=\"publish\" type=\"text\" value=\"" + publish + "\" required></td></tr>\n";
    html += "                <tr><td>Subscribe</td><td><input name=\"subscribe\" type=\"text\" value=\"" + subscribe + "\" required></td></tr>\n";
    html += "                <tr><td>Intervalo do Publish (s)</td><td><input name=\"pub_interval\" type=\"number\" value=\"" + String(pub_interval) + "\" required></td></tr>\n";
    html += "                <tr><td colspan=\"2\"><center>-----  Medidor -----</center></td></tr>";
    html += "                <tr><td>Endereço</td><td><input name=\"dts_address\" type=\"number\" value=\"" + String(dts_address) + "\" required></td></tr>\n";
    html += "                <tr>\n";
    html += "                    <td>Velocidade (bps)</td>\n";
    html += "                    <td>\n";
    html += "                        <select name=\"dts_baudrate\" value=\"" + String(dts_baudrate) + "\" required>";
    html += "                            <option value=\"9600\">9600</option>\n";
    html += "                            <option value=\"4800\">4800</option>\n";
    html += "                            <option value=\"2400\">2400</option>\n";
    html += "                            <option value=\"1200\">1200</option>\n";
    html += "                        </select>\n";
    html += "                    </td>\n";
    html += "                </tr>\n";
    html += "                <tr>\n";
    html += "                    <td>Ligar ao iniciar</td>\n";
    html += "                    <td>\n";
    html += "                        <select name=\"dts_on_relay\" value=\"" + String(dts_on_relay) + "\" required>";
    html += "                            <option value=\"0\">Não</option>\n";
    html += "                            <option value=\"1\">Sim</option>\n";
    html += "                        </select>\n";
    html += "                    </td>\n";
    html += "                </tr>\n";
    html += "            </table>\n";
    html += "            <div class=\"buttons\"><input type=\"submit\" value=\"Salvar\"></div>\n";
    html += "        </form>\n";
    html += "    </body>\n";
    html += "</html>";
    return html;
}

String ConnMQTTManager::html_message(String msg)
{
    String html = "<!DOCTYPE html>\n";
    html += "<html>\n";
    html += "    <head>\n";
    html += "        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    html += "        <meta charset=\"utf-8\"/>\n";
    html += "        <title>DTS238-7 Medidor - Configuração</title>\n";
    html += "        <style>\n";
    html += "        msg-box {\n";
    html += "            background-color: #adf;\n";
    html += "            padding: 20px;\n";
    html += "        }\n";
    html += "        </style>\n";
    html += "    </head>\n";
    html += "    <body>\n";
    html += "        <pre class\"msg-box\">";
    html += msg;
    html += "</pre>\n";
    html += "    </body>\n";
    html += "</html>";
    return html;
}

//  publics methods and functions //
ConnMQTTManager::ConnMQTTManager()
{
    ConnMQTTManager(80, 26);
}

ConnMQTTManager::ConnMQTTManager(int ap_port)
{
    ConnMQTTManager(ap_port, 26);
}

ConnMQTTManager::ConnMQTTManager(int ap_port, int reset_pin)
{
    //Iniciando pino restauração modo AP
    pinMode(reset_pin, INPUT_PULLUP);
    server = new WebServer(ap_port);
}

ConnMQTTManager::~ConnMQTTManager()
{
    WiFi.disconnect();
}

void ConnMQTTManager::autoConnect()
{

    //iniciando sistema de arquivos
    if (!SPIFFS.begin())
    {
        Serial.println("Erro ao iniciar sistema de arquivos.");
        restart();
    }

    //carregando configrações do arquivo
    load_configs();

    //Verificando se restaura modo AP
    if (digitalRead(__reset_pin) == 0)
    {
        //restaurando modo AP
        wmstate = WAIT_CONFIG;
        save_configs();
        delay(2000);
        while (digitalRead(__reset_pin) == 0)
            ;
    }

    //Verificando o estado do gerenciador de conexão

    if (wmstate != CONFIG_SAVED)
    {                     //modo AP
        configs_server(); //iniciando servidor de configuração
    }
    else
    {                 //modo STATION
        setup_wifi(); //iniciando o wifi
    }
}

String ConnMQTTManager::getBroker()
{
    return String(broker.c_str());
}

String ConnMQTTManager::getPublishTopic()
{
    return String(publish.c_str());
}

String ConnMQTTManager::getSubscribeTopic()
{
    return String(subscribe.c_str());
}

int ConnMQTTManager::getBrokerPort()
{
    return mqtt_port;
}

unsigned int ConnMQTTManager::getPublishInterval()
{
    return pub_interval;
}

int ConnMQTTManager::getDTSAddress()
{
    return dts_address;
}

bool ConnMQTTManager::getDTSOnRelay()
{
    return dts_on_relay > 0;
}

unsigned int ConnMQTTManager::getDTSBaudrate()
{
    return dts_baudrate;
}
