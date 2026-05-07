#include <stdio.h>
#include "pico/stdlib.h"

#define BTN_UP      2
#define BTN_DOWN    3
#define BTN_START   4

void setup_button(int pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

int main() {

    stdio_init_all();

    setup_button(BTN_UP);
    setup_button(BTN_DOWN);
    setup_button(BTN_START);

    sleep_ms(2000);

    int prev_up = 1;
    int prev_down = 1;
    int prev_start = 1;

    while (1) {

        int up = gpio_get(BTN_UP);
        int down = gpio_get(BTN_DOWN);
        int start = gpio_get(BTN_START);

        // Detect new button press

        if (!up && prev_up) {
            printf("U\n");
        }

        if (!down && prev_down) {
            printf("D\n");
        }

        if (!start && prev_start) {
            printf("S\n");
        }

        prev_up = up;
        prev_down = down;
        prev_start = start;

        sleep_ms(40);
    }
}