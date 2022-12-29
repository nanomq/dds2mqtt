#ifndef CYCLONEDDS_SUBPUB_H
#define CYCLONEDDS_SUBPUB_H

#include <pthread.h>
#include <stdlib.h>

#include "vector.h"
#include "mqtt_client.h"

typedef struct dds_cli dds_cli;

struct dds_cli {
	int running;
	nftp_vec *handleq;
	pthread_mutex_t mtx;
};

int publisher (int argc, char ** argv);
int subscriber (int argc, char ** argv);

int dds_client (dds_cli *ddscli, mqtt_cli *mqttcli);

int dds_proxy (int argc, char ** argv);

#endif
