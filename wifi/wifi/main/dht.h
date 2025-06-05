#ifndef DHT_H
#define DHT_H

#include "driver/gpio.h"    // Include this before using gpio_num_t
#include "esp_err.h"        // For esp_err_t

typedef enum {
    DHT_TYPE_DHT11 = 0,
    DHT_TYPE_DHT22
} dht_type_t;

esp_err_t dht_init(gpio_num_t pin, dht_type_t type);
esp_err_t dht_read(float *temperature, float *humidity);

#endif // DHT_H
