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

## Ejemplo incluido

Se incluye un ejemplo básico en `examples/BasicUsage`.

Para compilarlo con PlatformIO:

```bash
cd examples/BasicUsage
pio run
```

El ejemplo inicializa la librería, toma lecturas periódicas y las muestra por el puerto serie.

## Uso Básico

### Ejemplo Simple (con logging por defecto)

```cpp
#include "NoiseSensor.h"

NoiseSensor sensor;

void setup() {
    Serial.begin(115200);
    sensor.begin();  // Usa LOG_INFO por defecto
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

### Ejemplo con Configuración de Logging

```cpp
#include "NoiseSensor.h"

// Configurar nivel de logging
NoiseSensor::Config config;
config.logLevel = NoiseSensor::LOG_INFO;  // Ver opciones abajo

NoiseSensor sensor(config);

void setup() {
    Serial.begin(115200);
    sensor.begin();
}

void loop() {
    sensor.update();
    
    if (sensor.isCycleComplete()) {
        const auto& m = sensor.getMeasurements();
        // Procesar mediciones...
        sensor.resetCycle();
    }
}
```

### Ejemplos de Niveles de Logging

**Producción (sin logs):**
```cpp
config.logLevel = NoiseSensor::LOG_NONE;  // Máximo rendimiento
```

**Solo errores importantes:**
```cpp
config.logLevel = NoiseSensor::LOG_WARN;  // Warnings y errores
```

**Información estándar (por defecto):**
```cpp
config.logLevel = NoiseSensor::LOG_INFO;  // Promedios, ciclos completados
```

**Debug detallado:**
```cpp
config.logLevel = NoiseSensor::LOG_DEBUG;  // Picos, mínimos, promedios legales
```

**Desarrollo (muy verboso):**
```cpp
config.logLevel = NoiseSensor::LOG_VERBOSE;  // Incluye logs cada segundo
```

### Ejemplo Completo con Todas las Opciones

```cpp
#include "NoiseSensor.h"

NoiseSensor::Config config;

// Configuración de hardware
config.adcPin = 4;                    // GPIO para ADC

// Configuración de tiempos
config.dutyCycle = 120000;             // 2 minutos entre ciclos
config.legalPeriod = 5000;             // 5 segundos para promedio legal

// Configuración de ruido
config.lowNoiseLevel = 36;             // Nivel base de ruido
config.outlierThreshold = 4095;        // Umbral para outliers

// Configuración de logging
config.logLevel = NoiseSensor::LOG_INFO;  // Nivel de logging

// Modo indoor (no sleep)
config.indoor = false;

NoiseSensor sensor(config);

void setup() {
    Serial.begin(115200);
    delay(1000);
    sensor.begin();
}

void loop() {
    sensor.update();
    
    if (sensor.isCycleComplete()) {
        const auto& m = sensor.getMeasurements();
        
        // Mostrar todas las mediciones disponibles
        Serial.println("=== Mediciones del Ciclo ===");
        Serial.printf("Promedio: %.2f mV\n", m.noiseAvg);
        Serial.printf("Pico: %u mV\n", m.noisePeak);
        Serial.printf("Mínimo: %u mV\n", m.noiseMin);
        Serial.printf("Promedio Legal: %.2f mV\n", m.noiseAvgLegal);
        Serial.printf("Máximo Legal: %.2f mV\n", m.noiseAvgLegalMax);
        Serial.printf("Nivel Base: %d mV\n", m.lowNoiseLevel);
        Serial.printf("Ciclos: %u\n", m.cycles);
        Serial.println();
        
        sensor.resetCycle();
    }
    
    delay(10);
}
```

## Configuración Personalizada

```cpp
NoiseSensor::Config config;
config.adcPin = 4;               // GPIO para ADC (por defecto: 4)
config.dutyCycle = 120000;       // Periodo del ciclo en ms (por defecto: 120000 = 2min)
config.legalPeriod = 5000;       // Periodo de medición legal en ms (por defecto: 5000)
config.lowNoiseLevel = 36;       // Nivel base de ruido (por defecto: 36)
config.outlierThreshold = 4095;  // Umbral para descartar valores anómalos (por defecto: 4095)
config.indoor = false;           // Si true, no entra en modo sleep (por defecto: false)
config.logLevel = NoiseSensor::LOG_INFO;  // Nivel de logging (por defecto: LOG_INFO)

NoiseSensor sensor(config);
sensor.begin();
```

### Niveles de Logging

La librería incluye un sistema de logging optimizado con niveles configurables:

- `LOG_NONE` (0): Sin logs - máximo rendimiento
- `LOG_ERROR` (1): Solo errores críticos
- `LOG_WARN` (2): Warnings y errores
- `LOG_INFO` (3): Información importante (por defecto) - promedios, ciclos completados
- `LOG_DEBUG` (4): Debug detallado - picos, mínimos, promedios legales
- `LOG_VERBOSE` (5): Todo - incluye logs cada segundo

**Ejemplo para producción (sin logs):**
```cpp
config.logLevel = NoiseSensor::LOG_NONE;
```

**Ejemplo para debugging:**
```cpp
config.logLevel = NoiseSensor::LOG_VERBOSE;
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

