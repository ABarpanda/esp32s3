#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"

#include "driver/gpio.h"
#include "dht.h"

#define DHT_GPIO GPIO_NUM_4

static const char *TAG = "SOFTAP_DHT";

void start_softap() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    wifi_config_t ap_config = {
        .ap = {
            .ssid = "ESP32-SoftAP",
            .ssid_len = 0,
            .password = "12345678",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        }
    };
    if (strlen((char *)ap_config.ap.password) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "SoftAP started. SSID: ESP32-SoftAP, Password: 12345678");
}

esp_err_t dht_get_handler(httpd_req_t *req) {
    float temperature = 0, humidity = 0;
    if (dht_read(&temperature, &humidity) == ESP_OK) {
        char resp[100];
        snprintf(resp, sizeof(resp),
                 "Temperature: %.1f°C\nHumidity: %.1f%%", temperature, humidity);
        httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    } else {
        httpd_resp_send_500(req);
    }
    return ESP_OK;
}

void start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    httpd_start(&server, &config);

    httpd_uri_t dht_uri = {
        .uri      = "/dht",
        .method   = HTTP_GET,
        .handler  = dht_get_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &dht_uri);
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    dht_init(DHT_GPIO, DHT_TYPE_DHT11);
    start_softap();
    start_webserver();
}
