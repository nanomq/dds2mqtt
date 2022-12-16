#include "dds/dds.h"
#include "HelloWorld.h"
#include "subpub.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MQTT
#include <nng/mqtt/mqtt_client.h>
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>

#include "mqtt_client.h"
#include "HelloWorldMQTTTypes.h"

int publisher (int argc, char ** argv)
{
  dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t writer;
  dds_return_t rc;
  HelloWorld   msg;
  uint32_t status = 0;
  (void)argc;
  (void)argv;

  /* Create a Participant. */
  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  if (participant < 0)
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));

  /* Create a Topic. */
  topic = dds_create_topic (
    participant, &HelloWorld_desc, "HelloWorld", NULL, NULL);
  if (topic < 0)
    DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-topic));

  /* Create a Writer. */
  writer = dds_create_writer (participant, topic, NULL, NULL);
  if (writer < 0)
    DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-writer));

  printf("=== [Publisher]  Waiting for a reader to be discovered ...\n");
  fflush (stdout);

  rc = dds_set_status_mask(writer, DDS_PUBLICATION_MATCHED_STATUS);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_set_status_mask: %s\n", dds_strretcode(-rc));

  while(!(status & DDS_PUBLICATION_MATCHED_STATUS))
  {
    rc = dds_get_status_changes (writer, &status);
    if (rc != DDS_RETCODE_OK)
      DDS_FATAL("dds_get_status_changes: %s\n", dds_strretcode(-rc));

    /* Polling sleep. */
    dds_sleepfor (DDS_MSECS (20));
  }

  nng_msg* rmsg;
  nng_socket sock;
  uint32_t len;
  const char* url = "mqtt-tcp://127.0.0.1:1883";
  const char* mqtttopic = "MQTTCMD-HelloWorldTopic";

  mqtt_connect(&sock, url);
  mqtt_subscribe(&sock, mqtttopic, 0);

  if (0 != mqtt_recvmsg(&sock, &rmsg)) {
	  printf("Error in recv.\n");
  }
  const char *t = nng_mqtt_msg_get_publish_topic(rmsg, &len);
  printf("%-*s\n", len, t);
  if (strncmp(t, mqtttopic, len) != 0) {
	  printf("Error in Topic.\n");
  }

  fixed_mqtt_msg mqttmsg;
  uint8_t *data = nng_mqtt_msg_get_publish_payload(rmsg, &len);
  if (!data || len == 0) {
	  printf("Error in msg.\n");
  }
  mqttmsg.payload = data;
  mqttmsg.len     = len;

  MQTT_to_HelloWorld(&mqttmsg, &msg);

  /* Create a message to write. */
  msg.index = 1;

  printf ("=== [Publisher]  Writing : ");
  printf ("Message (%"PRId32", %s)\n", msg.index, msg.message);
  fflush (stdout);

  rc = dds_write (writer, &msg);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_write: %s\n", dds_strretcode(-rc));

  /* Deleting the participant will delete all its children recursively as well. */
  rc = dds_delete (participant);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));

  return EXIT_SUCCESS;
}
