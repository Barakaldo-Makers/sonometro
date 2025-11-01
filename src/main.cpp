#include <Arduino.h>
#include "NoiseSensor.h"

NoiseSensor sensor;

void setup() {
    Serial.begin(115200);
    sensor.begin();
    
    Serial.println("Noise monitoring started");
}

void loop() {
    sensor.update();
    
    // Procesar cuando el ciclo esté completo
    if (sensor.isCycleComplete()) {
        const auto& m = sensor.getMeasurements();
        
        Serial.println("=== Cycle Complete ===");
        Serial.printf("Average: %.2f\n", m.noiseAvg);
        Serial.printf("Peak: %u\n", m.noisePeak);
        Serial.printf("Min: %u\n", m.noiseMin);
        Serial.printf("Legal Max: %.2f\n", m.noiseAvgLegalMax);
        Serial.printf("Cycles: %u\n", m.cycles);
        Serial.printf("Low Noise Level: %d\n", m.lowNoiseLevel);
        Serial.println("=====================");
        
        // Aquí puedes enviar datos por WiFi, guardar en SD, etc.
        
        sensor.resetCycle();
    }
    
    delay(10); // Pequeña pausa para evitar saturar el loop
}
