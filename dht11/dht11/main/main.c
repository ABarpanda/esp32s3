#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dht.h"
#include "esp_log.h"
 
#define TAG "DHT_APP"
#define OUT_GPIO GPIO_NUM_2

void app_main(void) {
    int16_t temperature = 0;
    int16_t humidity = 0;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << OUT_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    while (1) {
        esp_err_t res1 = dht_read_data(DHT_TYPE_DHT11, GPIO_NUM_4, &humidity, &temperature);
        if (res1 == ESP_OK) {
            ESP_LOGI(TAG, "Humidity: %d%% Temp: %d°C from sensor 1", humidity/10, temperature/10);
        } else {
            ESP_LOGE(TAG, "Could not read data from sensor 1");
        }
        if (humidity/10 > 90){
            ESP_LOGI("GPIO", "Setting GPIO3 HIGH");
            gpio_set_level(OUT_GPIO, 1); // Set HIGH_NUM_2
            // vTaskDelay(pdMS_TO_TICKS(1000));
        }
        if (humidity/10<90){
            ESP_LOGI("GPIO", "Setting GPIO3 LOW");
            gpio_set_level(OUT_GPIO, 0); // Set LOW_NUM_2
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
