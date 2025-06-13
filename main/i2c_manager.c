#include "i2c_manager.h"
#include <driver/i2c.h>
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"

void i2c_manager_init(void) {
    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA,
        .scl_io_num = I2C_MASTER_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };
    i2c_param_config(I2C_PORT, &i2c_cfg);
    i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
}

void scan_i2c(char* out, size_t max_len) {
    size_t pos = 0;
    pos += snprintf(out + pos, max_len - pos, "{\"i2c\":[");
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(20));
        i2c_cmd_link_delete(cmd);
        if (ret == ESP_OK) {
            pos += snprintf(out + pos, max_len - pos, "%d,", addr);
        }
    }
    if (pos > 7 && out[pos - 1] == ',') pos--;  // Remove trailing comma
    snprintf(out + pos, max_len - pos, "]}");
}
