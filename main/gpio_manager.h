#ifndef GPIO_MANAGER_H
#define GPIO_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

#define GPIO_NUM_PINS 24  // Fixed size for arrays
#define GPIO_INVALID_PIN -1

// Pin data structure
typedef struct {
    bool mode;              // true for output, false for input
    bool state;            // current state (for outputs) or last read state (for inputs)
    int pwm_frequency;     // PWM frequency if enabled, 0 otherwise
    uint32_t bounce_count; // Number of state changes
    int64_t last_change;   // Time of last state change in microseconds
} gpio_pin_data_t;

// GPIO pin configuration array
extern const int usable_pins[GPIO_NUM_PINS];

// GPIO Manager Functions
esp_err_t gpio_manager_init(void);
esp_err_t set_pin_mode(int pin, bool output);
esp_err_t write_pin(int pin, bool value);
esp_err_t set_pwm(int pin, int freq);
esp_err_t get_pin_state(int pin, bool *state);
esp_err_t get_pin_mode(int pin, bool *is_output);
uint32_t get_pin_bounce_count(int pin);
int get_pin_pwm_frequency(int pin);
void get_all_pin_states(char* buffer, size_t buffer_size);

// Task creation
esp_err_t create_gpio_tasks(void);

#endif // GPIO_MANAGER_H
