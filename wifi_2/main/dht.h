// dht.h
#pragma once

#include "esp_err.h"

typedef enum {
    DHT_TYPE_DHT11 = 0
} dht_type_t;

esp_err_t dht_init(gpio_num_t pin, dht_type_t type);
esp_err_t dht_read(float *temperature, float *humidity);
