#include <Arduino.h>
#include "NoiseSensor.h"

// Configurar el nivel de logging según necesidad:
// LOG_NONE    - Sin logs (máximo rendimiento)
// LOG_ERROR   - Solo errores
// LOG_WARN    - Warnings y errores
// LOG_INFO    - Información importante (por defecto)
// LOG_DEBUG   - Debug detallado
// LOG_VERBOSE - Todo (muy verboso)

NoiseSensor::Config config;
// config.logLevel = NoiseSensor::LOG_NONE;     // Producción
// config.logLevel = NoiseSensor::LOG_INFO;     // Por defecto
// config.logLevel = NoiseSensor::LOG_DEBUG;     // Debug
config.logLevel = NoiseSensor::LOG_VERBOSE;     // Desarrollo

NoiseSensor sensor(config);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== NoiseSensor Basic Usage Example ===");
    Serial.print("Log level: ");
    switch(config.logLevel) {
        case NoiseSensor::LOG_NONE:    Serial.println("NONE"); break;
        case NoiseSensor::LOG_ERROR:   Serial.println("ERROR"); break;
        case NoiseSensor::LOG_WARN:    Serial.println("WARN"); break;
        case NoiseSensor::LOG_INFO:    Serial.println("INFO"); break;
        case NoiseSensor::LOG_DEBUG:   Serial.println("DEBUG"); break;
        case NoiseSensor::LOG_VERBOSE: Serial.println("VERBOSE"); break;
    }
    Serial.println();
    
    sensor.begin();
}

void loop() {
    sensor.update();

    if (sensor.isCycleComplete()) {
        const auto &measurements = sensor.getMeasurements();
        
        Serial.println("=== Cycle Complete ===");
        Serial.printf("Average noise: %.2f mV\n", measurements.noiseAvg);
        Serial.printf("Peak noise: %u mV\n", measurements.noisePeak);
        Serial.printf("Min noise: %u mV\n", measurements.noiseMin);
        Serial.printf("Legal Max: %.2f mV\n", measurements.noiseAvgLegalMax);
        Serial.printf("Cycles: %u\n", measurements.cycles);
        Serial.println();
        
        sensor.resetCycle();
    }

    delay(10);
}
