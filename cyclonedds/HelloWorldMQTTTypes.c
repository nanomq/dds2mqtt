#include <string.h>

#include "HelloWorldMQTTTypes.h"
#include "HelloWorld.h"
#include "cJSON.h"

void
HelloWorld_to_MQTT(example_struct *m1, fixed_mqtt_msg *m2)
{
	cJSON* obj = NULL;
	cJSON* test_msg = NULL;
	char* str;
	obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj,"int8_test",m1->int8_test);
    cJSON_AddNumberToObject(obj,"uint8_test",m1->uint8_test);
    cJSON_AddNumberToObject(obj,"int16_test",m1->int16_test);
    cJSON_AddNumberToObject(obj,"uint16_test",m1->uint16_test);
    cJSON_AddNumberToObject(obj,"int32_test",m1->int32_test);
    cJSON_AddNumberToObject(obj,"uint32_test",m1->uint32_test);
    cJSON_AddNumberToObject(obj,"int64_test",m1->int64_test);
    cJSON_AddNumberToObject(obj,"uint64_test",m1->uint64_test);

    cJSON_AddStringToObject(obj,"message",m1->message);
    cJSON_AddNumberToObject(obj,"example_enum",m1->emample_enum);
    test_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(test_msg,"message",m1->example_stru);
    cJSON_AddItemToObject(obj,"example_stru",test_msg);

	str = cJSON_Print(obj);

	m2->payload = str;
	m2->len = 256;
}

void
MQTT_to_HelloWorld(fixed_mqtt_msg *m1, example_struct *m2)
{
	cJSON* cjson_test = NULL;
    cJSON* cjson_tmp = NULL;
    cJSON* cjson_tmp2 = NULL;

    
    struct test_struct* es = &m2->example_stru;
    cjson_test = cJSON_Parse(str);
    if(cjson_test == NULL)
    {
        printf("Parse fail!\n");
        return NULL;
    }
    cjson_tmp = cJSON_GetObjectItem(cjson_test,"int8_test");
    m2->int8_test = cjson_tmp->valueint;
    cjson_tmp = cJSON_GetObjectItem(cjson_test,"uint8_test");
    m2->uint8_test = cjson_tmp->valueint;
    cjson_tmp = cJSON_GetObjectItem(cjson_test,"int16_test");
    m2->int16_test = cjson_tmp->valueint;
    cjson_tmp = cJSON_GetObjectItem(cjson_test,"uint16_test");
    m2->uint16_test = cjson_tmp->valueint;
    cjson_tmp = cJSON_GetObjectItem(cjson_test,"int32_test");
    m2->int32_test = cjson_tmp->valueint;
    cjson_tmp = cJSON_GetObjectItem(cjson_test,"uint32_test");
    m2->uint32_test = cjson_tmp->valueint;
    cjson_tmp = cJSON_GetObjectItem(cjson_test,"int64_test");
    m2->int64_test = cjson_tmp->valueint;
    cjson_tmp = cJSON_GetObjectItem(cjson_test,"uint64_test");
    m2->uint64_test = cjson_tmp->valueint;


    cjson_tmp = cJSON_GetObjectItem(cjson_test,"message");
    strcpy(m2->message,cjson_tmp->valuestring);
    cjson_tmp = cJSON_GetObjectItem(cjson_test,"example_enum");
    m2->example_enum = cjson_tmp->valueint;
    cjson_tmp = cJSON_GetObjectItem(cjson_test,"example_stru");
    cjson_tmp2 = cJSON_GetObjectItem(cjson_tmp,"message");
    strcpy(es->message, cjson_tmp2->valuestring);
    m2->example_stru = *es;



	//memcpy(m2->message, m1->payload, m1->len);
}

