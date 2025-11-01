/**
 * @file BasicUsage.cpp
 * @brief Ejemplo básico de uso de la librería NoiseSensor
 * 
 * Este ejemplo muestra cómo usar la librería NoiseSensor para realizar
 * mediciones de ruido ambiental en ESP32-C3, ESP32-S2 o ESP32-S3.
 * 
 * Hardware requerido:
 * - ESP32-C3, ESP32-S2 o ESP32-S3
 * - Sensor de sonido o micrófono conectado a GPIO 4
 * 
 * @author Carlos Orts
 */

#include <Arduino.h>
#include "NoiseSensor.h"

// Instancia del sensor con configuración por defecto
NoiseSensor sensor;

void setup() {
    // Inicializar comunicación serial
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("===========================================");
    Serial.println("   NoiseSensor Library - Basic Example");
    Serial.println("===========================================");
    
    // Inicializar el sensor
    sensor.begin();
    
    Serial.println("\nMonitoring started...");
    Serial.println("Cycle duration: 120 seconds");
    Serial.println("Legal period: 5 seconds");
    Serial.println("===========================================\n");
}

void loop() {
    // Actualizar mediciones (llamar repetidamente)
    sensor.update();
    
    // Verificar si el ciclo principal está completo
    if (sensor.isCycleComplete()) {
        // Obtener todas las mediciones
        const auto& m = sensor.getMeasurements();
        
        Serial.println("\n===========================================");
        Serial.println("         CYCLE COMPLETE");
        Serial.println("===========================================");
        Serial.printf("Average noise:        %.2f mV\n", m.noiseAvg);
        Serial.printf("Peak noise:           %u mV\n", m.noisePeak);
        Serial.printf("Min noise:            %u mV\n", m.noiseMin);
        Serial.printf("Legal max average:    %.2f mV\n", m.noiseAvgLegalMax);
        Serial.printf("Low noise level:      %d mV\n", m.lowNoiseLevel);
        Serial.printf("Cycle number:         %u\n", m.cycles);
        Serial.println("===========================================\n");
        
        // IMPORTANTE: Resetear el ciclo después de leer los datos
        sensor.resetCycle();
    }
    
    // Pequeña pausa para evitar saturar el loop
    delay(10);
}

