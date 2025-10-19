#include <PubSubClient.h>

constexpr uint32_t PERIOD_MS = 5000;  // OK, al ser constexpr no duplica símbolos

void netBegin();                  // conecta WiFi y MQTT
void mqttLoop(bool *estado);      // client.loop()
void mqttPublishPeriodic();      // publish cada PERIOD_MS