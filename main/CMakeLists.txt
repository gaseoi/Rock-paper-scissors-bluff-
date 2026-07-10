idf_component_register(SRCS "main.c" "bitmaps.c" "uibackend.c"
                    INCLUDE_DIRS "."
                    REQUIRES u8g2 u8g2-hal-esp-idf wifi-local-discovery esp_driver_i2c esp_adc)
