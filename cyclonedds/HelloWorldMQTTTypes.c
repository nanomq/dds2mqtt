#include <string.h>

#include "HelloWorldMQTTTypes.h"
#include "HelloWorld.h"

void
HelloWorld_to_MQTT(example_struct *m1, fixed_mqtt_msg *m2)
{
	const char *data = m1->message;
	int sz = strlen(m1->message);

	m2->message = (uint8_t *)data;
	m2->len = sz;
	
}

void
MQTT_to_HelloWorld(fixed_mqtt_msg *m1, example_struct *m2)
{
	char *data = (char *)m1->message;
	int   len  = m1->len;
	m2->int16_test = m1->int16_test;
	memcpy(m2->message, m1->message, len);
}

