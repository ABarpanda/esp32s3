idf_component_register(
    SRCS "main.c" "dht.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_http_server
)
