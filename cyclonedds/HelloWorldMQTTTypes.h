#ifndef _MQTT_DDS_HELLOWORLD_TYPES_H_
#define _MQTT_DDS_HELLOWORLD_TYPES_H_

#include "HelloWorld.h"

// It should not be changed
typedef struct fixed_mqtt_msg {
	uint8_t *message;
	uint8_t  len;
	int8_t   int8_test;
	uint8_t  uint8_test;
	int16_t  int16_test;
	uint16_t uint16_test;

	int32_t int32_test;

	uint32_t uint32_test;

	int64_t int64_test;

	uint64_t uint64_test;
	// array example
	test_enum   example_enum;
	test_struct example_stru;
} fixed_mqtt_msg;

void HelloWorld_to_MQTT(example_struct *m1, fixed_mqtt_msg *m2);

void MQTT_to_HelloWorld(fixed_mqtt_msg *m1, example_struct *m2);

#endif
