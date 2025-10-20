#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifiManager.h"

//mosquitto_sub -h 192.168.100.30 -t huerta/heartbeat
//mosquitto_pub -h 192.168.100.30 -t huerta/cmd -m "CERRAR"
/*bool estado = false;
void taskNet(void *pvParameters) {
  //const char* nombre = (const char*)pvParameters;
  
  for(;;) {
    mqttLoop(&estado);  // mantiene viva la sesión MQTT
    digitalWrite(2,estado); //LED de prueba de conexion
    mqttPublishPeriodic();
  }
}*/
void taskCommands(void *pvParameters) {
   for (;;) {
    taskMQTT();
   }
}

void taskHeartBeat(void *pvParameters) {
   for (;;) {
    mqttPublishPeriodic();
   }
}

void setup() {
  Serial.begin(115200);
  netBegin(); // conecta WiFi y MQTT
  //pinMode(2, OUTPUT);  // LED en GPIO 2 (la mayoría de los ESP32)
  xTaskCreate(taskCommands, "commands", 4096, (void*)"Tarea A", 1, NULL);
  xTaskCreate(taskHeartBeat, "heartBeat", 4096, (void*)"Tarea A", 2, NULL);

}

void loop() {}
