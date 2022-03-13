
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "ConnMQTTManager.h"
#include "DTS2387.h"
#include "PubSubClient.h"
//#include <time.h>

struct t_energy 
{
    float value;
    String time;
    bool changed;
};

ConnMQTTManager connmqttmng(80, 26);
DTS2387 *dts; //(2, 25, 1, 9600);
WiFiClient wclient;
PubSubClient mqttclient(wclient);
WiFiUDP udp;
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000); //Cria um objeto "NTP" com as configurações.utilizada no Brasil 
String broker = "";
String publish = "";
String subscribe = "";

unsigned int mqtt_port = 1883;
unsigned long lastTime = 0;
unsigned int pub_interval = 2000;

unsigned long now = 0;
unsigned long ellapsed = 0;
unsigned long reconntime = 0;

#define ENERGY_MAX 6

int pubCount = 0;
int energyTimeCount = 0;
int energyCount = 0;
uint reconncount = 0;
float energyLast = 0.0;

/*float energys[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
String times[] = {"", "", "", "", "", ""};*/

t_energy lastEnergy;

void ntp_srtart()
{
    ntp.begin(); //Inicia o NTP.
    Serial.print("Definindo data e hora ");
    while (!ntp.update())
    {
        Serial.print(".");
        ntp.forceUpdate(); //Força o Update.
        delay(500);
    }
    Serial.println("");
}

void load_stories() {}

tm datetime_now()
{
    //Obtendo o tempo atual em segundos. Utilize isso sempre que precisar obter o tempo atual
    time_t tt = ntp.getEpochTime();
    //Convertendo o tempo atual e atribuindo na estrutura
    return *gmtime(&tt);
}

String datetime_formated(tm data)
{
    char data_formatada[64];
    strftime(data_formatada, 64, "%d/%m/%Y %H:%M:%S", &data);
    return String(data_formatada);
}

void mqtt_send_last_hour_energy()
{
    if (!mqttclient.connected()) return;
    String jsvalues = "{\"time\":\"" + lastEnergy.time + "\",\"value\":" + String(lastEnergy.value) + "}";
    mqttclient.publish((publish + "/lasthourenergy").c_str(), (const uint8_t*)jsvalues.c_str(), jsvalues.length(), true);
}

void mqtt_send_energy()
{
    if (!mqttclient.connected()) return;
    //gerando o JSON com as variáveis
    String jsvalues = "{";
    jsvalues += "\"kwh\":"  + String(dts->getEnergy()) + ",";
    jsvalues += "\"rkwh\":" + String(dts->getReverseEnergy()) + ",";
    jsvalues += "\"fkwh\":" + String(dts->getForwordEnergy())/* + ","*/;
    jsvalues += "}";
    mqttclient.publish((publish + "/energy").c_str(), (const uint8_t*)jsvalues.c_str(), jsvalues.length(), true);
}

void mqtt_send_measures()
{
    if (!mqttclient.connected()) return;
    //carregando na memória o buffer enviado pelo DTS238-7...
    if (dts->load() != NOT_ERROR)
    {
        Serial.println("Erro ao carregar dados pela RS-485.");
        return;
    }

    //gerando o JSON com as variáveis
    String jsvalues = "{";
    jsvalues += "\"fr\":"   + String(dts->getFrequency()) + ",";
    jsvalues += "\"va\":"   + String(dts->getVoltage(PHASE_A)) + ",";
    jsvalues += "\"vb\":"   + String(dts->getVoltage(PHASE_B)) + ",";
    jsvalues += "\"vc\":"   + String(dts->getVoltage(PHASE_C)) + ",";
    jsvalues += "\"ia\":"   + String(dts->getCurrent(PHASE_A)) + ",";
    jsvalues += "\"ib\":"   + String(dts->getCurrent(PHASE_B)) + ",";
    jsvalues += "\"ic\":"   + String(dts->getCurrent(PHASE_C));
    jsvalues += "}";

    //necessário particionar para que o envio mqtt possa acontecer
    mqttclient.publish((publish + "/measures").c_str(), (const uint8_t*)jsvalues.c_str(), jsvalues.length(), true);

    jsvalues = "{";
    jsvalues += "\"p\":"    + String(dts->getActivePower()) + ",";
    jsvalues += "\"pa\":"   + String(dts->getActivePower(PHASE_A)) + ",";
    jsvalues += "\"pb\":"   + String(dts->getActivePower(PHASE_B)) + ",";
    jsvalues += "\"pc\":"   + String(dts->getActivePower(PHASE_C)) + ",";
    jsvalues += "\"q\":"    + String(dts->getReactivePower()) + ",";
    jsvalues += "\"qa\":"   + String(dts->getReactivePower(PHASE_A)) + ",";
    jsvalues += "\"qb\":"   + String(dts->getReactivePower(PHASE_B)) + ",";
    jsvalues += "\"qc\":"   + String(dts->getReactivePower(PHASE_C));
    jsvalues += "}";

    mqttclient.publish((publish + "/power1").c_str(), (const uint8_t*)jsvalues.c_str(), jsvalues.length(), true);

    jsvalues = "{";
    jsvalues += "\"n\":"    + String(dts->getApparentPower()) + ",";
    jsvalues += "\"na\":"   + String(dts->getApparentPower(PHASE_A)) + ",";
    jsvalues += "\"nb\":"   + String(dts->getApparentPower(PHASE_B)) + ",";
    jsvalues += "\"nc\":"   + String(dts->getApparentPower(PHASE_C)) + ",";
    jsvalues += "\"fp\":"   + String(dts->getPowerFactor()) + ",";
    jsvalues += "\"fpa\":"  + String(dts->getPowerFactor(PHASE_A)) + ",";
    jsvalues += "\"fpb\":"  + String(dts->getPowerFactor(PHASE_B)) + ",";
    jsvalues += "\"fpc\":"  + String(dts->getPowerFactor(PHASE_C));
    jsvalues += "}";

    mqttclient.publish((publish + "/power2").c_str(), (const uint8_t*)jsvalues.c_str(), jsvalues.length(), true);

}

void mqtt_reconnect()
{
    // Loop until we're reconnected
    /*while (!mqttclient.connected())
    {*/
    // Create a random mqttclient ID
    String clientId = "ESP8266Client_geTa6187";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttclient.connect(clientId.c_str()))
    {
        // Once connected, publish an announcement...
        /*mqtt_send_measures();
        mqtt_send_energy();
        mqtt_send_energys();*/
        // ... and resubscribe
        mqttclient.subscribe(subscribe.c_str());
    }
    else
    {
        // Wait 5 seconds before retrying
        //delay(5000);
        reconncount = 5;
    }
    //}
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{

    String cmd;

    for (int i = 0; i < length; i++)
    {
        cmd += (char)payload[i];
    }

    if (cmd == "RELAY ON")
    {
        dts->setRelayOn(true);
    }
    else if (cmd == "RELAY OFF")
    {
        dts->setRelayOn(false);
    }

    Serial.print("Recebido -> ");
    Serial.print(topic);
    Serial.print(": [ ");
    Serial.print(cmd);
    Serial.println(" ]\n");

}

void setup()
{

    //Iniciando porta serial padrão
    Serial.begin(115200);

    connmqttmng.autoConnect();

    //iniciando data e hora online
    ntp_srtart();

    broker = connmqttmng.getBroker();
    mqtt_port = connmqttmng.getBrokerPort();
    publish = connmqttmng.getPublishTopic();
    subscribe = connmqttmng.getSubscribeTopic();
    pub_interval = connmqttmng.getPublishInterval();

    Serial.println("Iniciando comunicação com o DTS238-7...");
    dts = new DTS2387(2, 25, connmqttmng.getDTSAddress(), connmqttmng.getDTSBaudrate());
    dts->begin();

    if (connmqttmng.getDTSOnRelay())
    {
        dts->setRelayOn(true);
    }

    energyLast = dts->getEnergy();

    lastEnergy.value = 0;
    lastEnergy.time = datetime_formated(datetime_now());
    lastEnergy.changed = false;

    Serial.println("Iniciando cliente MQTT...");
    if (broker.length() > 0 && mqtt_port > 0)
    {
        mqttclient.setServer(broker.c_str(), mqtt_port);
        mqttclient.setCallback(mqtt_callback);
    }
    else
    {
        Serial.println("Servidor broker inválido.");
        while (1 == 1);
    }

    Serial.println("Ok!");

    Serial.println("sincronizando o tempo...");
    unsigned long snow = ntp.getEpochTime();
    while (ntp.getEpochTime() == snow);
    lastTime = millis();

}

void tick1s()
{

    if (pubCount == 0)
    {
        //intervalo para leitura das grandezas elétricas
        mqtt_send_measures();
    }

    if (energyCount % 10 == 0)
    {
        //envia as informações de energia a cada dez segundos
        mqtt_send_energy();
        if (energyCount == 0) //a cada 30 segundos
        {
            //pega as informações do tempo atual
            tm tm_now = datetime_now();
            //se é hora cheia
            if (tm_now.tm_min == 0)
            {
                //se o consumo não foi salvo ainda
                if (!lastEnergy.changed)
                {
                    //leitura do consumo total atual
                    float energy = dts->getEnergy();
                    //obtendo consumo em uma hora
                    lastEnergy.value = energy - energyLast;
                    lastEnergy.time = datetime_formated(tm_now);
                    lastEnergy.changed = true;
                    //salvando a última leitura
                    energyLast = energy;
                }
            }
            else if (lastEnergy.changed)
            {
                lastEnergy.changed = false;
            }
            mqtt_send_last_hour_energy();
        }
    }

    pubCount = (pubCount + 1) % pub_interval;
    energyCount = (energyCount + 1) % 30; //a cada 30 segundos

}

void loop()
{

    if (mqttclient.connected())
    {
        mqttclient.loop();
    }
    else
    {
        if (reconncount > 0)
        {
            now = millis();
            if (now - reconntime > 999)
            {
                reconncount -= 1;
                reconntime = now;
            }
        }
        else
        {
            reconncount = 0;
            reconntime = millis();
            mqtt_reconnect();
        }
    }

    now = millis();
    ellapsed = now - lastTime;

    if (ellapsed > 999)
    {
        tick1s();
        lastTime = now;
    }
    
}
