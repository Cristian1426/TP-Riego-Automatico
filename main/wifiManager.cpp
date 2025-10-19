#include <wifiManager.h>
#include <PubSubClient.h>
#include <WiFi.h> 
const char* ssid = "WISP_Fibra_100778";
const char* password = "94573281";
const char* mqtt_server = "192.168.100.30";
unsigned long lastPub = 0;
WiFiClient espClient;
PubSubClient client(espClient);


void replyMsg(String msg){
  if (msg == "ABRIR") {
      Serial.println("→ Comando: abrir válvula");
      // acá iría digitalWrite(VALVE_PIN, HIGH);
      client.publish("huerta/status", "VALVULA_ABIERTA");
    } 
    else if (msg == "CERRAR") {
      Serial.println("→ Comando: cerrar válvula");
      // acá iría digitalWrite(VALVE_PIN, LOW);
      client.publish("huerta/status", "VALVULA_CERRADA");
    } 
    else {
      client.publish("huerta/status", "COMANDO_DESCONOCIDO");
    }
}

//Funcion de respuesta
void onMqttMsg(char* topic, byte* payload, unsigned int len) {
  // Convertir el payload a texto legible
  String msg;
  msg.reserve(len);                 // usamos len que ya viene del callback
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];

  Serial.print("Tópico: ");
  Serial.println(topic);
  Serial.print("Mensaje: ");
  Serial.println(msg);
  //Si el topico coincide hay respuesta
  if(strcmp(topic, "huerta/cmd") == 0) replyMsg(msg);
}


void netBegin() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  client.setServer(mqtt_server, 1883);
  client.connect("esp32_test");

  client.setCallback(onMqttMsg); // registrar callback
  client.subscribe("huerta/cmd");
}

void taskMQTT(){
  client.loop();                    // procesa mensajes / keep-alive
  vTaskDelay(pdMS_TO_TICKS(20));    // cede CPU (evita WDT)
}



/*
void mqttPublishPeriodic(){
  if (millis() - lastPub >= PERIOD_MS) {
    client.publish("test/topic", "hola desde ESP32");
    lastPub = millis();
  }
}

void mqttLoop(bool *estado) {
  *estado = client.loop();
}
*/