#ifndef BLECOMMUNICATE_H
#define BLECOMMUNICATE_H

#include <BLEDevice.h>


#define SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"   //服务与特征的UUID，需要和广播设备一致，需交流信息时使用
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef1"  
#define CHARACTERISTIC_UUID_2 "0c0f900e-d6fa-49de-9634-4d6e296b5155"
//变量声明
extern bool scan;
extern bool connected;
extern BLEClient *pClient;
extern BLEAddress *pServerAddress;
extern BLEAdvertisedDevice* ptargetDevice;
extern BLERemoteCharacteristic *pRemoteCharacteristic;
extern BLERemoteCharacteristic *pRemoteCharacteristic_2;
extern char targetName[];
extern bool isLost;
extern int deviceRSSI;
extern float distance;
extern int RSSICount;
extern float RSSI_hat;

//类与函数的声明
class MyClientCallbacks;
class MyAdvertisedDeviceCallbacks;
bool connectToServer();
bool connectToServerOfService();
void BLECommunicationInit();

#endif
