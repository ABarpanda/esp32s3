idf_component_register(
    SRCS "main.c" "dht.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_wifi esp_event esp_netif esp_http_server nvs_flash
)
