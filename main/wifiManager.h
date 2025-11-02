#include <Arduino.h>

// Inicializa WiFi + MQTT y crea la tarea MQTT
void netBegin();

// Publica manualmente
bool mqttPublish(const char* topic, const char* payload, bool retained = false);

// Iteracion manual
void mqttServiceOnce();

// Consulta estado de conexion MQTT
bool mqttConnected();

// Hook para comandos
void handleCommand(const String& msg);

// Hook del watchdog: se llama DESPUES de publicar correctamente el heartbeat
void onHeartbeatOk();