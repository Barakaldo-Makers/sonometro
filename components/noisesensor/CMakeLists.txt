# ESP-IDF component for NoiseSensor
idf_component_register(
    SRCS "noisesensor.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_timer log
)
