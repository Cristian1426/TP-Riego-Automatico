// === Red WiFi ===
#define WIFI_SSID     "Figura"
#define WIFI_PASS     "12348765"

// === Broker MQTT ===
#define MQTT_HOST     "10.84.129.196"
#define MQTT_PORT     1883

// === Topics ===
#define T_HEARTBEAT   "huerta/heartbeat"
#define T_STATUS      "huerta/status"    // Estado actual / fail-safe
#define T_CMD         "huerta/cmd"       // "ABRIR" / "CERRAR"

// === Intervalos ===
#define PERIOD_HB_MS  1000               // heartbeat cada 5s
#define WATCHDOG_TIMEOUT 5000            // 5s sin heartbeat activa un mecanismo fail-safe

// === Hardware ===
#define LED   2
#define RELAY_PIN     12
#define RELAY_ACTIVE_ON  HIGH
#define RELAY_SAFE_ON    LOW
