idf_component_register(
    SRCS "main.c"
         "gpio_manager.c"
         "wifi_manager.c"
         "uart_manager.c"
         "i2c_manager.c"
         "adc_manager.c"
         "webserver.c"
    INCLUDE_DIRS "."
    REQUIRES esp_http_server 
             esp_adc
             esp_timer
             nvs_flash
             esp_netif
             esp_wifi
             esp_event
             spiffs
             driver
             esp_system
             freertos
             soc
             hal
    PRIV_REQUIRES esp_timer esp_common
)

target_include_directories(${COMPONENT_LIB} PUBLIC 
    ${IDF_PATH}/components/driver/include
    ${IDF_PATH}/components/driver/gpio/include
    ${IDF_PATH}/components/esp_common/include
    ${IDF_PATH}/components/hal/include
    ${IDF_PATH}/components/esp_system/include
    ${IDF_PATH}/components/esp_timer/include
    ${CMAKE_BINARY_DIR}/config
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE HTTPD_WS_SUPPORT=1)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=array-parameter)
