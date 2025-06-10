#include "dht.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "dht";
static gpio_num_t dht_gpio;

static inline int wait_level(int level, uint32_t timeout_us) {
    for (uint32_t cnt = 0; cnt < timeout_us; ++cnt) {
        if (gpio_get_level(dht_gpio) == level) return cnt;
        esp_rom_delay_us(1);
    }
    return -1;
}

esp_err_t dht_init(gpio_num_t pin, dht_type_t type) {
    if (type != DHT_TYPE_DHT11) return ESP_ERR_INVALID_ARG;

    dht_gpio = pin;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << dht_gpio),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    ESP_LOGI("dht", "Initializing DHT on GPIO %d", pin);


    return ESP_OK;
}

esp_err_t dht_read(float *temperature, float *humidity) {
    int raw[5] = {0};

    gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_gpio, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(dht_gpio, 1);
    esp_rom_delay_us(40);
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT);

    if (wait_level(0, 90) < 0) {
        ESP_LOGE(TAG, "Timeout waiting for sensor response LOW");
        return ESP_FAIL;
    }
    if (wait_level(1, 90) < 0) {
        ESP_LOGE(TAG, "Timeout waiting for sensor response HIGH");
        return ESP_FAIL;
    }

    for (int i = 0; i < 40; i++) {
        if (wait_level(0, 60) < 0) {
            ESP_LOGE(TAG, "Timeout waiting for LOW bit %d", i);
            return ESP_FAIL;
        }
        int high_us = wait_level(1, 100);  // increased from 75 to 100
        if (high_us < 0) {
            ESP_LOGE(TAG, "Timeout waiting for HIGH bit %d", i);
            return ESP_FAIL;
        }

        raw[i / 8] <<= 1;
        if (high_us > 40) raw[i / 8] |= 1;
    }

    ESP_LOGI(TAG, "Raw data: %d %d %d %d %d", raw[0], raw[1], raw[2], raw[3], raw[4]);

    if (((raw[0] + raw[1] + raw[2] + raw[3]) & 0xFF) != raw[4]) {
        ESP_LOGE(TAG, "Checksum failed");
        return ESP_ERR_INVALID_CRC;
    }

    *humidity = raw[0];
    *temperature = raw[2];
    ESP_LOGI(TAG, "DHT11: %.0f%% %.0f°C", *humidity, *temperature);

    return ESP_OK;
}