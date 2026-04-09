#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

// MCP23008 I2C address: 0b0100000. In hex, 0x20
#define ADDR  0x20

// MCP23008 register addresses
#define REG_IODIR 0x00  // I/O direction:  1=input, 0=output
#define REG_GPPU 0x06  // Pull-up config: 1=100k pull-up enabled
#define REG_GPIO 0x09  
#define REG_OLAT 0x0A  // Output latch (write to set output pins)

// Pico LED for heartbeat
#define HEARTBEAT_LED 25

//function prototypes:
// Write a byte to an MCP23008 register
void mcp_write(uint8_t reg, uint8_t value);
// Read a byte from an MCP23008 register
uint8_t mcp_read(uint8_t reg);
// Set/clear a single output pin on the MCP23008 (pin 0-7)
void mcp_set_pin(uint8_t pin, uint8_t state);
// Read a single pin from the MCP23008 (returns 0 or 1)
uint8_t mcp_get_pin(uint8_t pin);


int main() {
    stdio_init_all();

    // I2C init at 400 kHz
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Pico heartbeat LED
    gpio_init(HEARTBEAT_LED);
    gpio_set_dir(HEARTBEAT_LED, GPIO_OUT);

    // MCP23008 init
    // IODIR: GP7=output (bit7=0), all others=input (bits 0-6 = 1). In hex = 0x7F
    mcp_write(REG_IODIR, 0x7F);

    // GPPU: enable internal pull-up on GP0 (bit0=1) so the button pulls the pin high when not pressed, low when pressed to GND
    mcp_write(REG_GPPU, 0x01);

    // OLAT: start with GP7 LED off
    mcp_write(REG_OLAT, 0x00);

    bool heartbeat = false;

    while (true) {
        // GP0 is active-low: reads 0 when button is pressed (pulled to GND)
        uint8_t button_pressed = (mcp_get_pin(0) == 0);

        // Drive GP7 LED based on button
        mcp_set_pin(7, button_pressed ? 1 : 0);

        // Heartbeat: toggle onboard LED so we know Pico isn't frozen
        heartbeat = !heartbeat; // turns on/off every iteration (100ms)
        gpio_put(HEARTBEAT_LED, heartbeat);

        sleep_ms(100);
    }
}

//helper functions below:

// Write a byte to an MCP23008 register
void mcp_write(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(I2C_PORT, ADDR, buf, 2, false);
}

// Read a byte from an MCP23008 register
uint8_t mcp_read(uint8_t reg) {
    uint8_t value;
    i2c_write_blocking(I2C_PORT, ADDR, &reg, 1, true);  // true = keep bus
    i2c_read_blocking(I2C_PORT, ADDR, &value, 1, false);
    return value;
}

// Set/clear a single output pin on the MCP23008 (pin 0-7)
void mcp_set_pin(uint8_t pin, uint8_t state) {
    uint8_t olat = mcp_read(REG_OLAT);
    if (state) {
        olat |=  (1 << pin);
    } else {
        olat &= ~(1 << pin);
    }
    mcp_write(REG_OLAT, olat);
}

// Read a single pin from the MCP23008 (returns 0 or 1)
uint8_t mcp_get_pin(uint8_t pin) {
    return (mcp_read(REG_GPIO) >> pin) & 0x01;
}