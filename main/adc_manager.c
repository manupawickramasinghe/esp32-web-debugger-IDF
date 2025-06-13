#include "adc_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>

static adc_oneshot_unit_handle_t adc_handle;
static int oscillo_pin = 34;
static float last_voltage = 0.0f;
static const float smoothing_factor = 0.3f;

static void oscilloscope_task(void* arg) {
    while (1) {
        int raw = 0;
        adc_oneshot_read(adc_handle, OSCILLO_ADC_CHANNEL, &raw);
        float voltage = raw * 3.3f / 4095.0f;
        last_voltage = last_voltage * (1.0f - smoothing_factor) + voltage * smoothing_factor;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void adc_manager_init(void) {
    adc_oneshot_unit_init_cfg_t adc_cfg = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&adc_cfg, &adc_handle);
    adc_oneshot_config_channel(adc_handle, OSCILLO_ADC_CHANNEL, &(adc_oneshot_chan_cfg_t){
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12
    });
}

void create_adc_tasks(void) {
    xTaskCreatePinnedToCore(oscilloscope_task, "oscilloscope_task", 4096, NULL, 5, NULL, 0);
}

float get_last_voltage(void) {
    return last_voltage;
}

void set_oscillo_pin(int pin) {
    oscillo_pin = pin;
}

int get_oscillo_pin(void) {
    return oscillo_pin;
}
