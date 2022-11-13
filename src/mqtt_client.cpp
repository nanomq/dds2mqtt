// Author: eeff <eeff at eeff dot dev>
//
// This software is supplied under the terms of the MIT License, a
// copy of which should be located in the distribution where this
// file was obtained (LICENSE.txt).  A copy of the license may also be
// found online at https://opensource.org/licenses/MIT.
//

//
// This is just a simple MQTT client demonstration application.
//
// The application has two sub-commands: `pub` and `sub`. The `pub`
// sub-command publishes a given message to the server and then exits.
// The `sub` sub-command subscribes to the given topic filter and blocks
// waiting for incoming messages.
//
// # Example:
//
// Publish 'hello' to `topic` with QoS `0`:
// ```
// $ ./mqtt_client pub mqtt-tcp://127.0.0.1:1883 0 topic hello
// ```
//
// Subscribe to `topic` with QoS `0` and waiting for messages:
// ```
// $ ./mqtt_client sub mqtt-tcp://127.0.0.1:1883 0 topic
// ```
//

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include <nng/mqtt/mqtt_client.h>
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>

// Subcommands
#define PUBLISH "pub"
#define SUBSCRIBE "sub"

void
fatal(const char *msg, int rv)
{
	fprintf(stderr, "%s: %s\n", msg, nng_strerror(rv));
}

static void
disconnect_cb(nng_pipe p, nng_pipe_ev ev, void *arg)
{
	int reason = 0;
	// get connect reason
	nng_pipe_get_int(p, NNG_OPT_MQTT_DISCONNECT_REASON, &reason);
	// property *prop;
	// nng_pipe_get_ptr(p, NNG_OPT_MQTT_DISCONNECT_PROPERTY, &prop);
	// nng_socket_get?
	printf("%s: disconnected!\n", __FUNCTION__);
}

static void
connect_cb(nng_pipe p, nng_pipe_ev ev, void *arg)
{
	int reason;
	// get connect reason
	nng_pipe_get_int(p, NNG_OPT_MQTT_CONNECT_REASON, &reason);
	// get property for MQTT V5
	// property *prop;
	// nng_pipe_get_ptr(p, NNG_OPT_MQTT_CONNECT_PROPERTY, &prop);
	printf("%s: connected!\n", __FUNCTION__);
}

// Connect to the given address.
int
client_connect(
    nng_socket *sock, nng_dialer *dialer, const char *url, bool verbose)
{
	int        rv;

	if ((rv = nng_mqtt_client_open(sock)) != 0) {
		fatal("nng_socket", rv);
	}

	if ((rv = nng_dialer_create(dialer, *sock, url)) != 0) {
		fatal("nng_dialer_create", rv);
	}

	// create a CONNECT message
	/* CONNECT */
	nng_msg *connmsg;
	nng_mqtt_msg_alloc(&connmsg, 0);
	nng_mqtt_msg_set_packet_type(connmsg, NNG_MQTT_CONNECT);
	nng_mqtt_msg_set_connect_proto_version(connmsg, 4);
	nng_mqtt_msg_set_connect_keep_alive(connmsg, 60);
	nng_mqtt_msg_set_connect_user_name(connmsg, "nng_mqtt_client");
	nng_mqtt_msg_set_connect_password(connmsg, "secrets");
	nng_mqtt_msg_set_connect_will_msg(
	    connmsg, (uint8_t *) "bye-bye", strlen("bye-bye"));
	nng_mqtt_msg_set_connect_will_topic(connmsg, "will_topic");
	nng_mqtt_msg_set_connect_clean_session(connmsg, true);

	nng_mqtt_set_connect_cb(*sock, connect_cb, sock);
	nng_mqtt_set_disconnect_cb(*sock, disconnect_cb, connmsg);

	uint8_t buff[1024] = { 0 };

	if (verbose) {
		nng_mqtt_msg_dump(connmsg, buff, sizeof(buff), true);
		printf("%s\n", buff);
	}

	printf("Connecting to server ...\n");
	nng_dialer_set_ptr(*dialer, NNG_OPT_MQTT_CONNMSG, connmsg);
	nng_dialer_start(*dialer, NNG_FLAG_NONBLOCK);

	return (0);
}

// Publish a message to the given topic and with the given QoS.
int
client_publish(nng_socket sock, const char *topic, uint8_t *payload,
    uint32_t payload_len, uint8_t qos, bool verbose)
{
	int rv;

	// create a PUBLISH message
	nng_msg *pubmsg;
	nng_mqtt_msg_alloc(&pubmsg, 0);
	nng_mqtt_msg_set_packet_type(pubmsg, NNG_MQTT_PUBLISH);
	nng_mqtt_msg_set_publish_dup(pubmsg, 0);
	nng_mqtt_msg_set_publish_qos(pubmsg, qos);
	nng_mqtt_msg_set_publish_retain(pubmsg, 0);
	nng_mqtt_msg_set_publish_payload(
	    pubmsg, (uint8_t *) payload, payload_len);
	nng_mqtt_msg_set_publish_topic(pubmsg, topic);

	if (verbose) {
		uint8_t print[1024] = { 0 };
		nng_mqtt_msg_dump(pubmsg, print, 1024, true);
		printf("%s\n", print);
	}

	printf("Publishing to '%s' ...\n", topic);
	if ((rv = nng_sendmsg(sock, pubmsg, NNG_FLAG_NONBLOCK)) != 0) {
		fatal("nng_sendmsg", rv);
	}

	return rv;
}

static void
sub_callback(void *arg) {
	nng_mqtt_client *client = (nng_mqtt_client *) arg;
	nng_aio *aio = client->sub_aio;
	nng_msg *msg = nng_aio_get_msg(aio);
	uint32_t count;
	reason_code *code;
	code = (reason_code *)nng_mqtt_msg_get_suback_return_codes(msg, &count);
	printf("aio mqtt result %d \n", nng_aio_result(aio));
	// printf("suback %d \n", *code);
	nng_msg_free(msg);
}

static void
unsub_callback(void *arg) {
	nng_mqtt_client *client = (nng_mqtt_client *) arg;
	nng_aio *aio = client->unsub_aio;
	nng_msg *msg = nng_aio_get_msg(aio);
	uint32_t count;
	reason_code *code;
	// code = (reason_code *)nng_mqtt_msg_get_suback_return_codes(msg, &count);
	printf("aio mqtt result %d \n", nng_aio_result(aio));
	// printf("suback %d \n", *code);
	nng_msg_free(msg);
}

int
mqtt_connect(nng_socket *sock, const char *url)
{
	bool        verbose = 1;

	nng_dialer  dialer;
	client_connect(sock, &dialer, url, verbose);

	// TODO create a thread to send / recv mqtt msg

	return 0;
}

int
mqtt_disconnect(nng_socket *sock)
{}

int
mqtt_subscribe(nng_socket *sock, const char *topic, const uint8_t qos)
{
	nng_mqtt_topic_qos subscriptions[] = {
		{
		    .qos   = qos,
		    .topic = {
				.buf    = (uint8_t *) topic,
		        .length = (uint32_t) strlen(topic),
			},
		},
	};

	// Sync subscription
	return nng_mqtt_subscribe(sock, subscriptions, 1, NULL);
	/*
	nng_mqtt_cb_opt cb_opt = {
		.sub_ack_cb = sub_callback,
		.unsub_ack_cb = unsub_callback,
	};

	// Asynchronous subscription
	nng_mqtt_client *client = nng_mqtt_client_alloc(sock, &cb_opt, true);
	nng_mqtt_subscribe_async(client, subscriptions, 1, NULL);

	return 0;
	*/
}

int
mqtt_unsubscribe(nng_socket *sock, const char *topic)
{
	nng_mqtt_topic unsubscriptions[] = {
		{
		    .buf    = (uint8_t *) topic,
		    .length = (uint32_t) strlen(topic),
		},
	};

	// nng_mqtt_unsubscribe_async(client, unsubscriptions, 1, NULL);

	return 0;
}

int
mqtt_publish(nng_socket *sock, const char *topic, uint8_t qos, uint8_t *data, int len)
{
	return client_publish(*sock, topic, data, len, qos, 1);
}

int
mqtt_recvmsg(nng_socket *sock, nng_msg **msgp)
{
	int rv;
	nng_msg *msg;
	if ((rv = nng_recvmsg(*sock, &msg, 0)) != 0) {
		printf("Error in nng_recvmsg %d.\n", rv);
		return -2;
	}

	// we should only receive publish messages
	if (nng_mqtt_msg_get_packet_type(msg) != NNG_MQTT_PUBLISH) {
		printf("Invalid MQTT Msg type.\n");
		return -3;
	}

	*msgp = msg;
	return 0;
}

