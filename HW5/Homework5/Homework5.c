#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "IMUlib.h"

#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

#define LED_PIN 25

#define SCREEN_W 128
#define SCREEN_H 32
#define CX (SCREEN_W / 2) // 64
#define CY (SCREEN_H / 2) // 16

// Scale factor: 1g maps to this many pixels
// Max half-dimension is 16px (height), so 1g -> 14px leaves a small margin
#define ACCEL_SCALE 14.0f

// function to draw a line
void drawLine(int x0, int y0, int x1, int y1) {
    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (1) {
        ssd1306_drawPixel(x0, y0, 1);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}


int main() {
    stdio_init_all();

    // heart beat LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    // I2C — 400kHz 
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    sleep_ms(100);

    // initialize OLED
    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    // initialize IMU
    uint8_t who = imu_read_register(WHO_AM_I);
    if (who != 0x68 && who != 0x98) {
        gpio_put(LED_PIN, 1);
        while (true) {}
    }

    imu_write_register(PWR_MGMT_1,   0x00); // wake
    imu_write_register(ACCEL_CONFIG, 0x00); // +/-2g
    imu_write_register(GYRO_CONFIG,  0x18); // +/-2000 dps

    imu_data_t data;

    while (true) {
        imu_read_all(&data);

        // data.accel_x is positive when tilted right (+X wing down)
        // data.accel_y is positive when tilted forward (+Y nose down)
        // We want the line to point in the direction gravity pulls,
        // so map X tilt -> horizontal pixel offset,
        // Y tilt -> vertical pixel offset (Y axis inverted for screen coords)
        int tip_x = CX + (int)(data.accel_x * ACCEL_SCALE);
        int tip_y = CY - (int)(data.accel_y * ACCEL_SCALE);

        // Clamp to screen
        if (tip_x < 0) tip_x = 0;
        if (tip_x >= SCREEN_W) tip_x = SCREEN_W - 1;
        if (tip_y < 0) tip_y = 0;
        if (tip_y >= SCREEN_H) tip_y = SCREEN_H - 1;

        ssd1306_clear();

        // Draw crosshair at center so the origin is always visible
        ssd1306_drawPixel(CX,     CY,     1);
        ssd1306_drawPixel(CX - 1, CY,     1);
        ssd1306_drawPixel(CX + 1, CY,     1);
        ssd1306_drawPixel(CX,     CY - 1, 1);
        ssd1306_drawPixel(CX,     CY + 1, 1);

        // Draw gravity vector line
        drawLine(CX, CY, tip_x, tip_y);

        ssd1306_update();
        // No sleep — run as fast as possible for responsive updates
    }
}