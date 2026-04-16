#include "hardware/i2c.h"
#include "IMUlib.h"

// function to write to IMU
void imu_write_register(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(i2c0, IMU_ADDR, buf, 2, false);
}

// function to read from IMU registor
uint8_t imu_read_register(uint8_t reg) {
    uint8_t value;
    i2c_write_blocking(i2c0, IMU_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c0, IMU_ADDR, &value, 1, false);
    return value;
}

// function to read from IMU registors to fill struct imu_data_t
void imu_read_all(imu_data_t *data) {
    uint8_t reg = ACCEL_XOUT_H;
    uint8_t buf[14];

    // Burst read 14 bytes starting from ACCEL_XOUT_H
    i2c_write_blocking(i2c0, IMU_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c0, IMU_ADDR, buf, 14, false);

    // Recombine bytes into signed 16-bit integers
    int16_t raw_ax = (int16_t)((buf[0]  << 8) | buf[1]);
    int16_t raw_ay = (int16_t)((buf[2]  << 8) | buf[3]);
    int16_t raw_az = (int16_t)((buf[4]  << 8) | buf[5]);
    int16_t raw_t  = (int16_t)((buf[6]  << 8) | buf[7]);
    int16_t raw_gx = (int16_t)((buf[8]  << 8) | buf[9]);
    int16_t raw_gy = (int16_t)((buf[10] << 8) | buf[11]);
    int16_t raw_gz = (int16_t)((buf[12] << 8) | buf[13]);

    // Scale to physical units
    data->accel_x = raw_ax * 0.000061f; // g
    data->accel_y = raw_ay * 0.000061f;
    data->accel_z = raw_az * 0.000061f;
    data->gyro_x  = raw_gx * 0.007630f; // degrees/s
    data->gyro_y  = raw_gy * 0.007630f;
    data->gyro_z  = raw_gz * 0.007630f;
    data->temp    = raw_t  / 340.00f + 36.53f; // degrees C
}