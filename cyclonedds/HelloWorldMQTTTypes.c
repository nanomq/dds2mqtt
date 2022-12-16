#include <string.h>

#include "HelloWorldMQTTTypes.h"
#include "HelloWorld.h"

void
HelloWorld_to_MQTT(HelloWorld *m1, fixed_mqtt_msg *m2)
{
	const char *data = m1->message;
	int sz = strlen(m1->message);

	m2->payload = (uint8_t *)data;
	m2->len = sz;
}

void
MQTT_to_HelloWorld(fixed_mqtt_msg *m1, HelloWorld *m2)
{
	char *data = (char *)m1->payload;
	int   len  = m1->len;

	m2->message = strndup(data, len);
}

