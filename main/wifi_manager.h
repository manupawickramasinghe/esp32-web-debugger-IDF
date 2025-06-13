#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_netif.h>

// WiFi configuration
#define WIFI_SSID "Nokia Repeater"
#define WIFI_PASS "12345678"

// AP Configuration
#define AP_SSID "ESP32-Debugger"
#define AP_PASS "12345678"
#define AP_CHANNEL 1
#define AP_MAX_CONNECTIONS 4
#define AP_SSID_HIDDEN 0

// WiFi initialization and task management
void wifi_manager_init(void);
void create_wifi_tasks(void);

// WiFi status functions
void get_wifi_status(char* buffer, size_t buffer_size);
bool is_wifi_connected(void);
int get_connected_stations(void);
int get_wifi_rssi(void);

#endif // WIFI_MANAGER_H
