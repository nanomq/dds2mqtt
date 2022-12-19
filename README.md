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
