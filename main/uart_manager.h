#ifndef UART_MANAGER_H
#define UART_MANAGER_H

#include <driver/uart.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define UART_PORT UART_NUM_1
#define UART_TX 17
#define UART_RX 16
#define UART_BUF_SIZE 1024

void uart_manager_init(void);
void uart_send(const char* str);
void uart_read(char* out, size_t max_len);
void uart_task_create(void);

#endif // UART_MANAGER_H
