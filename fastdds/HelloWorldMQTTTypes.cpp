#include <string.h>

#include <iostream>
#include <string>

#include "HelloWorldMQTTTypes.h"

void
HelloWorld_to_MQTT(HelloWorld *m1, fixed_mqtt_msg *m2)
{
	const char *data = m1->message().c_str();
	int sz = m1->message().size();

	m2->payload = (uint8_t *)data;
	m2->len = sz;
}

void
MQTT_to_HelloWorld(fixed_mqtt_msg *m1, HelloWorld *m2)
{
	char *data = (char *)m1->payload;
	int   len  = m1->len;

	m2->message(std::string(data).substr(0, len));
}

