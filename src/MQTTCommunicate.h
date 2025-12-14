#ifndef MQTTCOMMUNICATE_H
#define MQTTCOMMUNICATE_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

//#define LFL
#define HZL

#ifdef LFL
extern const char *ssid;
extern const char *password;
extern const char *mqtt_broker;
extern const char* mqtt_usrname;
extern const char* mqtt_password;
extern int mqtt_port;
extern WiFiClientSecure espClient;//创建底层SSL连接对象
extern PubSubClient client(espClient);//基于SSL的MQTT客户端，传入网络对象
extern const char* ca_cert;
#endif

#ifdef HZL
extern const char *ssid;
extern const char *password;
extern const char *mqtt_broker;
extern const char* mqtt_usrname;
extern const char* mqtt_password;
extern int mqtt_port;
extern WiFiClient espClient;
extern PubSubClient client;

#endif


void connectToMQTT();
void MQTTInit();

#endif
