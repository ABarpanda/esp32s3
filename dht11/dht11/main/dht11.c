#include "dht11.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "esp_rom/ets_sys.h"
#include "esp_rom/esp32/ets_sys.h"


#define DHT11_GPIO 10

esp_err_t dht11_read(int *temperature, int *humidity) {
    // Basic DHT11 protocol implementation for ESP32
    // Simplified for demo, not for production

    int data[5] = {0};

    gpio_set_direction(DHT11_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT11_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(20));  // at least 18ms
    gpio_set_level(DHT11_GPIO, 1);
    ets_delay_us(30);
    gpio_set_direction(DHT11_GPIO, GPIO_MODE_INPUT);

    while (gpio_get_level(DHT11_GPIO) == 1);
    while (gpio_get_level(DHT11_GPIO) == 0);
    while (gpio_get_level(DHT11_GPIO) == 1);

    for (int i = 0; i < 40; i++) {
        while (gpio_get_level(DHT11_GPIO) == 0);
        int len = 0;
        while (gpio_get_level(DHT11_GPIO) == 1) {
            ets_delay_us(1);
            len++;
            if (len > 100) break;
        }
        data[i / 8] <<= 1;
        if (len > 40) data[i / 8] |= 1;
    }

    if ((data[0] + data[1] + data[2] + data[3]) & 0xFF != data[4])
        return ESP_ERR_INVALID_CRC;

    *humidity = data[0];
    *temperature = data[2];

    return ESP_OK;
}
