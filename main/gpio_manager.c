#include "gpio_manager.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "GPIO_Manager";

// Define the usable pins array
const int usable_pins[GPIO_NUM_PINS] = {
    0, 2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19,
    21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39
};

// Global state for all GPIO pins
static gpio_pin_data_t pin_data[GPIO_NUM_PINS] = {0};

// Private helper functions
static int get_pin_index(int pin) {
    for (int i = 0; i < GPIO_NUM_PINS; i++) {
        if (usable_pins[i] == pin) return i;
    }
    return GPIO_INVALID_PIN;
}

esp_err_t gpio_manager_init(void) {
    ESP_LOGI(TAG, "Initializing GPIO Manager");
    
    // Initialize all usable pins as inputs by default
    for (int i = 0; i < GPIO_NUM_PINS; i++) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << usable_pins[i]),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        esp_err_t ret = gpio_config(&io_conf);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to configure GPIO %d", usable_pins[i]);
            return ret;
        }
        pin_data[i].mode = false;  // Input mode
        pin_data[i].state = false;
        pin_data[i].pwm_frequency = 0;
        pin_data[i].bounce_count = 0;
        pin_data[i].last_change = 0;
    }
    
    return ESP_OK;
}

esp_err_t set_pin_mode(int pin, bool output) {
    int idx = get_pin_index(pin);
    if (idx == GPIO_INVALID_PIN) {
        return ESP_ERR_INVALID_ARG;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = output ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        return ret;
    }

    pin_data[idx].mode = output;
    pin_data[idx].pwm_frequency = 0;  // Reset PWM when changing mode
    return ESP_OK;
}

esp_err_t write_pin(int pin, bool value) {
    int idx = get_pin_index(pin);
    if (idx == GPIO_INVALID_PIN) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!pin_data[idx].mode) {
        return ESP_ERR_INVALID_STATE;  // Pin is not in output mode
    }

    esp_err_t ret = gpio_set_level(pin, value);
    if (ret == ESP_OK) {
        pin_data[idx].state = value;
        pin_data[idx].last_change = esp_timer_get_time();
        pin_data[idx].bounce_count++;
    }
    return ret;
}

esp_err_t get_pin_state(int pin, bool *state) {
    int idx = get_pin_index(pin);
    if (idx == GPIO_INVALID_PIN || state == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (pin_data[idx].mode) {
        *state = pin_data[idx].state;  // For output pins, return stored state
    } else {
        *state = gpio_get_level(pin);  // For input pins, read current state
    }
    return ESP_OK;
}

esp_err_t get_pin_mode(int pin, bool *is_output) {
    int idx = get_pin_index(pin);
    if (idx == GPIO_INVALID_PIN || is_output == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    *is_output = pin_data[idx].mode;
    return ESP_OK;
}

uint32_t get_pin_bounce_count(int pin) {
    int idx = get_pin_index(pin);
    if (idx == GPIO_INVALID_PIN) {
        return 0;
    }
    return pin_data[idx].bounce_count;
}

int get_pin_pwm_frequency(int pin) {
    int idx = get_pin_index(pin);
    if (idx == GPIO_INVALID_PIN) {
        return 0;
    }
    return pin_data[idx].pwm_frequency;
}

void get_all_pin_states(char* buffer, size_t buffer_size) {
    if (buffer == NULL || buffer_size == 0) {
        return;
    }

    int offset = 0;
    for (int i = 0; i < GPIO_NUM_PINS && offset < buffer_size - 1; i++) {
        bool current_state;
        if (get_pin_state(usable_pins[i], &current_state) == ESP_OK) {
            offset += snprintf(buffer + offset, buffer_size - offset, 
                "%d:%d;", usable_pins[i], current_state ? 1 : 0);
        }
    }
    buffer[buffer_size - 1] = '\0';  // Ensure null termination
}

esp_err_t create_gpio_tasks(void) {
    // Currently no background tasks needed
    return ESP_OK;
}

esp_err_t set_pwm(int pin, int freq) {
    int idx = get_pin_index(pin);
    if (idx == GPIO_INVALID_PIN) {
        return ESP_ERR_INVALID_ARG;
    }

    // Initialize LEDC for this pin with basic PWM configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,  // 13 bit duty resolution
        .freq_hz = freq,                       // Set frequency
        .clk_cfg = LEDC_AUTO_CLK              // Auto select clock source
    };

    esp_err_t ret = ledc_timer_config(&ledc_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error configuring LEDC timer: %d", ret);
        return ret;
    }

    // Configure LEDC channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = 0,  // Use channel 0
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = pin,
        .duty = 4096,  // Set initial duty to 50% (13-bit resolution: 8192/2)
        .hpoint = 0
    };

    ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error configuring LEDC channel: %d", ret);
        return ret;
    }

    // Update pin data
    pin_data[idx].mode = true;  // Consider it as output
    pin_data[idx].pwm_frequency = freq;

    ESP_LOGI(TAG, "PWM initialized on GPIO%d at %dHz", pin, freq);
    return ESP_OK;
}
