/**
 * @file CustomConfig.cpp
 * @brief Ejemplo de uso con configuración personalizada
 * 
 * Este ejemplo muestra cómo configurar parámetros personalizados
 * de la librería NoiseSensor.
 * 
 * @author Barakaldo Makers
 */

#include <Arduino.h>
#include "NoiseSensor.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("NoiseSensor - Custom Configuration Example");
    
    // Configurar parámetros personalizados
    NoiseSensor::Config config;
    config.adcPin = 4;               // GPIO para ADC
    config.dutyCycle = 60000;        // Ciclo de 60 segundos (en lugar de 120)
    config.legalPeriod = 5000;       // Período legal de 5 segundos
    config.lowNoiseLevel = 36;       // Nivel base de ruido
    config.outlierThreshold = 4095;  // Umbral para outliers
    config.indoor = false;           // Permite modo sleep automático
    
    // Crear instancia con configuración personalizada
    NoiseSensor sensor(config);
    
    // Inicializar
    sensor.begin();
    
    Serial.println("Sensor initialized with custom configuration:");
    Serial.printf("  Duty cycle: %lu ms\n", config.dutyCycle);
    Serial.printf("  Legal period: %lu ms\n", config.legalPeriod);
    Serial.printf("  Outlier threshold: %d mV\n", config.outlierThreshold);
    Serial.println();
}

void loop() {
    // Tu código aquí
}

