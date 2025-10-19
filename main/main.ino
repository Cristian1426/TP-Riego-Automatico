#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifiManager.h"

void taskNet(void *pvParameters) {
  //const char* nombre = (const char*)pvParameters;
  bool estado = false;
  netBegin(); // conecta WiFi y MQTT
  for(;;) {
    mqttLoop(&estado);  // mantiene viva la sesión MQTT
    digitalWrite(2,estado); //LED de prueba de conexion
    mqttPublishPeriodic();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);  // LED en GPIO 2 (la mayoría de los ESP32)
  xTaskCreate(taskNet, "taskNet", 4096, (void*)"Tarea A", 1, NULL);
}

void loop() {
  
}
