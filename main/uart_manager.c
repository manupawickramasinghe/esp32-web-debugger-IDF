#include "uart_manager.h"
#include <driver/uart.h>
#include <string.h>

void uart_manager_init(void) {
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_PORT, 1024, 0, 0, NULL, 0);
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void uart_send(const char* str) {
    uart_write_bytes(UART_PORT, str, strlen(str));
}

void uart_read(char* out, size_t max_len) {
    int len = 0;
    uint8_t byte;
    while (uart_read_bytes(UART_PORT, &byte, 1, 10 / portTICK_PERIOD_MS) > 0 && len < max_len - 1) {
        out[len++] = byte;
    }
    out[len] = '\0';
}
