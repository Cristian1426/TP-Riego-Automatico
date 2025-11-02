#include <Arduino.h>
#include "config.h"
#include "wifiManager.h"

// Simulación de relé con el LED
static volatile bool g_activo = false;

// Handle de la tarea watchdog
static TaskHandle_t g_watchdogTask = nullptr;

// ---- Acciones simuladas ----
static inline void actuadorSeguro()   { digitalWrite(LED, LOW);  g_activo = false; digitalWrite(RELAY_PIN, RELAY_SAFE_ON); }
static inline void actuadorActivo()   { digitalWrite(LED, HIGH); g_activo = true;  digitalWrite(RELAY_PIN, RELAY_ACTIVE_ON); }

// ---- Hook: se llama después de publicar un heartbeat exitoso ----
void onHeartbeatOk() {
  BaseType_t xHigher = pdFALSE;
  if (g_watchdogTask) xTaskNotifyGiveIndexed(g_watchdogTask, 0);  // “patada”
  portYIELD_FROM_ISR(xHigher);
}

// ---- Tarea Watchdog ----
static void taskWatchdog(void* pv) {
  (void)pv;
  actuadorSeguro();  // arranque seguro

  for (;;) {
    uint32_t got = ulTaskNotifyTakeIndexed(0, pdTRUE, pdMS_TO_TICKS(WATCHDOG_TIMEOUT));
    if (got == 0) {
      // Timeout → modo seguro
      actuadorSeguro();
      mqttPublish(T_STATUS, "FAILSAFE", true);
    }
  }
}


// -----------------------------------------------------------------------------
// setup: se ejecuta una sola vez al encender o resetear el ESP32
// -----------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);
  delay(300);
  Serial.println("\nESP32 Encendido!");

  netBegin();  // Crea la tarea MQTT y arranca WiFi/MQTT

  xTaskCreate(taskWatchdog, "watchdog", 3072, nullptr, 4, &g_watchdogTask);
}

void loop() {

  delay(10);
}
