#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define PIN_CS_RAM 7
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS_DAC 17
#define PIN_SCK 18
#define PIN_MOSI 19

union FloatBytes {
    float f;
    uint8_t bytes[4];
};

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}

void update_dac_from_ram(int);
void spi_ram_init();
void spi_ram_write(uint16_t, uint8_t *, int);
void spi_ram_read(uint16_t, uint8_t *, int);
void ram_write_sine();
void writeDAC(int channel, float v);

void writeDAC(int channel, float v) {
    uint16_t myV = (uint16_t)(v / 3.3f * 1023.0f);
    if (myV > 1023) myV = 1023;

    uint16_t word = 0;
    word |= ((channel & 0x1) << 15);
    word |= (0 << 14);
    word |= (1 << 13);
    word |= (1 << 12);
    word |= (myV << 2);

    uint8_t data[2];
    data[0] = (word >> 8) & 0xFF;
    data[1] = word & 0xFF;

    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS_DAC);
}

int main() {
    stdio_init_all();

    spi_init(SPI_PORT, 1000 * 1000 * 2);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS_DAC);
    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);

    gpio_init(PIN_CS_RAM);
    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_put(PIN_CS_RAM, 1);

    spi_ram_init();
    ram_write_sine();

    while (true) {
        for (int i = 0; i < 1000; i++) {
            update_dac_from_ram(i * 4);
            sleep_ms(1);
        }
        /*for (int i = 0; i < 10; i++) {
            union FloatBytes fb_write, fb_read;

            fb_write.f = (float)i * 0.5f;

            spi_ram_write(i * 4, fb_write.bytes, 4);
            spi_ram_read(i * 4, fb_read.bytes, 4);

            printf("addr %d: wrote %f read %f\n", i*4, fb_write.f, fb_read.f);
        }*/
        // write a known value
        /*union FloatBytes fb_write;
        fb_write.f = 1.23f;
        spi_ram_write(0, fb_write.bytes, 4);

        // read it back
        union FloatBytes fb_read;
        spi_ram_read(0, fb_read.bytes, 4);

        printf("wrote: %f, read back: %f\n", fb_write.f, fb_read.f);*?
    }
    /*while (true) {
    for (int i = 0; i < 1000; i++) {
        float v = (sinf(2.0f * 3.14159f * i / 1000.0f) + 1.0f) / 2.0f * 3.3f;
        writeDAC(0, v);
        sleep_ms(1);
    }*/
    }
}


void spi_ram_init() {
    uint8_t data[2];
    data[0] = 0b00000001;
    data[1] = 0b01000000;
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS_RAM);
}

void spi_ram_write(uint16_t addr, uint8_t *data, int len) {
    uint8_t packet[3];
    packet[0] = 0b00000010;
    packet[1] = (addr >> 8) & 0xFF;
    packet[2] = addr & 0xFF;

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, packet, 3);
    spi_write_blocking(SPI_PORT, data, len);
    cs_deselect(PIN_CS_RAM);
}

void spi_ram_read(uint16_t addr, uint8_t *data, int len) {
    uint8_t packet[3];
    packet[0] = 0b00000011;
    packet[1] = (addr >> 8) & 0xFF;
    packet[2] = addr & 0xFF;

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, packet, 3);
    spi_read_blocking(SPI_PORT, 0, data, len);
    cs_deselect(PIN_CS_RAM);
}

void ram_write_sine() {
    uint16_t addr = 0;

    for (int i = 0; i < 1000; i++) {
        union FloatBytes fb;
        fb.f = (sinf(2.0f * 3.14159f * i / 1000.0f) + 1.0f) / 2.0f * 3.3f;
        spi_ram_write(addr, fb.bytes, 4);
        addr = addr + 4;
    }
}

void update_dac_from_ram(int i) {
    union FloatBytes fb;
    spi_ram_read(i, fb.bytes, 4);
    writeDAC(0, fb.f);
}