// ESP32 Web Debugger — Pure ESP-IDF Rewrite
// Supports GPIO, PWM, ADC, UART, I2C, WebSocket, SPIFFS Web UI
#include "sdkconfig.h"
#include <stdio.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_manager.h"
#include "wifi_manager.h"
#include "uart_manager.h"
#include "i2c_manager.h"
#include "adc_manager.h"
#include "webserver.h"

#define TAG "WebDebug"

static void network_init_task(void* parameter) {
    // Initialize WiFi first
    wifi_manager_init();
    vTaskDelay(pdMS_TO_TICKS(2000));  // Wait for WiFi to stabilize
    
    // Start the web server
    start_web_server();
    
    // Create WiFi status monitoring task
    create_wifi_tasks();
    
    // Keep the network task running
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {
    // Initialize NVS flash for settings storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize all hardware and software subsystems
    webserver_init();  // Must be first to initialize data structures
    gpio_manager_init();
    uart_manager_init();
    i2c_manager_init();
    adc_manager_init();

    // Create real-time tasks on Core 0 (lower priority numbers mean higher priority)
    create_gpio_tasks();     // Priority 5 - GPIO debouncing
    create_adc_tasks();      // Priority 5 - ADC sampling

    // Create network-related task on Core 1
    xTaskCreatePinnedToCore(
        network_init_task,   // Task function
        "network_init",      // Task name for debugging
        8192,               // Stack size (bytes)
        NULL,               // No parameters needed
        1,                  // Lower priority for network tasks
        NULL,               // No need to store task handle
        1                   // Run on Core 1
    );

    ESP_LOGI(TAG, "ESP32 Web Debugger initialized successfully");
}
