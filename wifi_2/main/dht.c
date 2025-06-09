#include "dht.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TAG "DHT"
#define DHT_MAX_TIMINGS 85

static gpio_num_t dht_gpio;

esp_err_t dht_init(gpio_num_t pin, dht_type_t type) {
    if (type != DHT_TYPE_DHT11) return ESP_ERR_INVALID_ARG;

    dht_gpio = pin;
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(dht_gpio, GPIO_PULLUP_ONLY);

    return ESP_OK;
}

static int wait_for_level(int level, uint32_t timeout_us) {
    uint32_t time = 0;
    while (gpio_get_level(dht_gpio) != level) {
        ets_delay_us(1);
        if (++time > timeout_us) return -1;
    }
    return time;
}

esp_err_t dht_read(float *temperature, float *humidity) {
    int data[5] = {0, 0, 0, 0, 0};

    // Send start signal
    gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_gpio, 0);
    vTaskDelay(pdMS_TO_TICKS(20)); // at least 18ms
    gpio_set_level(dht_gpio, 1);
    ets_delay_us(40);
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT);

    // Wait for DHT response
    if (wait_for_level(0, 80) < 0 || wait_for_level(1, 80) < 0)
        return ESP_FAIL;

    // Read 40 bits (5 bytes)
    for (int i = 0; i < 40; i++) {
        if (wait_for_level(0, 50) < 0) return ESP_FAIL;
        int width = wait_for_level(1, 70);
        if (width < 0) return ESP_FAIL;

        data[i / 8] <<= 1;
        if (width > 40) data[i / 8] |= 1; // 70us = 1, 26-28us = 0
    }

    // Check checksum
    if (((data[0] + data[1] + data[2] + data[3]) & 0xFF) != data[4]) {
        ESP_LOGE(TAG, "Checksum failed: %02x != %02x", 
                 data[4], (data[0] + data[1] + data[2] + data[3]) & 0xFF);
        return ESP_ERR_INVALID_CRC;
    }

    *humidity = data[0];
    *temperature = data[2];

    ESP_LOGI(TAG, "DHT Read: Temp=%.1f, Hum=%.1f", *temperature, *humidity);

    return ESP_OK;
}
