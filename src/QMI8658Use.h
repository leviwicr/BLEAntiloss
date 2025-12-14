#ifndef QMI8658USE_H
#define QMI8658USE_H

#include "SensorQMI8658.hpp"

#ifndef SENSOR_SDA
#define SENSOR_SDA  21
#endif
#ifndef SENSOR_SCL
#define SENSOR_SCL  22
#endif
#ifndef SENSOR_IRQ
#define SENSOR_IRQ  39
#endif

extern uint8_t QMI8658_I2C_ADDR_PRIMARY;
extern uint8_t QMI8658_I2C_ADDR_SECONDARY;
extern SensorQMI8658 qmi;

struct IMUData{
  float acc_x;
  float acc_y;
  float acc_z;
  float gyro_x;
  float gyro_y;
  float gyro_z;
};
extern struct IMUData *imu;

#endif
