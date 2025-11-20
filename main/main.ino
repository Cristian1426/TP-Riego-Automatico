#include <Arduino.h>
#include "config.h"
#include "wifiManager.h"

// =============================================================================
// ARQUITECTURA DE SOBERANÍA DEL WATCHDOG
// =============================================================================
// El watchdog debe ser SOBERANO: ejecutarse sin importar el estado de WiFi/MQTT
// 
// Estrategia implementada:
// 1. CORE DEDICADO: Watchdog pinned a Core 1 (WiFi/MQTT en Core 0)
// 2. PRIORIDAD MÁXIMA: Watchdog prio=4 > MQTT prio=3 (preempts si necesario)
// 3. PERÍODO DETERMINISTA: Usa vTaskDelayUntil() para garantizar ejecución periódica
// 4. ACCESO ATÓMICO: lastHeartbeatTick es volatile (en próxima fase: mutex)
// 5. LOGGING: Reporta periódicamente que está ejecutando
//
// Resultado: El watchdog puede cerrar la válvula incluso si:
// - WiFi se desconecta
// - MQTT intenta reconectar (bloqueante)
// - El Core 0 está en deep-sleep
// =============================================================================

// --- Estado del rele ---
static volatile bool rele_activo = false;

// --- Handle de la tarea watchdog ---
static TaskHandle_t watchdogTask = nullptr;

// Tick del ultimo heartbeat recibido desde la BASE
// PROTEGIDO POR MUTEX para evitar race conditions entre MQTT (Core 0) y Watchdog (Core 1)
static TickType_t lastHeartbeatTick = 0;
static SemaphoreHandle_t heartbeatMutex = nullptr;

// ---- Acciones simuladas ----
static inline void actuadorSeguro() {
  digitalWrite(LED, LOW);
  rele_activo = false;
  digitalWrite(RELAY_PIN, RELAY_SAFE_ON);
}

static inline void actuadorActivo() {
  digitalWrite(LED, HIGH);
  rele_activo = true;
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_ON);
}


// --- Hook que se llama cuando LLEGA un heartbeat valido (Desde BASE al ESP32 via MQTT)
void onHeartbeatOk() {
  if (xSemaphoreTake(heartbeatMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    lastHeartbeatTick = xTaskGetTickCount();
    xSemaphoreGive(heartbeatMutex);
  } else {
    // Timeout esperando el mutex (no debería pasar, pero log por si acaso)
    Serial.println("[HB] Advertencia: timeout adquiriendo mutex");
  }
}

// =================================================
//  Hook para comandos de debugging
//  wifiManager.cpp llama a esto cuando llega T_DEBUG
// =================================================
void handleDebugCommand(const String& msg) {
  Serial.print("[DEBUG] Comando recibido: ");
  Serial.println(msg);

  if (msg.equalsIgnoreCase("PAUSE_HB")) {
    // Simula pérdida de conexión forzando el timestamp del heartbeat a 0
    // El watchdog detectará el timeout y cerrará la válvula después de 5 segundos
    if (xSemaphoreTake(heartbeatMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
      lastHeartbeatTick = 0;  // Fuerza timeout inmediato
      xSemaphoreGive(heartbeatMutex);
    }
    Serial.println("[DEBUG] ⏸ Simulando pérdida de heartbeat - watchdog activará en 5s");
    mqttPublish(T_STATUS, "DEBUG: Heartbeat timestamp reiniciado (timeout en 5s)", true);
  }
  else if (msg.equalsIgnoreCase("RESUME_HB")) {
    // Restaura el timestamp del heartbeat como si acabara de llegar uno
    // Esto evita que el watchdog actúe inmediatamente
    onHeartbeatOk();
    Serial.println("[DEBUG] ✅ Heartbeat timestamp restaurado");
    mqttPublish(T_STATUS, "DEBUG: Timestamp de heartbeat actualizado", true);
  }
  else if (msg.equalsIgnoreCase("STATS")) {
    // Reporta estadísticas del watchdog
    Serial.println("[DEBUG] Estadísticas solicitadas");
    mqttPublish(T_STATUS, "DEBUG: Stats - core watchdog=1, core mqtt=0, timeout=5s, heartbeat=1s", true);
  }
  else {
    Serial.println("[DEBUG] Comando desconocido");
  }
}

// =================================================
//  Hook para comandos MQTT ("ABRIR"/"CERRAR")
//  wifiManager.cpp llama a esto cuando llega T_CMD
// =================================================
void handleCommand(const String& msg) {
  Serial.print("[CMD] Mensaje recibido: ");
  Serial.println(msg);

  if (msg.equalsIgnoreCase("ABRIR")) {
    actuadorActivo();
    mqttPublish(T_STATUS, "VALVULA ABIERTA", true);
  } else if (msg.equalsIgnoreCase("CERRAR")) {
    actuadorSeguro();
    mqttPublish(T_STATUS, "VALVULA CERRADA", true);
  } else {
    Serial.println("[CMD] Comando desconocido");
  }
}

// --- Tarea Watchdog ---
void taskWatchdog(void *pv) {
  const TickType_t checkPeriod = pdMS_TO_TICKS(100); // chequeo cada 100 ms
  const TickType_t timeoutTicks = pdMS_TO_TICKS(WATCHDOG_TIMEOUT);

  // Arranca en estado seguro
  actuadorSeguro();
  
  // Inicializar el mutex (si no está inicializado)
  // El mutex del heartbeat se crea en setup() para evitar races con MQTT.
  
  // Establecer primer heartbeat
  if (xSemaphoreTake(heartbeatMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    lastHeartbeatTick = xTaskGetTickCount();
    xSemaphoreGive(heartbeatMutex);
  }

  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint32_t cycleCount = 0;
  TickType_t lastLogTick = xLastWakeTime;

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, checkPeriod);

    TickType_t now = xTaskGetTickCount();
    TickType_t elapsed = 0;
    
    // Leer heartbeat con protección de mutex
    if (xSemaphoreTake(heartbeatMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
      elapsed = now - lastHeartbeatTick;
      xSemaphoreGive(heartbeatMutex);
    } else {
      // Si no se puede adquirir mutex, asumir timeout (FAIL-SAFE)
      elapsed = timeoutTicks + pdMS_TO_TICKS(1000);
      Serial.println("[WD] Advertencia: timeout adquiriendo mutex, asumiendo timeout fail-safe");
    }
    
    cycleCount++;

    // Logging cada 5 segundos (50 ciclos de 100ms)
    if (now - lastLogTick > pdMS_TO_TICKS(5000)) {
      Serial.printf("[WD] Ejecutando determinísticamente. Ciclos: %lu, Ticks sin HB: %lu / %lu, Core: %d\n",
        cycleCount,
        elapsed,
        timeoutTicks,
        xPortGetCoreID()
      );
      lastLogTick = now;
      cycleCount = 0;
    }

    if (elapsed > timeoutTicks) {
      if (rele_activo) {
        Serial.println("[WD] TIMEOUT sin heartbeat, cerrando válvula (fail-safe)");
      }
      actuadorSeguro();
      mqttPublish(T_STATUS, "WATCHDOG_TRIPPED", true);
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

  pinMode(LED, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  actuadorSeguro();  // Arrancar siempre seguro

  // Crear mutex del heartbeat tempranamente para evitar race condition
  // si llega un heartbeat via MQTT antes de que la tarea watchdog arranque.
  heartbeatMutex = xSemaphoreCreateMutex();
  if (heartbeatMutex == nullptr) {
    Serial.println("[SETUP] CRÍTICO: No se pudo crear mutex del heartbeat!");
    while (1) vTaskDelay(pdMS_TO_TICKS(1000));
  }

  netBegin();  // Crea la tarea MQTT y arranca WiFi/MQTT (Core 0)

  // WATCHDOG PINNED TO CORE 1 - Máxima soberanía
  // Prioridad 4 (crítica), aislado del WiFi stack que está en Core 0
  xTaskCreatePinnedToCore(
    taskWatchdog,           // Función
    "watchdog",             // Nombre
    3072,                   // Stack size
    nullptr,                // Parámetro
    4,                      // Prioridad (máxima)
    &watchdogTask,          // Handle
    1                       // Core 1 (dedicado)
  );
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(10));
}
