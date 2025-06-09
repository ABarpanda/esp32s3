// #pragma once
// #include "esp_err.h"
// #include "driver/gpio.h"

// typedef enum { DHT_TYPE_DHT11 = 0 } dht_type_t;

// esp_err_t dht_init(gpio_num_t pin, dht_type_t type);
// /* Reads the sensor once and fills °C / %RH (integer for DHT11) */
// esp_err_t dht_read(float *temperature, float *humidity);


#ifndef DHT_H
#define DHT_H

#include "esp_err.h"
#include "driver/gpio.h"

typedef enum {
    DHT_TYPE_DHT11 = 0
} dht_type_t;

esp_err_t dht_init(gpio_num_t pin, dht_type_t type);
esp_err_t dht_read(float *temperature, float *humidity);

#endif // DHT_H