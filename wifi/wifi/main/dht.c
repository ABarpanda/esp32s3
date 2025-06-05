#include "dht.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define TAG "dht"

static gpio_num_t dht_gpio;
static dht_type_t dht_type;

#define DHT_TIMEOUT_US 1000

// Utility: Wait for level with timeout
static int wait_for_level(int level, uint32_t timeout_us) {
    uint32_t start = esp_timer_get_time();
    while (gpio_get_level(dht_gpio) == level) {
        if (esp_timer_get_time() - start > timeout_us)
            return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}

esp_err_t dht_init(gpio_num_t pin, dht_type_t type) {
    dht_gpio = pin;
    dht_type = type;

    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(dht_gpio, GPIO_PULLUP_ONLY);
    return ESP_OK;
}

esp_err_t dht_read(float *temperature, float *humidity) {
    uint8_t data[5] = {0};

    // Send start signal
    gpio_set_level(dht_gpio, 0);
    gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    vTaskDelay(pdMS_TO_TICKS(dht_type == DHT_TYPE_DHT11 ? 20 : 2));
    gpio_set_level(dht_gpio, 1);
    ets_delay_us(40);
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT);

    // Wait for sensor response
    if (wait_for_level(0, DHT_TIMEOUT_US) != ESP_OK) return ESP_FAIL;
    if (wait_for_level(1, DHT_TIMEOUT_US) != ESP_OK) return ESP_FAIL;

    // Read 40 bits
    for (int i = 0; i < 40; i++) {
        if (wait_for_level(0, DHT_TIMEOUT_US) != ESP_OK) return ESP_FAIL;
        uint32_t start = esp_timer_get_time();
        if (wait_for_level(1, DHT_TIMEOUT_US) != ESP_OK) return ESP_FAIL;
        uint32_t duration = esp_timer_get_time() - start;

        data[i / 8] <<= 1;
        if (duration > 40) data[i / 8] |= 1;  // 1 bit
    }

    // Checksum
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        ESP_LOGE(TAG, "Checksum failed: %d != %d", checksum, data[4]);
        return ESP_FAIL;
    }

    // Convert
    if (dht_type == DHT_TYPE_DHT11) {
        *humidity = data[0];
        *temperature = data[2];
    } else {
        *humidity = ((data[0] << 8) | data[1]) * 0.1;
        *temperature = (((data[2] & 0x7F) << 8) | data[3]) * 0.1;
        if (data[2] & 0x80) *temperature = -*temperature;
    }

    return ESP_OK;
}
