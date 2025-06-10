#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "dht.h"

#define TAG "softAP_DHT"

static esp_err_t dht_handler(httpd_req_t *req) {
    float t = 0, h = 0;
    char json[64];
    esp_err_t ret = ESP_FAIL;

    for (int i = 0; i < 3; i++) {
        ret = dht_read(&t, &h);
        if (ret == ESP_OK) break;
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    if (ret == ESP_OK) {
        snprintf(json, sizeof(json), "{\"temperature\":%.0f,\"humidity\":%.0f}", t, h);
        httpd_resp_set_type(req, "application/json");
        return httpd_resp_sendstr(req, json);
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read sensor");
        return ESP_FAIL;
    }
}

static void start_webserver(void) {
    httpd_handle_t srv;
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.uri_match_fn = httpd_uri_match_wildcard;

    httpd_start(&srv, &cfg);

    httpd_uri_t uri = {
        .uri      = "/dht",
        .method   = HTTP_GET,
        .handler  = dht_handler,
    };
    httpd_register_uri_handler(srv, &uri);

    ESP_LOGI(TAG, "HTTP server ready – GET http://192.168.4.1/dht");
}

static void wifi_event_handler(void *arg, esp_event_base_t base,
                               int32_t id, void *data) {
    if (id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *e = data;
        ESP_LOGI(TAG, "STA "MACSTR" joined, AID=%d", MAC2STR(e->mac), e->aid);
    } else if (id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *e = data;
        ESP_LOGI(TAG, "STA "MACSTR" left; reason=%d", MAC2STR(e->mac), e->reason);
    }
}

static void wifi_init_softap(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                       ESP_EVENT_ANY_ID, wifi_event_handler, NULL, NULL));

    wifi_config_t wc = {
        .ap = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .ssid_len = strlen(CONFIG_ESP_WIFI_SSID),
            .channel = CONFIG_ESP_WIFI_CHANNEL,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .max_connection = CONFIG_ESP_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = { .required = true },
        },
    };
    if (strlen(CONFIG_ESP_WIFI_PASSWORD) == 0)
        wc.ap.authmode = WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wc));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "SoftAP up: SSID=%s PASS=%s CH=%d",
             CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD, CONFIG_ESP_WIFI_CHANNEL);
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_LOGI(TAG, "Booting…");

    dht_init(GPIO_NUM_4, DHT_TYPE_DHT11);     // <- sensor ready
    wifi_init_softap();
    start_webserver();
}
