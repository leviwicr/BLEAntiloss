#include "MQTTCommunicate.h"
#include "BLECommunicate.h"

#ifdef LFL
const char *ssid="Xiaomi 15";
const char *password="99999999";
const char *mqtt_broker="pbcac80e.ala.cn-hangzhou.emqxsl.cn";
const char* mqtt_usrname="levi";
const char* mqtt_password="1029384756";
int mqtt_port=8883;
WiFiClientSecure espClient;//创建底层SSL连接对象
PubSubClient client(espClient);//基于SSL的MQTT客户端，传入网络对象
const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----
)EOF";
#endif

#ifdef HZL
const char *ssid="Xiaomi 15";
const char *password="99999999";
const char *mqtt_broker="192.168.213.36";
const char* mqtt_usrname="BlueTeeth2";
const char* mqtt_password="123456";
int mqtt_port=1883;
WiFiClient espClient;//创建底层TCP连接对象
PubSubClient client(espClient);//基于TCP的MQTT客户端，传入网络对象

#endif

void connectToMQTT(){
  /*
  configTime(8*3600, 0, "pool.ntp.org", "ntp.aliyun.com");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Waiting for NTP time...");
    delay(500);
  }
  time_t now = time(NULL);
  Serial.printf("ESP32 current timestamp: %ld\n", now);
  */
  #ifdef LFL
  espClient.setCACert(ca_cert);
  #endif
  client.setServer(mqtt_broker,mqtt_port);
  client.setKeepAlive(60);

  while (!client.connected()) {
        String client_id = "esp32-client-" + String(WiFi.macAddress());
        Serial.printf("Connecting to MQTT Broker as %s...\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_usrname, mqtt_password)) {
            Serial.println("Connected to MQTT Broker");
        } else {
            Serial.print("Failed to connect, rc=");
            Serial.print(client.state());
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds.");
            delay(5000);
        }
    }
}
void MQTTInit(){
  Serial.println("Linking to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);//立即返回，连接在后台执行
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected, ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  connectToMQTT();
  //发送初始位置
  deviceRSSI=pClient->getRssi();
  distance=pow(10,(-75-deviceRSSI)/23.3);
  String msg = "{\"status\":\"blueteeth has connected\",\"distance\":";
      msg += String(distance);
      msg += "}";
      client.publish("/HZL2/pub", msg.c_str());
}