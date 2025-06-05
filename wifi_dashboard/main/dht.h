#ifndef DHT_H
#define DHT_H

#include "driver/gpio.h"
#include "esp_err.h"

#define DHT_TYPE_DHT11 11

esp_err_t dht_read_data(uint8_t dht_type, gpio_num_t pin, int16_t *humidity, int16_t *temperature);

#endif // DHT_H
