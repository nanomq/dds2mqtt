#ifndef _MQTT_DDS_HELLOWORLD_TYPES_H_
#define _MQTT_DDS_HELLOWORLD_TYPES_H_

#include "HelloWorld.h"

// It should not be changed
typedef struct fixed_mqtt_msg {
	uint8_t *payload;
	uint32_t len;
} fixed_mqtt_msg;

void HelloWorld_to_MQTT(HelloWorld *m1, fixed_mqtt_msg *m2);

void MQTT_to_HelloWorld(fixed_mqtt_msg *m1, HelloWorld *m2);

#endif
