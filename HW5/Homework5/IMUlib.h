#ifndef IMULIB_H
#define IMULIB_H

#include <stdio.h>
#include <stdint.h>

// IMU I2C address
#define IMU_ADDR     0x68

// config registers
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C
// sensor data registers:
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H   0x41
#define TEMP_OUT_L   0x42
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48
#define WHO_AM_I     0x75

//defining a structure to store IMU data
typedef struct {
    float accel_x, accel_y, accel_z; // g
    float gyro_x,  gyro_y,  gyro_z; // degrees/s
    float temp; // degrees C
} imu_data_t;

//write to IMU
void imu_write_register(uint8_t reg, uint8_t value);
// read from IMU
uint8_t imu_read_register(uint8_t reg);
// read from multiple IMU registors (to fill struct imu_date_t)
void imu_read_all(imu_data_t *data);

#endif // IMULIB_H
