idf_component_register(SRCS "main.c" "dht.c"
                       INCLUDE_DIRS "."
		       REQUIRES esp_http_server esp_netif esp_wifi nvs_flash esp_event)
