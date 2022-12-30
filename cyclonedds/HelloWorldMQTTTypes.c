#include <string.h>

#include "HelloWorldMQTTTypes.h"
#include "HelloWorld.h"

void
HelloWorld_to_MQTT(example_struct *m1, fixed_mqtt_msg *m2)
{
	m2->payload = (char *)m1->message;
	m2->len = 256;
}

void
MQTT_to_HelloWorld(fixed_mqtt_msg *m1, example_struct *m2)
{
	memcpy(m2->message, m1->payload, m1->len);
}

