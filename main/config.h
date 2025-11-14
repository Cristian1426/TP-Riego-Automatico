// === Red WiFi ===
#define WIFI_SSID     "Redmi Note 10 Pro"
#define WIFI_PASS     "cris1234"

// === Broker MQTT ===
#define MQTT_HOST     "10.210.129.196"
#define MQTT_PORT     1883

// === Topics ===
#define T_HEARTBEAT   "huerta/heartbeat"
#define T_STATUS      "huerta/status"    // Estado actual / fail-safe
#define T_CMD         "huerta/cmd"       // "ABRIR" / "CERRAR"
#define T_DEBUG       "huerta/debug"     // Comandos de debugging: "PAUSE_HB", "RESUME_HB", "STATS"

// === Intervalos ===
#define PERIOD_HB_MS  5000               // heartbeat cada 5s
#define WATCHDOG_TIMEOUT 10000            // 10s sin heartbeat activa un mecanismo fail-safe

// === Hardware ===
#define LED   2
#define RELAY_PIN     12
#define RELAY_ACTIVE_ON  HIGH
#define RELAY_SAFE_ON    LOW
