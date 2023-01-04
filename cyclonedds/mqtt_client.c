// Author: wangha <wanghamax at gmail dot com>
//
// This software is supplied under the terms of the MIT License, a
// copy of which should be located in the distribution where this
// file was obtained (LICENSE.txt).  A copy of the license may also be
// found online at https://opensource.org/licenses/MIT.
//

//
// This is a simple wrap for nanosdk to help send and receive mqtt msgs
// for dds2mqtt.
//

#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "HelloWorldMQTTTypes.h"
#include "mqtt_client.h"
#include "vector.h"
#include "dds_client.h"

#include <nng/mqtt/mqtt_client.h>
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>

handle *
mk_handle(int type, void *data, int len)
{
	handle *hd = malloc(sizeof(handle));
	if (hd == NULL)
		return NULL;
	hd->data = data;
	hd->type = type;
	hd->len  = len;

	return hd;
}

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
	int rv;

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

static pthread_t recvthr;
static nftp_vec *rmsgq;

// TODO
// It works in a NONBLOCK way
// Return 0 when got msg. return 1 when no msg; else errors happened
int
client_recv(mqtt_cli *cli, nng_msg **msgp)
{
	if (nftp_vec_len(rmsgq) == 0) {
		return 1;
	}
	nftp_vec_pop(rmsgq, (void **)msgp, NFTP_HEAD);
	return 0;
}

int
client_recv2(mqtt_cli *cli, nng_msg **msgp)
{
	int      rv;
	nng_msg *msg;
	if ((rv = nng_recvmsg(cli->sock, &msg, NNG_FLAG_NONBLOCK)) != 0) {
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

static void
mqtt_recv_loop(void *arg)
{
	mqtt_cli      *cli = arg;
	nng_msg       *msg;
	while (cli->running) {
		msg = NULL;
		client_recv2(cli, &msg);
		nftp_vec_append(rmsgq, msg);
	}
}

static void
mqtt_loop(void *arg)
{
	mqtt_cli      *cli = arg;
	handle        *hd  = NULL;
	nng_msg       *msg;
	void          *ddsmsg;
	fixed_mqtt_msg mqttmsg;
	int            rv;
	dds_cli       *ddscli = cli->ddscli;

	while (cli->running) {
		// If handle queue is not empty. Handle it first.
		// Or we need to receive msgs from nng in a NONBLOCK way and
		// put it to the handle queue. Sleep when handle queue is
		// empty.
		hd = NULL;

		pthread_mutex_lock(&cli->mtx);
		if (nftp_vec_len(cli->handleq))
			nftp_vec_pop(cli->handleq, (void **) &hd, NFTP_HEAD);
		pthread_mutex_unlock(&cli->mtx);

		if (hd)
			goto work;

		rv = client_recv(cli, &msg);
		if (rv < 0) {
			printf("Errror in recv msg\n");
			continue;
		} else if (rv == 0) {
			// Received msg and put to handleq
			hd = mk_handle(HANDLE_TO_DDS, msg, 0);

			pthread_mutex_lock(&cli->mtx);
			nftp_vec_append(cli->handleq, (void *) hd);
			pthread_mutex_unlock(&cli->mtx);

			continue;
		}
		// else No msgs available

		// Sleep and continue
		nng_msleep(500);
		continue;
	work:
		switch (hd->type) {
		case HANDLE_TO_DDS:
			// Put to DDSClient's handle queue
			pthread_mutex_lock(&ddscli->mtx);
			nftp_vec_append(ddscli->handleq, (void *) hd);
			pthread_mutex_unlock(&ddscli->mtx);

			printf("[MQTT] send msg to dds.\n");
			break;
		case HANDLE_TO_MQTT:
			// Translate DDS msg to MQTT format
			ddsmsg = hd->data;
			printf("[MQTT] send msg to mqtt.\n");
			HelloWorld_to_MQTT(ddsmsg, &mqttmsg);
			mqtt_publish(cli, cli->mqttsend_topic, 0,
			    (uint8_t *)mqttmsg.payload, mqttmsg.len);
			break;
		default:
			printf("Unsupported handle type.\n");
			break;
		}
	}
}

int
mqtt_connect(mqtt_cli *cli, const char *url, void *dc)
{
	bool       verbose = 1;
	nng_dialer dialer;
	dds_cli *  ddscli = dc;

	client_connect(&cli->sock, &dialer, url, verbose);

	// Start mqtt thread
	cli->running = 1;

	nftp_vec_alloc(&cli->handleq);
	pthread_mutex_init(&cli->mtx, NULL);

	cli->ddscli = ddscli;

	// Create a thread to send / recv mqtt msg
	pthread_create(&cli->thr, NULL, mqtt_loop, (void *) cli);

	// XXX Create a temparary thread to recv mqtt msg
	nftp_vec_alloc(&rmsgq);
	pthread_create(&recvthr, NULL, mqtt_recv_loop, (void *) cli);

	return 0;
}

int
mqtt_disconnect(mqtt_cli *cli)
{
	// TODO Send disconnect msg
	cli->running = 0;

	if (cli->handleq)
		nftp_vec_free(cli->handleq);
	pthread_mutex_destroy(&cli->mtx);
	return 0;
}

int
mqtt_subscribe(mqtt_cli *cli, const char *topic, const uint8_t qos)
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
	return nng_mqtt_subscribe(&cli->sock, subscriptions, 1, NULL);
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
mqtt_unsubscribe(mqtt_cli *cli, const char *topic)
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
mqtt_publish(
    mqtt_cli *cli, const char *topic, uint8_t qos, uint8_t *data, int len)
{
	return client_publish(cli->sock, topic, data, len, qos, 1);
}

int
mqtt_recvmsg(mqtt_cli *cli, nng_msg **msgp)
{
	return 0;
}

/*
static void
sub_callback(void *arg) {
        nng_mqtt_client *client = (nng_mqtt_client *) arg;
        nng_aio *aio = client->sub_aio;
        nng_msg *msg = nng_aio_get_msg(aio);
        uint32_t count;
        reason_code *code;
        code = (reason_code *)nng_mqtt_msg_get_suback_return_codes(msg,
&count); printf("aio mqtt result %d \n", nng_aio_result(aio));
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
        // code = (reason_code *)nng_mqtt_msg_get_suback_return_codes(msg,
&count); printf("aio mqtt result %d \n", nng_aio_result(aio));
        // printf("suback %d \n", *code);
        nng_msg_free(msg);
}
*/
