#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

// UART config
#define UART_ID       uart0
#define BAUD_RATE     115200
#define UART_TX_PIN   0   // GP0 -> STM32 RX
#define UART_RX_PIN   1   // GP1 -> STM32 TX

#define BUF_SIZE      256

int main()
{
    // Init USB serial (for the serial monitor in your PC)
    stdio_init_all();

    // Init UART0 (for talking to the STM32)
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Wait for USB serial monitor to connect
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Pico 2 ready\n");

    char rx_buf[BUF_SIZE];
    int rx_index = 0;
    int counter = 0;

    while (1)
    {
    uart_puts(UART_ID, "Pico alive\r\n");
    printf("sent\n");
    sleep_ms(1000);
        // Only listen for incoming bytes — send reply when we get a complete message
        if (uart_is_readable(UART_ID))
        {
            char c = uart_getc(UART_ID);

            if (c == '\n')
            {
                rx_buf[rx_index] = '\0';
                printf("Pico received: %s\n", rx_buf);

                char reply[100];
                snprintf(reply, sizeof(reply), "Pico reply: %d\r\n", counter++);
                uart_puts(UART_ID, reply);
                printf("Pico sent: %s", reply);

                rx_index = 0;
            }
            else if (c != '\r')
            {
                if (rx_index < BUF_SIZE - 1)
                    rx_buf[rx_index++] = c;
            }
        }
    }

    return 0;
}