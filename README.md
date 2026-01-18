# NoiseSensor Library

[![PlatformIO Registry](https://badges.registry.platformio.org/packages/roberbike/library/NoiseSensor.svg)](https://registry.platformio.org/libraries/roberbike/NoiseSensor)

Librería para medición de ruido ambiental en ESP32-C3, ESP32-S2 y ESP32-S3.

**Versión actual: 1.1.0**

## Características

- **Cálculos precisos**: Implementa los cálculos legales de ruido
- **ADC en GPIO 4**: Configurable
- **Mediciones múltiples**: LAeq, pico, mínimo, promedio legal y máximo legal
- **Logging configurable**: 6 niveles
- **Ciclos independientes**: ciclo principal y legal
- **Ajuste dinámico**: nivel base automático
- **Sin dependencias externas**: Arduino/ESP32

## Instalación (PlatformIO)

```ini
lib_deps = 
    roberbike/NoiseSensor@^1.1.0
```

o

```bash
pio lib install "roberbike/NoiseSensor@^1.1.0"
```

## Compatibilidad

- ✅ **ESP32-C3**
- ✅ **ESP32-S2**
- ✅ **ESP32-S3**
- ❌ **ESP32 (clásico)**
- ❌ **ESP8266**

## Uso rápido (Arduino)

```cpp
#include "NoiseSensor.h"

NoiseSensor sensor;

void setup() {
    Serial.begin(115200);
    sensor.begin();
}

void loop() {
    sensor.update();
    if (sensor.isCycleComplete()) {
        const auto& m = sensor.getMeasurements();
        Serial.printf("Promedio: %.2f mV\n", m.noiseAvg);
        sensor.resetCycle();
    }
}
```

## Configuración

```cpp
NoiseSensor::Config config;
config.adcPin = 4;
config.dutyCycle = 120000;
config.legalPeriod = 5000;
config.lowNoiseLevel = 36;
config.outlierThreshold = 4095;
config.indoor = false;
config.logLevel = NoiseSensor::LOG_INFO;

NoiseSensor sensor(config);
sensor.begin();
```

### Niveles de logging

- `LOG_NONE`, `LOG_ERROR`, `LOG_WARN`, `LOG_INFO`, `LOG_DEBUG`, `LOG_VERBOSE`

## Mediciones disponibles

- `noise`, `noiseAvg`, `noiseAvgPre`
- `noisePeak`, `noiseMin`
- `lowNoiseLevel`
- `noiseAvgLegal`, `noiseAvgLegalMax`
- `cycles`

## ESP-IDF

Componente en `components/noisesensor` con ADC calibrado.

Ejemplos:
- `idf_examples/i2c_slave_noise` (ESP32-C3 SuperMini esclavo, SDA=GPIO8, SCL=GPIO10, ADC=GPIO4)
- `idf_examples/i2c_master_noise` (ESP32-C3 SuperMini maestro, SDA=GPIO8, SCL=GPIO10)

### Compilar ejemplo ESP-IDF (esclavo)

```bash
cd idf_examples/i2c_slave_noise
idf.py set-target esp32c3
idf.py build
idf.py flash monitor
```

### Compilar ejemplo ESP-IDF (maestro)

```bash
cd idf_examples/i2c_master_noise
idf.py set-target esp32c3
idf.py build
idf.py flash monitor
```

## Changelog

### 1.1.0
- Migración ESP-IDF (componente + ejemplos I2C)
- Limpieza de documentación y ajustes de pines

## Licencia

GPL-3.0-or-later

## Enlaces

- PlatformIO: https://registry.platformio.org/libraries/roberbike/NoiseSensor
- Repositorio: https://github.com/Barakaldo-Makers/sonometro

