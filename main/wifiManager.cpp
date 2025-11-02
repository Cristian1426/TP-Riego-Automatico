#include "wifiManager.h"
#include "config.h"

#include <WiFi.h>
#include <PubSubClient.h>

// === Estado global privado del módulo ===
static WiFiClient      s_wifi;
static PubSubClient    s_client(s_wifi);
static TaskHandle_t    s_mqttTaskHandle = nullptr;

static unsigned long   s_lastHB = 0;
static bool            s_ledState = false;

// Prototipos
static void ensureWifi();
static void ensureMqtt();
static void mqttCallback(char* topic, byte* payload, unsigned int len);
static void taskMqtt(void*);


// -----------------------------------------------------------------------------
// chipId: arma un clientID unico usando la efuse MAC (evita colisiones en el broker)
// -----------------------------------------------------------------------------
static String chipId() {
  uint64_t mac = ESP.getEfuseMac();
  char buf[17];
  snprintf(buf, sizeof(buf), "%04X%08X", (uint16_t)(mac>>32), (uint32_t)mac);
  return String(buf);
}

// ============ API pública ============

void netBegin() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // >>> INICIALIZACIÓN DEL RELÉ <<<
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_SAFE_ON);

  // Config WiFi y Modo estacion
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Config broker y registra el callback
  s_client.setServer(MQTT_HOST, MQTT_PORT);
  s_client.setCallback(mqttCallback);

  // Arranca tarea MQTT (core libre, prioridad media)
  if (!s_mqttTaskHandle) {
    xTaskCreate(
      taskMqtt,
      "mqttService",
      4096,           // stack
      nullptr,
      2,              // prioridad
      &s_mqttTaskHandle
    );
  }
}

// -----------------------------------------------------------------------------
// mqttConnected: devuelve TRUE si hay sesion MQTT activa 
// -----------------------------------------------------------------------------
bool mqttConnected() {
  return s_client.connected();
}

// -----------------------------------------------------------------------------
// mqttPublish: publica en un topic solo si hay sesion activa
// -----------------------------------------------------------------------------
bool mqttPublish(const char* topic, const char* payload, bool retained) {
  if (!s_client.connected()) return false;
  return s_client.publish(topic, payload, retained);
}


// -----------------------------------------------------------------------------
// mqttPublish: asegura la conexion al WiFi, MQTT y publica el heartbeat
// -----------------------------------------------------------------------------
void mqttServiceOnce() {
  ensureWifi();
  ensureMqtt();
  s_client.loop();

  // Heartbeat cada PERIOD_HB_MS
  unsigned long now = millis();
  if (now - s_lastHB >= PERIOD_HB_MS) {
    s_lastHB = now;
    if (s_client.publish(T_HEARTBEAT, "alive")) {
      onHeartbeatOk();
    }
  }
}

// ============ Internas ============

static void ensureWifi() {
  if (WiFi.status() == WL_CONNECTED) return;

  // Intento de conexion
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

static void ensureMqtt() {
  if (s_client.connected()) return;

  // Si el ESP32 desaparece el broker publica offline
  String cid = "esp32-" + chipId();
  const char* willTopic = T_STATUS;
  const char* willMsg   = "offline";
  int  willQos          = 0;
  bool willRetain       = true;

  while (!s_client.connected()) {
    ensureWifi();
    if (WiFi.status() != WL_CONNECTED) {
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }

    if (s_client.connect(cid.c_str(), nullptr, nullptr, willTopic, willQos, willRetain, willMsg)) {
      // Si conecta
      s_client.publish(T_STATUS, "online", true);
      s_client.subscribe(T_CMD, 0);
    } else {
      // Espera y reintenta
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

static void mqttCallback(char* topic, byte* payload, unsigned int len) {
  String msg; msg.reserve(len);
  for (unsigned i = 0; i < len; ++i) msg += (char)payload[i];
  msg.trim();

  if (strcmp(topic, T_CMD) == 0) {
    String up = msg; up.toUpperCase();
    if (up == "ABRIR") {
      s_ledState = true;
      digitalWrite(LED, HIGH);
        digitalWrite(RELAY_PIN, RELAY_ACTIVE_ON);   // activar relé
      s_client.publish(T_STATUS, "VALVULA_ABIERTA", true);
    } else if (up == "CERRAR") {
      s_ledState = false;
      digitalWrite(LED, LOW);
        digitalWrite(RELAY_PIN, RELAY_SAFE_ON);     // estado seguro
      s_client.publish(T_STATUS, "VALVULA_CERRADA", true);
    } else {
      s_client.publish(T_STATUS, "CMD:UNKNOWN", false);
    }
    handleCommand(msg);
  }
}

static void taskMqtt(void*) {
  for (;;) {
    mqttServiceOnce();
    vTaskDelay(pdMS_TO_TICKS(20)); // ceder CPU, evita WDT
  }
}

// ============ Hooks opcionales ============

void handleCommand(const String& msg) {
  // Punto de extensión: acá podés enganchar tu lógica sin tocar el core.
  // Ejemplo: Serial log corto
  Serial.print("[CMD] "); Serial.println(msg);
}
