#include "HelloWorld.h"
#include "dds/dds.h"
#include "subpub.h"
#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HelloWorldMQTTTypes.h"
#include "mqtt_client.h"

/* An array of one message (aka sample in dds terms) will be used. */
#define MAX_SAMPLES 1

#define MQTT_URL "mqtt-tcp://127.0.0.1:1883"

static mqtt_cli  mqttcli;
static nftp_vec *handleq;

int
dds_client(int argc, char **argv)
{
	dds_entity_t      participant;
	dds_entity_t      topic;
	dds_entity_t      reader;
	dds_entity_t      writer;
	HelloWorld       *msg;
	void             *samples[MAX_SAMPLES];
	dds_sample_info_t infos[MAX_SAMPLES];
	dds_return_t      rc;
	dds_qos_t        *qos;
	uint32_t          status = 0;
	handle           *hd;
	(void) argc;
	(void) argv;

	/* Create handle queue */
	nftp_vec_alloc(&handleq);

	/* Create a Participant. */
	participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
	if (participant < 0)
		DDS_FATAL("dds_create_participant: %s\n",
		    dds_strretcode(-participant));

	/* Create a Topic. */
	topic = dds_create_topic(
	    participant, &HelloWorld_desc, "HelloWorld", NULL, NULL);
	if (topic < 0)
		DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));

	/* Create a reliable Reader. */
	qos = dds_create_qos();
	dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
	reader = dds_create_reader(participant, topic, qos, NULL);
	if (reader < 0)
		DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-reader));
	dds_delete_qos(qos);

	// TODO Topics for writer and reader **MUST** be different.
	// Or Circle messages happened
	/* Create a Writer */
	writer = dds_create_writer(participant, topic, NULL, NULL);
	if (writer < 0)
		DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-writer));

	printf("=== [Publisher]  Waiting for a reader to be discovered ...\n");
	fflush(stdout);

	rc = dds_set_status_mask(writer, DDS_PUBLICATION_MATCHED_STATUS);
	if (rc != DDS_RETCODE_OK)
		DDS_FATAL("dds_set_status_mask: %s\n", dds_strretcode(-rc));

	while (!(status & DDS_PUBLICATION_MATCHED_STATUS)) {
		rc = dds_get_status_changes(writer, &status);
		if (rc != DDS_RETCODE_OK)
			DDS_FATAL("dds_get_status_changes: %s\n",
			    dds_strretcode(-rc));

		/* Polling sleep. */
		dds_sleepfor(DDS_MSECS(20));
	}

	// MQTT Client create
	// mqtt_connect(&mqttcli, MQTT_URL);

	printf("\n=== [Subscriber] Waiting for a sample ...\n");
	fflush(stdout);

	/* Initialize sample buffer, by pointing the void pointer within
	 * the buffer array to a valid sample memory location. */
	samples[0] = HelloWorld__alloc();

	/* Poll until data has been read. */
	while (true) {
		// If handle queue is not empty. Handle it first.
		// Or we need to receive msgs from DDS in a NONBLOCK way and
		// put it to the handle queue. Sleep when handle queue is
		// empty.

		if (nftp_vec_len(handleq)) {
			nftp_vec_pop(handleq, (void **) &hd, NFTP_HEAD);
			goto work;
		}

		/* Do the actual read.
		 * The return value contains the number of read samples. */
		rc =
		    dds_take(reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
		if (rc < 0)
			DDS_FATAL("dds_read: %s\n", dds_strretcode(-rc));
		/* Check if we read some data and it is valid. */
		if ((rc > 0) && (infos[0].valid_data)) {
			/* Print Message. */
			msg = (HelloWorld *) samples[0];
			printf("=== [Subscriber] Received : ");
			printf("Message (%"PRId32", %s)\n", msg->index,
			    msg->message);
			fflush(stdout);

			/* Put msg to handleq */
			hd = mk_handle(HANDLE_TO_MQTT, msg, 0);
			nftp_vec_append(handleq, (void *) hd);
			hd = NULL;
			continue;
		} else {
			/* Polling sleep. */
			dds_sleepfor(DDS_MSECS(20));
			continue;
		}

work:
		switch (hd->type) {
		case HANDLE_TO_DDS:
			/* Send the msg received */
			rc = dds_write(writer, msg);
			if (rc != DDS_RETCODE_OK)
				DDS_FATAL("dds_write: %s\n", dds_strretcode(-rc));
			break;
		case HANDLE_TO_MQTT:
			// Put to MQTTClient's handle queue
			// TODO
			printf("TODO send a msg to mqtt.\n");
			break;
		default:
			printf("Unsupported handle type.\n");
			break;
		}
	}

	/* Free the data location. */
	HelloWorld_free(samples[0], DDS_FREE_ALL);

	/* Deleting the participant will delete all its children recursively as
	 * well. */
	rc = dds_delete(participant);
	if (rc != DDS_RETCODE_OK)
		DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));

	return EXIT_SUCCESS;
}
