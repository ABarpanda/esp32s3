#include "dht.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

#define TAG "DHT"

static int dht_wait(gpio_num_t pin, int level, uint32_t timeout_us) {
    int64_t start_time = esp_timer_get_time();
    while (gpio_get_level(pin) == level) {
        if ((esp_timer_get_time() - start_time) > timeout_us) {
            return -1;
        }
    }
    return 0;
}

esp_err_t dht_read_data(uint8_t dht_type, gpio_num_t pin, int16_t *humidity, int16_t *temperature) {
    int data[5] = {0};

    // Start signal
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
    vTaskDelay(pdMS_TO_TICKS(20)); // >18ms
    gpio_set_level(pin, 1);
    ets_delay_us(30);
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    if (dht_wait(pin, 1, 80) < 0) return ESP_FAIL;
    if (dht_wait(pin, 0, 80) < 0) return ESP_FAIL;

    for (int i = 0; i < 40; i++) {
        if (dht_wait(pin, 1, 50) < 0) return ESP_FAIL;
        int64_t start = esp_timer_get_time();
        if (dht_wait(pin, 0, 70) < 0) return ESP_FAIL;
        int64_t duration = esp_timer_get_time() - start;

        data[i / 8] <<= 1;
        if (duration > 40) data[i / 8] |= 1;
    }

    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        ESP_LOGE(TAG, "Checksum failed");
        return ESP_FAIL;
    }

    if (dht_type == DHT_TYPE_DHT11) {
        *humidity = data[0] * 10;
        *temperature = data[2] * 10;
    } else {
        return ESP_ERR_INVALID_ARG;
    }

    return ESP_OK;
}
