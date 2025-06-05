#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "driver/gpio.h"
#include "dht.h"
// #include "esp_http_client.h"  // Required!
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"
// #include "nvs_flash.h"
// #include "esp_wifi.h"
// #include "esp_event.h"
// #include "esp_netif.h"
// #include "esp_system.h"

#define TAG "MAIN"

#define AP_SSID "ESP32_AP"
#define AP_PASS "12345678"
#define MAX_STA_CONN 4

int latest_temp = 0;
int latest_hum = 0;

static esp_err_t root_get_handler(httpd_req_t *req)
{
    char html[256];
    snprintf(html, sizeof(html),
        "<!DOCTYPE html><html><head><title>DHT11 Data</title></head><body>"
        "<h2>DHT11 Sensor Data</h2>"
        "<p>Temperature: %d °C</p>"
        "<p>Humidity: %d %%</p>"
        "</body></html>", latest_temp, latest_hum);

    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &root_uri);
    }
    return server;
}

void dht11_task(void *pv)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    while (1) {
        int16_t temperature = 0, humidity = 0;
        if (dht_read_data(DHT_TYPE_DHT11, GPIO_NUM_4, &humidity, &temperature) == ESP_OK) {
            latest_temperature = temperature / 10;
            latest_humidity = humidity / 10;
            ESP_LOGI(TAG, "Temperature: %d °C | Humidity: %d %%", latest_temperature, latest_humidity);
        } else {
            ESP_LOGW(TAG, "Failed to read DHT11");
        }
        send_data_to_django(humidity / 10, temperature / 10);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void wifi_init_softap(void)
{
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .password = AP_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(AP_PASS) == 0) wifi_config.ap.authmode = WIFI_AUTH_OPEN;

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "Wi-Fi AP initialized: %s", AP_SSID);
}

// void send_data_to_django(int humidity, int temp) {
//     char post_data[64];
//     snprintf(post_data, sizeof(post_data), "humidity=%d&temp=%d", humidity, temp);

//     esp_http_client_config_t config = {
//         .url = "http://192.168.1.5:8000/api/post/", // Replace with your Django server IP
//     };
//     esp_http_client_handle_t client = esp_http_client_init(&config);
//     esp_http_client_set_method(client, HTTP_METHOD_POST);
//     esp_http_client_set_post_field(client, post_data, strlen(post_data));
//     esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");

//     esp_http_client_perform(client);
//     esp_http_client_cleanup(client);
// }


void app_main(void)
{
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();

    wifi_init_softap();
    start_webserver();

    xTaskCreate(dht11_task, "dht11_task", 4096, NULL, 5, NULL);
}

// #include "driver/gpio.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// void app_main() {
//     gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
//     while (1) {
//         gpio_set_level(GPIO_NUM_2, 1);
//         vTaskDelay(pdMS_TO_TICKS(500));
//         gpio_set_level(GPIO_NUM_2, 0);
//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }
