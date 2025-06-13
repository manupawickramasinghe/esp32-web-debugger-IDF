#ifndef ADC_MANAGER_H
#define ADC_MANAGER_H

#include <esp_adc/adc_oneshot.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define OSCILLO_ADC_CHANNEL ADC_CHANNEL_6  // GPIO34 = ADC1_CH6

void adc_manager_init(void);
void create_adc_tasks(void);
float get_last_voltage(void);
void set_oscillo_pin(int pin);
int get_oscillo_pin(void);
void notify_adc_value(void);

#endif // ADC_MANAGER_H
