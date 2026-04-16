#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/adc.h"

#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9

// Pico LED for heartbeat
#define HEARTBEAT_LED 25

void drawChar(uint16_t x, uint16_t y, char letter);
void drawMessage(uint16_t x, uint16_t y, char*m);

int main()
{
    stdio_init_all();

    //i2c initialization at 100kHz
    i2c_init(I2C_PORT, 1700*1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    // ADC init
    adc_init();
    adc_gpio_init(26);   
    adc_select_input(0); 

    char message[50];

    while (true) {
        // Pico heartbeat LED
        //gpio_init(HEARTBEAT_LED);
       // gpio_set_dir(HEARTBEAT_LED, GPIO_OUT);

        /*ssd1306_drawPixel(10,20,1);
        ssd1306_update();
        sleep_ms(1000);
        ssd1306_drawPixel(10,20,0);
        ssd1306_update();
        sleep_ms(1000);*/


        // Read ADC and convert to volts (12-bit, 3.3V reference)
        uint16_t raw = adc_read();
        float volts = raw * 3.3f / (1 << 12);
        absolute_time_t t1, t2;
        t1 = get_absolute_time();
        ssd1306_clear();

        sprintf(message, "ADC0: %.2f V", volts);
        drawMessage(0, 0, message);
        drawMessage(0, 8, message);
        drawMessage(0, 16, message);
        drawMessage(0, 24, message);

        ssd1306_update();

        // Calculate FPS from time elapsed this frame
        t2 = get_absolute_time();
        uint64_t ta;
        char speed[30];

        ta = to_us_since_boot(t2) - to_us_since_boot(t1);
        sprintf(speed, "FPS = %6.3f  ",1.0/(ta/1000000.0));
        drawMessage(0, 24, speed);
        ssd1306_update();
        sleep_ms(1000);
    }
}


void drawChar(uint16_t x, uint16_t y, char letter) {
    // ASCII table starts at 0x20 (space), so offset by 0x20
    if (letter < 0x20 || letter > 0x7f) return;
    const char *glyph = ASCII[letter - 0x20];
    for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 8; row++) {
            unsigned char pixel = (glyph[col] >> row) & 0x01;
            ssd1306_drawPixel(x + col, y + row, pixel);
        }
    }
}

void drawMessage(uint16_t x, uint16_t y, char *m) {
    int i = 0;
    while (m[i] != '\0') {
        drawChar(x + i * 5, y, m[i]);
        i++;
    }
}