#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

// ---------------- AS5600 (encoder) ----------------
#define AS5600_ADDR        0x36
#define AS5600_REG_ANGLE_H 0x0E
#define AS5600_I2C         i2c0
#define AS5600_SDA_PIN     16
#define AS5600_SCL_PIN     17

// ---------------- UART link to STM32 ----------------
#define LINK_UART   uart0
#define LINK_TX_PIN 0    // GP0 -> STM32 PB7 (RX)
#define LINK_RX_PIN 1    // GP1 -> STM32 PB6 (TX), optional
#define LINK_BAUD   115200

// ---------------- Bump effect parameters ----------------
#define BUMP_PEAK   100.0f   // peak current in mA at center of bump
#define BUMP_CENTER 3000.0f  // center angle (0-4095), 2048 = middle
#define BUMP_WIDTH  50.0f   // width of the bump in angle counts

// ================= AS5600 functions =================
void as5600_init(void) {
    i2c_init(AS5600_I2C, 400000);
    gpio_set_function(AS5600_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(AS5600_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(AS5600_SDA_PIN);
    gpio_pull_up(AS5600_SCL_PIN);
}

uint16_t as5600_read_angle(void) {
    uint8_t reg = AS5600_REG_ANGLE_H;
    uint8_t buf[2];
    i2c_write_blocking(AS5600_I2C, AS5600_ADDR, &reg, 1, true);
    i2c_read_blocking(AS5600_I2C, AS5600_ADDR, buf, 2, false);
    return ((buf[0] & 0x0F) << 8) | buf[1];
}

// ================= Bump effect =================
// Gaussian bump: resistive current peaks at BUMP_CENTER
float compute_bump_current(uint16_t angle) {
    float d = (float)angle - BUMP_CENTER;
    float current = BUMP_PEAK * expf(-(d * d) / (BUMP_WIDTH * BUMP_WIDTH));
    return current;
}

// ================= Main =================
int main(void) {
    stdio_init_all();
    as5600_init();

    // Set up UART link to STM32
    uart_init(LINK_UART, LINK_BAUD);
    gpio_set_function(LINK_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(LINK_RX_PIN, GPIO_FUNC_UART);

    while (1) {
        uint16_t angle = as5600_read_angle();
        float desired = compute_bump_current(angle);

        // Send to STM32 as a number followed by newline
        char msg[16];
        int len = snprintf(msg, sizeof(msg), "%d\n", (int)desired);
        uart_write_blocking(LINK_UART, (const uint8_t *)msg, len);

        // Also print to USB for debugging
        printf("angle: %u  desired: %d\n", angle, (int)desired);

        sleep_ms(10);  // ~100Hz update rate
    }

    return 0;
}