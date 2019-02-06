/*
* Arduino Wireless Communication Tutorial
*     Example 1 - Transmitter Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "src/Nrf/Nrf.h"

const byte address[][13] = {"00001", "00002"};
const char *ssid = "Niels";
const char *password = "cdh@1686!";
const char *mqtt_server = "178.128.254.40";

RF24 rfradio(0, 2); // CE, CSN
Nrf nrf(&rfradio);
WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
    Serial.begin(115200);

    // Setup Wifi
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    // Setup NRF
    nrf.radio->begin();
    nrf.radio->setRetries(5, 15);
    nrf.radio->enableDynamicPayloads();

    // Do not use 0 as reading pipe! This pipe is already in use ase writing pipe
    nrf.radio->openWritingPipe(address[0]);
    nrf.radio->openReadingPipe(1, address[1]);
    nrf.radio->startListening();

    // Setup MQTT client
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }

    client.loop();
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("[");
    Serial.print(topic);
    Serial.print("] ");

    char movements[length * sizeof(char)];
    int num = 1;

    for (int i = 0; i < length; i++)
    {
        char temp = (char)payload[i];

        if (temp == ';')
        {
            num = num + 1;
        }

        movements[i] = temp;
    }

    for (int i = 0; i < 2; i++)
    {
        Serial.println(i);
        String strMovement = split(movements, ';', i);

        char movement[sizeof(strMovement)] = "";
        strMovement.toCharArray(movement, sizeof(strMovement));

        nrf.sendMessage(movement, 32);

        bool done = false;
        char response[32] = "";

        // Send movements
        unsigned long started_waiting_at = millis();
        bool timeout = false;

        while (!done && !timeout)
        {
            nrf.readMessage(response);

            if (strlen(response) > 0)
            {
                done = true;
            }

            if (millis() - started_waiting_at > 10000)
            {
                timeout = true;
            }

            yield();
        }

        Serial.println(response);
        client.publish("table/status", movement);
    }
}

String split(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void reconnect()
{
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        String clientId = "vroom-cc";

        // Attempt to connect
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected");
            client.subscribe("table/movement");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}
