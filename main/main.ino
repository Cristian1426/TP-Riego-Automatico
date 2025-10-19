#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifiManager.h"


void setup() {
  Serial.begin(115200);
  netBegin(); // conecta WiFi y MQTT
}

void loop() {
  mqttLoop();  // mantiene viva la sesión MQTT
  mqttPublishPeriodic();
}
