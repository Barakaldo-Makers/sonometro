# Release v1.1.0

## Resumen
Esta version introduce soporte ESP-IDF con componente propio y ejemplos I2C
maestro/esclavo, ademas de ajustes de pines para ESP32-C3 SuperMini y limpieza
de documentacion.

## Cambios principales
- Nuevo componente ESP-IDF: `components/noisesensor`
- Ejemplos ESP-IDF: `idf_examples/i2c_slave_noise` y `idf_examples/i2c_master_noise`
- Pines SuperMini: SDA=GPIO8, SCL=GPIO10, ADC=GPIO4
- README simplificado y actualizado

## Notas de uso
- ESP-IDF slave: `idf_examples/i2c_slave_noise`
- ESP-IDF master: `idf_examples/i2c_master_noise`
- PlatformIO: `lib_deps = roberbike/NoiseSensor@^1.1.0`
