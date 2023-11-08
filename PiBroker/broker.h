#ifndef __BROKER_H
#define __BROKER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <dlfcn.h>
#include "event.h"

typedef struct network_handler {
} network_handler_t;

typedef struct hardware_handler {
} hardware_handler_t;

typedef struct iot_device {
  int id;
  network_handler_t network_handler;
  hardware_handler_t hardware_handler;
  void* data;
} iot_device_t;

#endif
