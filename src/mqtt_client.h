#ifndef DDS2MQTT_MQTT_CLIENT
#define DDS2MQTT_MQTT_CLIENT

#include <nng/mqtt/mqtt_client.h>
#include <nng/nng.h>

int mqtt_connect(nng_socket *sock, const char *url);

int mqtt_disconnect(nng_socket *sock);

int mqtt_subscribe(nng_socket *sock, const char *topic);

int mqtt_unsubscribe(nng_socket *sock, const char *topic, const uint8_t qos);

int mqtt_publish(nng_socket *sock, const char *topic, uint8_t qos, uint8_t *data, int len);

int mqtt_recvmsg(nng_socket *sock, const char *topic, uint8_t *qos, uint8_t **datap, int *lenp);

#endif
