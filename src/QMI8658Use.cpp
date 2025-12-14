#include "QMI8658Use.h"

uint8_t QMI8658_I2C_ADDR_PRIMARY   = 0x18;
uint8_t QMI8658_I2C_ADDR_SECONDARY = 0x19;//先测试实际目标qmi8658设备的地址！！
SensorQMI8658 qmi;
struct IMUData *imu;

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
