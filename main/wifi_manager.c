#include "wifi_manager.h"
#include "webserver.h"
#include <string.h>
#include <esp_log.h>

#define TAG "WiFi_Manager"

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_AP_STACONNECTED:
                ESP_LOGI(TAG, "Station connected to AP");
                notify_wifi_status();
                break;
            case WIFI_EVENT_AP_STADISCONNECTED:
                ESP_LOGI(TAG, "Station disconnected from AP");
                notify_wifi_status();
                break;
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "Connected to AP");
                notify_wifi_status();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "Disconnected from AP");
                esp_wifi_connect(); // Try to reconnect
                notify_wifi_status();
                break;
        }
    }
}

static void wifi_status_task(void* arg) {
    while (1) {
        notify_wifi_status();
        vTaskDelay(pdMS_TO_TICKS(5000)); // Update every 5 seconds
    }
}

void wifi_manager_init(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    
    // Create both AP and STA interfaces
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handler
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      NULL));

    // Configure AP
    wifi_config_t ap_config = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .channel = AP_CHANNEL,
            .password = AP_PASS,
            .max_connection = AP_MAX_CONNECTIONS,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .ssid_hidden = AP_SSID_HIDDEN
        }
    };

    // Configure STA
    wifi_config_t sta_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        }
    };

    // Set mode to APSTA (both AP and STA)
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    
    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());
}

void create_wifi_tasks(void) {
    xTaskCreatePinnedToCore(wifi_status_task, "wifi_status", 4096, NULL, 1, NULL, 1);
}

bool is_wifi_connected(void) {
    wifi_ap_record_t ap_info;
    return (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK);
}

int get_connected_stations(void) {
    wifi_sta_list_t sta_list;
    esp_wifi_ap_get_sta_list(&sta_list);
    return sta_list.num;
}

int get_wifi_rssi(void) {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        return ap_info.rssi;
    }
    return 0;
}

void get_wifi_status(char* buffer, size_t buffer_size) {
    wifi_ap_record_t ap_info;
    wifi_sta_list_t sta_list;
    wifi_mode_t mode;

    esp_wifi_get_mode(&mode);
    esp_wifi_ap_get_sta_list(&sta_list);
    bool connected = (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK);

    snprintf(buffer, buffer_size, 
        "{\"wifi\":{"
        "\"mode\":\"APSTA\","
        "\"ap\":{"
        "\"ssid\":\"%s\","
        "\"connected_stations\":%d"
        "},"
        "\"sta\":{"
        "\"connected\":%s,"
        "\"rssi\":%d"
        "}"
        "}}",
        AP_SSID,
        sta_list.num,
        connected ? "true" : "false",
        connected ? ap_info.rssi : 0
    );
}
