#ifndef ZIGBEE_H
#define ZIGBEE_H

#include "esp_zigbee_core.h"

#define MANUFACTURER_NAME "\x08" \
                          "Owl Corp"
#define MODEL_IDENTIFIER "\x09" \
                         "Finch 1.0"

void set_endpoints();
void set_reporting();

#endif