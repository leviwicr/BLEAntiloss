#include "Arduino.h"

#include "BLECommunicate.h"
#include "MQTTCommunicate.h"


bool scan=false;//是否扫描到设备的标志位
bool connected=false;//判断连接是否成功与正常的标志位

BLEClient *pClient=nullptr;//主机端
BLEAddress *pServerAddress=nullptr;//目标BLE设备的地址
BLEAdvertisedDevice* ptargetDevice = nullptr;//包含目标连接对象的信息

BLERemoteCharacteristic *pRemoteCharacteristic=nullptr;//BLE协议中的特征，类
BLERemoteCharacteristic *pRemoteCharacteristic_2=nullptr;//特征，类

char targetName[]="linzhi8";//目标BLE设备的名字
//用于RSSI检测、滤波与距离估算
bool isLost=false;
int deviceRSSI=-100;
float distance=0;
int RSSICount=0;
float RSSI_hat=0.0;


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
