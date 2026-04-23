#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define PIN_CS_RAM 17
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS_DAC 20
#define PIN_SCK 18
#define PIN_MOSI 19



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

void update_dac(uint8_t channel, float voltage);
void update_dac_from_ram(int);
void spi_ram_init();
void spi_ram_write(uint16_t, uint8_t *, int);
void spi_ram_read(uint16_t, uint8_t *, int);

void spi_write_sine():


void writeDAC(int channel, float v) {
    uint16_t myV = (uint16_t)(v / 3.3f * 1023.0f);
    if (myV > 1023) myV = 1023;  // clamp

    uint16_t word = 0;
    word |= ((channel & 0x1) << 15); // channel select
    word |= (0 << 14); // BUF = 0
    word |= (1 << 13); // GA = 1 (1x)
    word |= (1 << 12); // SHDN = 1 (active)
    word |= (myV << 2); // 10-bit value

    uint8_t data[2];
    data[0] = (word >> 8) & 0xFF;
    data[1] = word & 0xFF;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}

int main() {
    stdio_init_all();

    spi_init(SPI_PORT, 1000 * 1000 * 20);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_DAC, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);
    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_put(PIN_CS_RAM, 1);

    spi_ram_init();
    ram_write_sine();
    int i = 0;


    float t = 0.0f;

    while (true) {
        // 2Hz sine on channel 0, range 0-3.3V
        float sine_v = (sinf(2.0f * 3.14159f * 2.0f * t) + 1.0f) / 2.0f * 3.3f;
        writeDAC(0, sine_v);

        // 1Hz triangle on channel 1, range 0-3.3V
        float phase = fmodf(t, 1.0f); // 0..1 over 1 second
        float tri_v = (phase < 0.5f)
            ? (phase * 2.0f) * 3.3f // rising
            : ((1.0f - phase) * 2.0f) * 3.3f;  // falling
        writeDAC(1, tri_v);

        t += 0.01f;  // 10ms step
        sleep_ms(10);
    }
}

void spi_init_ram(){
    uint data[2];
    int len = 2;
    data[0] = 0b00000001;
    data[1] = 0b01000000;
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, data, len);
    cs_deselect(PIN_CS_RAM);

}

void spi_ram_write(uint16_t addr, uint8_t * data, int len){
    uint8_t packet[5];
    packet[0] = 0b00000010;
    packet[1] = addr<<8;
    packet[2] = addr&0xFF;
    packet[3] = data[0];
    packet[4] = data[1];
}