#include "webserver.h"
#include "gpio_manager.h"
#include "uart_manager.h"
#include "i2c_manager.h"
#include "adc_manager.h"
#include <esp_spiffs.h>
#include <esp_log.h>
#include <string.h>
#include <esp_wifi.h>

#define TAG "WebServer"

static httpd_handle_t server = NULL;
int ws_clients[MAX_WS_CLIENTS];  // Made non-static as it's declared extern in header

void notify_clients(const char* message) {
    for (int i = 0; i < MAX_WS_CLIENTS; i++) {
        if (ws_clients[i] >= 0) {
            httpd_ws_frame_t frame = {
                .type = HTTPD_WS_TYPE_TEXT,
                .payload = (uint8_t *)message,
                .len = strlen(message),
                .final = true
            };
            httpd_ws_send_frame_async(server, ws_clients[i], &frame);
        }
    }
}

void notify_pin_states(void) {
    char buffer[2048];
    get_all_pin_states(buffer, sizeof(buffer));
    notify_clients(buffer);
}

void notify_adc_value(void) {
    char json[64];
    snprintf(json, sizeof(json), 
        "{\"oscilloscope\":{\"pin\":%d,\"voltage\":%.2f}}", 
        get_oscillo_pin(), get_last_voltage());
    notify_clients(json);
}

void notify_wifi_status(void) {
    wifi_ap_record_t ap_info;
    wifi_sta_list_t sta_list;
    char json[256];
    wifi_mode_t mode;

    esp_wifi_get_mode(&mode);
    esp_wifi_ap_get_sta_list(&sta_list);
    esp_wifi_sta_get_ap_info(&ap_info);

    snprintf(json, sizeof(json), 
        "{\"wifi\":{"
        "\"mode\":\"APSTA\","
        "\"ap\":{"
        "\"ssid\":\"ESP32-Debugger\","
        "\"connected_stations\":%d"
        "},"
        "\"sta\":{"
        "\"connected\":%s,"
        "\"rssi\":%d"
        "}"
        "}}",
        sta_list.num,
        ap_info.rssi != 0 ? "true" : "false",
        ap_info.rssi
    );
    
    notify_clients(json);
}

void notify_i2c_scan(void) {
    char buf[128];
    scan_i2c(buf, sizeof(buf));
    notify_clients(buf);
}

void notify_uart_read(void) {
    char buffer[128];
    uart_read(buffer, sizeof(buffer));
    char json[256];
    snprintf(json, sizeof(json), "{\"uart\":\"%s\"}", buffer);
    notify_clients(json);
}

static esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        int sockfd = httpd_req_to_sockfd(req);
        for (int i = 0; i < MAX_WS_CLIENTS; i++) {
            if (ws_clients[i] < 0) {
                ws_clients[i] = sockfd;
                break;
            }
        }
        ESP_LOGI(TAG, "WebSocket client connected: sockfd=%d", sockfd);
        notify_pin_states();
        return ESP_OK;
    }

    httpd_ws_frame_t frame = { .type = HTTPD_WS_TYPE_TEXT, .payload = NULL };
    esp_err_t ret = httpd_ws_recv_frame(req, &frame, 0);
    if (ret != ESP_OK || frame.len == 0) return ret;

    frame.payload = malloc(frame.len + 1);
    if (!frame.payload) return ESP_ERR_NO_MEM;

    ret = httpd_ws_recv_frame(req, &frame, frame.len);
    if (ret != ESP_OK) {
        free(frame.payload);
        return ret;
    }

    frame.payload[frame.len] = '\0';
    char* msg = (char*)frame.payload;

    if (strncmp(msg, "MODE:", 5) == 0) {
        int pin = atoi(msg + 5);
        char* mode = strchr(msg + 5, ',');
        if (mode) set_pin_mode(pin, strstr(mode, "OUTPUT") != NULL);
        notify_pin_states();
    } else if (strncmp(msg, "WRITE:", 6) == 0) {
        int pin = atoi(msg + 6);
        int val = atoi(strchr(msg + 6, ',') + 1);
        write_pin(pin, val);
        notify_pin_states();
    } else if (strncmp(msg, "PWM:", 4) == 0) {
        int pin = atoi(msg + 4);
        int freq = atoi(strchr(msg + 4, ',') + 1);
        set_pwm(pin, freq);
        notify_pin_states();
    } else if (strncmp(msg, "OSCILLO:", 8) == 0) {
        set_oscillo_pin(atoi(msg + 8));
    } else if (strncmp(msg, "UART_SEND:", 10) == 0) {
        uart_send(msg + 10);
    } else if (strcmp(msg, "I2C_SCAN") == 0) {
        notify_i2c_scan();
    } else if (strcmp(msg, "UART_READ") == 0) {
        notify_uart_read();
    }

    free(frame.payload);
    return ESP_OK;
}

static esp_err_t index_handler(httpd_req_t *req) {
    FILE* f = fopen("/spiffs/index.html", "r");
    if (!f) return httpd_resp_send_404(req);

    char line[128];
    httpd_resp_set_type(req, "text/html");
    while (fgets(line, sizeof(line), f)) {
        httpd_resp_sendstr_chunk(req, line);
    }
    fclose(f);
    return httpd_resp_sendstr_chunk(req, NULL);
}

static esp_err_t static_file_handler(httpd_req_t *req) {
    char filepath[600];
    snprintf(filepath, sizeof(filepath), "/spiffs%s", req->uri);

    FILE* f = fopen(filepath, "r");
    if (!f) return httpd_resp_send_404(req);

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        httpd_resp_sendstr_chunk(req, line);
    }
    fclose(f);
    return httpd_resp_sendstr_chunk(req, NULL);
}

void mount_spiffs(void) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 8,
        .format_if_mount_failed = true
    };
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));
}

void webserver_init(void) {
    memset(ws_clients, -1, sizeof(ws_clients));
    mount_spiffs();
}

void start_web_server(void) {
    static httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(httpd_start(&server, &config));

    static httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler
    };
    httpd_register_uri_handler(server, &index_uri);

    static httpd_uri_t static_handler = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = static_file_handler
    };
    httpd_register_uri_handler(server, &static_handler);

    static httpd_uri_t ws_uri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = ws_handler,
        .user_ctx = NULL,
        .is_websocket = true,
        .handle_ws_control_frames = false,
        .supported_subprotocol = NULL
    };
    httpd_register_uri_handler(server, &ws_uri);
}
