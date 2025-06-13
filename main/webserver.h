#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <esp_http_server.h>
#include <stddef.h>

#define MAX_WS_CLIENTS 8

// WebSocket client array
extern int ws_clients[MAX_WS_CLIENTS];

// WebServer initialization and startup
void webserver_init(void);
void start_web_server(void);
void mount_spiffs(void);

// WebSocket client notification functions
void notify_clients(const char* message);
void notify_pin_states(void);
void notify_adc_value(void);
void notify_wifi_status(void);
void notify_i2c_scan(void);
void notify_uart_read(void);
extern int ws_clients[MAX_WS_CLIENTS];

#endif // WEBSERVER_H
