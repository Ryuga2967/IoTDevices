#ifndef __MQTT_BRIDGE_H
#define __MQTT_BRIDGE_H

#include "../../MQTT/3.1.1/mqtt.h"

struct MqttBridge {
  struct mqtt_client_t client;
  uint8_t isSsl;
  char* ip;
  uint32_t port;
};

#endif
