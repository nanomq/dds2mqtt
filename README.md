# dds2mqtt

Here we combine dds with mqtt. So DDS node can communicate with MQTT broker.

FastDDS node <--local--> NanoSDK client <--network--> NanoMQ Broker

## Requires

Select one of DDS backends.

+ FastDDS version == 2.8.x

+ CycloneDDS version == 0.10.1

NanoSDK is a MQTT SDK.

+ NanoSDK version >= 0.7.5

Note. libfastrtps/libddsc and libnng should be installed.

## NOTE

Select topics from configure file is not supported.

## TEST

Tab1. Turn on nanomq
Tab2. ./dds2mqtt proxy

DDS to MQTT
Tab3. ./nanomq_cli/nanomq_cli sub --url "mqtt-tcp://127.0.0.1:1883" -t "DDS/HelloWorld"
Tab4. ./dds2mqtt pub

MQTT to DDS
Tab5. ./dds2mqtt
Tab6. ./nanomq_cli/nanomq_cli pub -t DDSCMD/HelloWorld -m aaaaa

