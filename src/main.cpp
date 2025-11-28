#include <Arduino.h>
#include <BLEDevice.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>


/* 蓝牙通信与RSSI检测部分 */
//#define SERVICE_UUID="";//服务与特征的UUID，需要和广播设备一致，需交流信息时使用
//#define CHARACTERISTIC_UUID="";
static boolean scan=false;//是否扫描到设备的标志位
static boolean connected=false;//判断连接是否成功与正常的标志位
static bool isLost=false;//判断RSSI值是否正常的标志位
static BLEClient *pClient;//主机端
static BLEAddress *pServerAddress;//加与不加static有何区别？指针必须！！！指向一个确定的地方
static BLEAdvertisedDevice* ptargetDevice = nullptr;//包含目标连接对象的信息
//static BLECharacteristic *pCharacteristic;//特征，类
static char targetName[]="linzhi8";
static int deviceRSSI=-100;
//unsigned long current_time=0;
//unsigned long lastscan_time=0;
int RSSICount=0;
float RSSI_hat=0.0;


/* 屏幕部分 */
//TFT_eSPI tft=TFT_eSPI();//创建屏幕对象，可指定对象尺寸，不指定将使用User_Setup.h中定义的尺寸

/* WiFi与MQTT部分 */
//const char *ssid="happyhappy";
//const char *password="1029384756";
const char *ssid="Xiaomi 15";
const char *password="99999999";
const char *mqtt_broker="pbcac80e.ala.cn-hangzhou.emqxsl.cn";
const char* mqtt_usrname="levi";
const char* mqtt_password="1029384756";
int mqtt_port=8883;
WiFiClientSecure espClient;//创建底层TCP连接对象
PubSubClient client(espClient);//基于TCP的MQTT客户端，传入网络对象
// Load DigiCert Global Root CA ca_cert, which is used by EMQX Cloud Serverless Deployment
const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";
/* 蓝牙通信部分 */
class MyClientCallbacks:public BLEClientCallbacks {
  void onConnect(BLEClient *pclient){}
  //断联之后的回调函数
  void onDisconnect(BLEClient *pclient){
    connected=false;//作为标志方便尝试重新连接
    Serial.println("onDisconnect:Client Disconnected");
  }
};
//扫描，当任何BLE设备被发现时onResult方法会被调用
class MyAdvertisedDeviceCallbacks:public BLEAdvertisedDeviceCallbacks{
  void onResult(BLEAdvertisedDevice advertisedDevice){
    if(scan) return;
    if(advertisedDevice.haveName() && strcmp(advertisedDevice.getName().c_str(),targetName)==0){
      deviceRSSI=advertisedDevice.getRSSI();
      Serial.printf("Successfully find: %s RSSI=%d\n", advertisedDevice.getName().c_str(),deviceRSSI);
      ptargetDevice = new BLEAdvertisedDevice(advertisedDevice);
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());//new分配空间给BLEAddress这个类的一个实例，通过指针指向
      scan=true;

      BLEDevice::getScan()->stop();       // 关键：真正停止扫描！
      BLEDevice::getScan()->clearResults();//?????
    }
  }
};
bool connectToServer(){
  //万万不可把pClient作为函数参数传入！！！该步将改变pClient（指针）的值，但改变的是局部变量！函数中使用指针的正确方法是改变指针指向地址而不能改变指针本身！
  //传入的pAddress就是实际的从机地址
  // 连接到远程 BLE 服务器（从机）
  if (!pClient->connect(ptargetDevice)) {
    Serial.println(" - Connection failed");
    return false;
  }
  Serial.println(" - Connected to server");
  connected=true;
  return true;
}
void BLECommunicationInit(){
  BLEDevice::init("ESP32Scanner");
  BLEScan *pBLEScan=BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks);
  //pBLEScan->setInterval(100);//设置扫描间隔
  //pBLEScan->setWindow(80);//实际扫描时间
  pBLEScan->setActiveScan(true);//设置为主动扫描模式
  Serial.println("Scanning until device found...");
  
  pBLEScan->start(60,NULL,true);//一直扫描，无结束回调函数，不阻塞
  while(!scan){
    delay(10);
  }

  //创建client,连接从机
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallbacks());
  
  Serial.print("trying to Connect to ");
  Serial.println((*pServerAddress).toString().c_str());
  while(!connectToServer()){
    delay(10);
  }
}

/* WiFi与MQTT部分 */

void connectToMQTT(){
  espClient.setCACert(ca_cert);
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
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  connectToMQTT();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting......");

  //esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  /* 蓝牙通信部分 */
  BLECommunicationInit();
  
  //显示屏部分
  //tft.fillScreen(TFT_BLACK);
  //tft.setCursor(0,0,2);
  //tft.setTextColor(TFT_WHITE,TFT_BLACK);
  //tft.setTextSize(1);
  //tft.println("Hello world");

  /* WiFi与MQTT部分*/
  MQTTInit();
}
//无法处理连接断开情况，考虑如何解决
void loop() {
  if(connected){
    if(!client.connected()) connectToMQTT();//后续再考虑两者互不堵塞
    client.loop();

    deviceRSSI=pClient->getRssi();
    if(RSSICount<=10){
      RSSICount++;
      float alpha=(1/float(RSSICount));
      RSSI_hat=(1-alpha)*RSSI_hat+alpha*deviceRSSI;
    }else{
      RSSI_hat=0.9*RSSI_hat+0.1*deviceRSSI;
    }
    isLost=((RSSI_hat<(-90.0)));
    Serial.printf("RSSI_hat: %f\n",RSSI_hat);
    //如果丢失进行报警
    if(isLost){
      Serial.println("Warning!!!!!");
      //tft.println("Warning!!!!");
      String msg = "{\"status\":\"lost\",\"rssi\":";
      msg += String(RSSI_hat);
      msg += "}";
      client.publish("/tracker/alert", msg.c_str());
      delay(1000);
    }
    else{
      Serial.println("safe");
    }
    delay(1000);
  }
  else{
    while(!connectToServer()){
      Serial.print(".");
      delay(10);
    }
  }
}
