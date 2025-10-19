#include <wifiManager.h>
#include <PubSubClient.h>
#include <WiFi.h> 
const char* ssid = "WISP_Fibra_100778";
const char* password = "94573281";
const char* mqtt_server = "192.168.100.30";
unsigned long lastPub = 0;
WiFiClient espClient;
PubSubClient client(espClient);

void netBegin() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  client.setServer(mqtt_server, 1883);
  client.connect("esp32_test");
}

void mqttPublishPeriodic(){
  if (millis() - lastPub >= PERIOD_MS) {
    client.publish("test/topic", "hola desde ESP32");
    lastPub = millis();
  }
}

void mqttLoop() {
  client.loop();
}