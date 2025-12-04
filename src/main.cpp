#include <Arduino.h>
#include <BLEDevice.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "SensorQMI8658.hpp"

//#define LFL
#define HZL


/* 蓝牙通信与RSSI检测部分 */
#define SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"   //服务与特征的UUID，需要和广播设备一致，需交流信息时使用
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef1"  
#define CHARACTERISTIC_UUID_2 "0c0f900e-d6fa-49de-9634-4d6e296b5155"
static boolean scan=false;//是否扫描到设备的标志位
static boolean connected=false;//判断连接是否成功与正常的标志位
static bool isLost=false;//判断RSSI值是否正常的标志位
static BLEClient *pClient=nullptr;//主机端
static BLEAddress *pServerAddress=nullptr;//static限定该变量作用域仅为该文件
static BLEAdvertisedDevice* ptargetDevice = nullptr;//包含目标连接对象的信息

static BLERemoteCharacteristic *pRemoteCharacteristic=nullptr;//特征，类
static BLERemoteCharacteristic *pRemoteCharacteristic_2=nullptr;//特征，类

static char targetName[]="linzhi8";
static int deviceRSSI=-100;
float distance=0;
//unsigned long current_time=0;
//unsigned long lastscan_time=0;
int RSSICount=0;
float RSSI_hat=0.0;

uint16_t voltage=10;

/* WiFi与MQTT部分 */
//const char *ssid="happyhappy";
//const char *password="1029384756";

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
const char *mqtt_broker="192.168.53.36";
const char* mqtt_usrname="BlueTeeth2";
const char* mqtt_password="123456";
int mqtt_port=1883;
WiFiClient espClient;//创建底层TCP连接对象
PubSubClient client(espClient);//基于TCP的MQTT客户端，传入网络对象

#endif

/* 屏幕部分 */
TFT_eSPI tft=TFT_eSPI();//创建屏幕对象，可指定对象尺寸，不指定将使用User_Setup.h中定义的尺寸
#define LCD_BL_PIN TFT_BL			// PWD 的 IO 引脚
#define LCD_BL_PWM_CHANNEL 0		// Channel  通道, 0 ~ 16，高速通道（0 ~ 7）由80MHz时钟驱动，低速通道（8 ~ 15）由 1MHz 时钟驱动

//IMU部分
#ifndef SENSOR_SDA
#define SENSOR_SDA  21
#endif

#ifndef SENSOR_SCL
#define SENSOR_SCL  22
#endif

#ifndef SENSOR_IRQ
#define SENSOR_IRQ  39
#endif

uint8_t QMI8658_I2C_ADDR_PRIMARY   = 0x18;
uint8_t QMI8658_I2C_ADDR_SECONDARY = 0x19;//先测试实际目标qmi8658设备的地址！！
SensorQMI8658 qmi;

struct IMUData{
  float acc_x;
  float acc_y;
  float acc_z;
  float gyro_x;
  float gyro_y;
  float gyro_z;
};
struct IMUData *imu;

const int buzzer=16;
/* 蓝牙通信部分 */
class MyClientCallbacks:public BLEClientCallbacks {
  void onConnect(BLEClient *pclient){
    ledcSetup(1,2000,8);
    ledcWrite(1,0);
  }
  //断联之后的回调函数
  void onDisconnect(BLEClient *pclient){
    ledcSetup(1,4000,8);
    ledcWrite(1,127);
    connected=false;//作为标志方便尝试重新连接
    Serial.println("断联:onDisconnect:Client Disconnected");
    String msg = "{\"status\":\"blueteeth has disconnected\",\"explain:\":\"totally disconnected\",\"distance\":";
    msg += String(distance);
    msg += "}";
    client.publish("/HZL2/pub", msg.c_str());
  }
};
//扫描，当任何BLE设备被发现时onResult方法会被调用
class MyAdvertisedDeviceCallbacks:public BLEAdvertisedDeviceCallbacks{
  void onResult(BLEAdvertisedDevice advertisedDevice){
    if(scan) return;
    if(advertisedDevice.haveName() && strcmp(advertisedDevice.getName().c_str(),targetName)==0){
      deviceRSSI=advertisedDevice.getRSSI();
      Serial.printf("Successfully find: %s RSSI=%d\n", advertisedDevice.getName().c_str(),deviceRSSI);
      if (ptargetDevice != nullptr) {
        delete ptargetDevice;
        ptargetDevice = nullptr;
      }
      if (pServerAddress != nullptr) {
        delete pServerAddress;
        pServerAddress = nullptr;
      }
      
      ptargetDevice = new BLEAdvertisedDevice(advertisedDevice);
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());//new分配空间给BLEAddress这个类的一个实例，通过指针指向
      scan=true;

      BLEDevice::getScan()->stop();       // 关键：真正停止扫描！
      BLEDevice::getScan()->clearResults();//?????
    }
  }
};

bool connectToServer(){
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
bool connectToServerOfService(){
  //传入的pAddress就是实际的从机地址
  // 连接到远程 BLE 服务器（从机）
  if (!pClient->connect(ptargetDevice)) {
    //Serial.println(" - Connection failed");
    return false;
  }
  Serial.println(" - Connected to server");
  BLERemoteService *pRemoteService=pClient->getService(SERVICE_UUID);
  if(pRemoteService==nullptr){
    Serial.print("Failed to find service UUID:");
    
    Serial.println(SERVICE_UUID);
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Service found");
  pRemoteCharacteristic=pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
  pRemoteCharacteristic_2=pRemoteService->getCharacteristic(CHARACTERISTIC_UUID_2);

  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find characteristic UUID: ");
    Serial.println(CHARACTERISTIC_UUID);
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Characteristic1 found");
  /*
  if (pRemoteCharacteristic) {
    Serial.print("Char UUID: ");
    Serial.println(pRemoteCharacteristic->getUUID().toString().c_str());
    Serial.print("Can read: "); Serial.println(pRemoteCharacteristic->canRead());
    Serial.print("Can notify: "); Serial.println(pRemoteCharacteristic->canNotify());
    Serial.print("Can write: "); Serial.println(pRemoteCharacteristic->canWrite());
  }
  */
  if (pRemoteCharacteristic_2 == nullptr) {
    Serial.print("Failed to find characteristic2 UUID: ");
    Serial.println(CHARACTERISTIC_UUID_2);
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Characteristic2 found");
  /*
  if (pRemoteCharacteristic_2) {
    Serial.print("Char UUID2: ");
    Serial.println(pRemoteCharacteristic_2->getUUID().toString().c_str());
    Serial.print("Can read: "); Serial.println(pRemoteCharacteristic_2->canRead());
    Serial.print("Can notify: "); Serial.println(pRemoteCharacteristic_2->canNotify());
    Serial.print("Can write: "); Serial.println(pRemoteCharacteristic_2->canWrite());
  }
  */
  connected = true;
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
  while(!connectToServerOfService()){
    delay(10);
  }
}

/* WiFi与MQTT部分 */

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

/* 显示屏部分 */
void DisplayInit(){
  // /* 配置LEDC PWM通道属性，PWD通道为 0，频率为1KHz，8位分辨率*/
    ledcSetup(LCD_BL_PWM_CHANNEL, 1000, 8);
 
	// /* 配置LEDC PWM通道属性,通道0的PWM波在LCD_BL_PIN上输出 */
    ledcAttachPin(LCD_BL_PIN, LCD_BL_PWM_CHANNEL);
 
	ledcWrite(LCD_BL_PWM_CHANNEL, (int)(1 * 255));//满亮度
  tft.init();
  tft.setRotation(0);  //设置显示图像旋转方向
  tft.invertDisplay(0);  //是否反转所有显示颜色
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,0,2);//将“光标”设置在显示器的左上角（0,0），并选择字体2 
  tft.setTextColor(TFT_WHITE,TFT_BLUE);//将字体颜色设置为白色，背景为蓝色
  tft.setTextSize(1);//将文本大小倍增设置为1
  tft.println("Hello world");
}

//IMU部分
void IMUInit(){

  pinMode(SENSOR_IRQ, INPUT);

  //Serial.println("QMI8658 Sensor Temperature");
    
  if (!qmi.begin(Wire, QMI8658_I2C_ADDR_SECONDARY, SENSOR_SDA, SENSOR_SCL)) {
    Serial.println("Failed to find QMI8658 - check your wiring!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("Init QMI8658 Sensor success!");
}
void IMUTest(){
  qmi.getAccelerometerScales();
  qmi.getAccelerometer(imu->acc_x,imu->acc_y,imu->acc_z);
  qmi.getGyroscopeScales();
  qmi.getGyroscope(imu->gyro_x,imu->gyro_y,imu->gyro_z);
  Serial.printf("acc: %f,%f,%f  gyro: %f,%f,%f",imu->acc_x,imu->acc_y,imu->acc_z,imu->gyro_x,imu->gyro_y,imu->gyro_z);

}
/* 蜂鸣器部分 */
void BuzzerInit(){
  ledcSetup(1,2000,8);
  ledcAttachPin(buzzer,1);
}
void setup() {
  Serial.begin(115200);
  Serial.println("Starting......");
  /* 蜂鸣器 */
  BuzzerInit();
  //esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  /* 蓝牙通信部分 */
  BLECommunicationInit();
  
  //显示屏部分
  //DisplayInit();

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

    //计算距离
    distance=pow(10,(-75-RSSI_hat)/23.3);

    isLost=((RSSI_hat<(-90.0)));
    Serial.printf("RSSI_hat: %f    distance: %f     ",RSSI_hat,distance);
    uint8_t temp=uint8_t(abs(RSSI_hat));
    //voltage=pRemoteCharacteristic_2->readUInt16();
    //Serial.printf("Voltage: %u\n",voltage);
    pRemoteCharacteristic->writeValue(&temp,1);
    //如果丢失进行报警
    if(isLost){
      Serial.println("Warning!!!!!");
      //tft.println("Warning!!!!");
      ledcWrite(1,127);
      String msg = "{\"status\":\"blueteeth has disconnected\",\"explain:\":\"device may loss\",\"distance\":";
      msg += String(distance);
      msg += "}";
      client.publish("/HZL2/pub", msg.c_str());
      delay(500);
    }
    else{
      Serial.println("safe");
      ledcWrite(1,0);
    }
    delay(500);
  }
  
  else{
    while(!connectToServerOfService()){
      Serial.print(".");
      delay(10);
    }
  }
}
