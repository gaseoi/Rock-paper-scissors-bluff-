idf_component_register(SRCS "wifi-local-discovery.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_wifi esp_event nvs_flash mdns lwip)
