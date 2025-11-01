# NoiseSensor Library

Librería para medición de ruido ambiental en ESP32-C3, ESP32-S2 y ESP32-S3.

## Características


- **Cálculos precisos**: Implementa todos los cálculos de ruido del sistema original
- **ADC en GPIO 4**: Configurado para leer desde GPIO 4
- **Mediciones múltiples**: 
  - Promedio de ruido (LAeq)
  - Valores pico y mínimo
  - Promedio legal (PMI)
  - Máximo promedio legal

## Uso Básico

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
        
        Serial.printf("Average: %.2f mV\n", m.noiseAvg);
        Serial.printf("Peak: %u mV\n", m.noisePeak);
        Serial.printf("Legal Max: %.2f mV\n", m.noiseAvgLegalMax);
        
        sensor.resetCycle();
    }
}
```

## Configuración Personalizada

```cpp
NoiseSensor::Config config;
config.adcPin = 4;               // GPIO para ADC (por defecto: 4)
config.dutyCycle = 120000;       // Periodo del ciclo en ms (por defecto: 120000 = 2min)
config.legalPeriod = 5000;       // Periodo de medición legal en ms (por defecto: 5000)
config.lowNoiseLevel = 36;       // Nivel base de ruido (por defecto: 36)
config.outlierThreshold = 4095;  // Umbral para descartar valores anómalos (por defecto: 4500)
config.indoor = false;           // Si true, no entra en modo sleep (por defecto: false)

NoiseSensor sensor(config);
sensor.begin();
```

## Mediciones Disponibles

La estructura `Measurements` contiene:

- `noise`: Valor actual de ruido en mV
- `noiseAvg`: Promedio de ruido en el ciclo actual
- `noiseAvgPre`: Promedio del ciclo anterior
- `noisePeak`: Valor pico detectado
- `noiseMin`: Valor mínimo detectado
- `lowNoiseLevel`: Nivel base dinámico ajustado automáticamente
- `noiseAvgLegal`: Promedio legal del período actual
- `noiseAvgLegalMax`: Máximo promedio legal registrado
- `cycles`: Contador de ciclos

## Funcionamiento

1. **Inicialización**: Llama a `sensor.begin()` en `setup()`
2. **Actualización**: Llama a `sensor.update()` en `loop()` repetidamente
3. **Detección de ciclo**: Verifica `sensor.isCycleComplete()` para saber cuándo procesar datos
4. **Lecturas**: Accede a las mediciones con `sensor.getMeasurements()` - **IMPORTANTE**: Lee los datos ANTES de llamar a `resetCycle()`
5. **Reset**: Llama a `sensor.resetCycle()` después de procesar los datos para preparar el próximo ciclo

**⚠️ Nota**: Los valores (peak, min, etc.) se resetean automáticamente cuando llamas a `resetCycle()`, así que asegúrate de leerlos ANTES de hacer el reset.

## Algoritmos Implementados

- **Detección de outliers**: Valores superiores a 4095 mV se descartan
- **Cálculo LAeq**: Promedio basado en muestras tomadas cada segundo
- **Medición legal**: Promedio de 5 segundos para cumplimiento normativo (independiente del ciclo principal)
- **Ajuste dinámico**: El nivel de ruido bajo se ajusta automáticamente
- **Control de ciclos**: Oscilación 1-99 para sincronización

## Detalles Técnicos

### Ciclos Independientes

La librería gestiona dos ciclos independientes:
- **Ciclo principal**: Cada `dutyCycle` (por defecto 120 segundos)
  - Calcula promedio general, pico y mínimo
  - Resetea valores cuando llamas a `resetCycle()`
  
- **Ciclo legal**: Cada `legalPeriod` (por defecto 5 segundos)
  - Calcula promedio legal y máximo promedio legal
  - Funciona de forma independiente y continua
  - NO se resetea con `resetCycle()`, solo con cada período legal

Esto permite tener mediciones legales más frecuentes que el reporte principal.

## Soporte de Hardware

La librería está configurada para compilar en múltiples placas ESP32:

### PlatformIO

El proyecto incluye configuraciones predefinidas en `platformio.ini`:
- **ESP32-C3 Mini** (`lolin_c3_mini`)
- **ESP32-S2** (`esp32-s2`)
- **ESP32-S3** (`esp32-s3`)

Para compilar para una placa específica:
```bash
pio run -e lolin_c3_mini    # Para ESP32-C3 Mini
pio run -e esp32-s2         # Para ESP32-S2
pio run -e esp32-s3         # Para ESP32-S3
```

### ADC Configuration

La librería usa `analogReadMilliVolts()` de Arduino ESP32, que devuelve directamente los valores en milivoltios sin necesidad de configuración adicional. Compatible con atenuación estándar de ADC.

