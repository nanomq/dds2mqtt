#ifndef DDS2MQTT_MQTT_CLIENT
#define DDS2MQTT_MQTT_CLIENT

#include <nng/mqtt/mqtt_client.h>
#include <nng/nng.h>

typedef struct mqtt_cli mqtt_cli;

int mqtt_connect(mqtt_cli *cli, const char *url);

int mqtt_disconnect(mqtt_cli *cli);

int mqtt_subscribe(mqtt_cli *cli, const char *topic, const uint8_t qos);

// Not supported yet
int mqtt_unsubscribe(mqtt_cli *cli, const char *topic);

int mqtt_publish(mqtt_cli *cli, const char *topic, uint8_t qos, uint8_t *data, int len);

int mqtt_recvmsg(mqtt_cli *cli, nng_msg **msgp);

#endif
