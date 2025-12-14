#include <Arduino.h>
#include "BLECommunicate.h"
#include "MQTTCommunicate.h"
#include "DisplayUse.h"
#include "QMI8658Use.h"

uint16_t voltage=10;//接受对方的电压值（作为特征值在BLE通信服务特征中）

const int buzzer=16;
const int adc=36;
float my_voltage=0;
const int charge=34;//检测电池是否在充电的端口
bool energy_full=true;


void setup() {
  Serial.begin(115200);
  Serial.println("Starting......");
  /* 蜂鸣器 */
  ledcSetup(1,2000,8);
  ledcAttachPin(buzzer,1);
  //esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  /* 蓝牙通信部分 */
  BLECommunicationInit();
  
  //显示屏部分
  //DisplayInit();

  /* WiFi与MQTT部分*/
  MQTTInit();

  pinMode(charge,INPUT_PULLUP);
  pinMode(26,INPUT_PULLUP);

}
//无法处理连接断开情况，考虑如何解决
void loop() {
  if(connected){
    if(!client.connected()) connectToMQTT();//后续再考虑两者互不堵塞
    client.loop();
    //RSSI值滤波
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
    my_voltage=(((float)(analogRead(adc)))/4095)*3.3;
    if (digitalRead(charge)==0){
      Serial.printf("充电中：my_voltage is %f\n",my_voltage);
    }else{
      Serial.printf("电池供电中：my_voltage is %f\n",my_voltage);
      if(my_voltage<=1.5) energy_full=false;
    }
    uint8_t a=digitalRead(charge);
    Serial.printf("charge or not: %d\n",digitalRead(charge));
    pRemoteCharacteristic->writeValue(&temp,1);
    //pRemoteCharacteristic->writeValue(&a,1);
    String msg = "{\"status\":\"blueteeth connected normally\",\"explain:\":\"device zc\",\"distance\":";
    msg += String(distance);
    msg += "}";
    client.publish("/HZL2/pub", msg.c_str());
    delay(500);
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
    if(!energy_full){
      String msg = "{\"status\":\"battery is low !!!\"}";
      client.publish("/HZL2/pub", msg.c_str());
    }
    else{
      String msg = "{\"status\":\"battery is fully charged. \"}";
      client.publish("/HZL2/pub", msg.c_str());
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
