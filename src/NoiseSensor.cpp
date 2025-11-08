#include "NoiseSensor.h"

NoiseSensor::NoiseSensor()
    : cycleComplete(false) {
    measurements.cycles = 50;
}

NoiseSensor::NoiseSensor(const Config& config) 
    : config(config), cycleComplete(false) {
    measurements.cycles = 50;
}

void NoiseSensor::begin() {
    // Configurar pin ADC (no necesario en ESP32, pero dejamos el comentario)
    // pinMode no se usa para ADC en ESP32
    
    // Inicializar variables
    tmpIni = millis();
    countStart = millis();
    legalStart = millis();
    
    measurements.noiseAvg = config.lowNoiseLevel;
    measurements.noiseAvgLegal = config.lowNoiseLevel;
    measurements.noiseAvgPre = config.lowNoiseLevel;
    measurements.noiseAvgLegalMax = config.lowNoiseLevel;
    measurements.noisePeak = 0;
    measurements.noiseMin = 1000;
    measurements.lowNoiseLevel = config.lowNoiseLevel;
    measurements.noise = 0;
    
    noiseSum = 0;
    noiseSumLegal = 0;
    loops = 0;
    loopsLegal = 0;
    icycles = 1;
    
    Serial.println("NoiseSensor initialized");
}

void NoiseSensor::update() {
    // Leer ruido cada milisegundo
    measurements.noise = readADC_mV();

    // ++++++++++++++  Eliminar outlier ++++++++++++++++
    if (measurements.noise > config.outlierThreshold) {
        Serial.print("Outlier removed: ");
        Serial.println(measurements.noise);
    } else {
        // Recalcular el LowNoiseLevel
        if (measurements.noise < measurements.lowNoiseLevel) {
            measurements.lowNoiseLevel = measurements.noise;
        }
        
        // ++++++++++++++  LAeq basado en muestras cada segundo ++++++++++++++++
        if (millis() - tmpIni > 1000) {
            noiseSum += measurements.noise;
            loops++;

            Serial.print("Noise: ");
            Serial.print(measurements.noise);
            Serial.print(" loop: ");
            Serial.print(loops);
            Serial.print(" cycle: ");
            Serial.print(measurements.cycles);
            Serial.print(" loops_legal: ");
            Serial.println(loopsLegal);
            tmpIni = millis();
        }

        // ++++++++++++++  Cálculos de ruido legal ++++++++++++++++
        loopsLegal++;
        noiseSumLegal += measurements.noise;
        if (millis() - legalStart > config.legalPeriod) {
            calculateLegalAverage();
            legalStart = millis();
            loopsLegal = 0;
            noiseSumLegal = 0;
        }

        // ++++++++++++++  Detección de máximo y mínimo ++++++++++++++++
        if (measurements.noise > measurements.noisePeak) {
            measurements.noisePeak = measurements.noise;
            Serial.print("Noise peak: ");
            Serial.println(measurements.noisePeak);
        }
        if (measurements.noise < measurements.noiseMin && loops > 5) {
            measurements.noiseMin = measurements.noise;
            Serial.print("Noise min: ");
            Serial.println(measurements.noiseMin);
        }
    }

    // Procesar ciclo principal
    if (millis() - countStart > config.dutyCycle) {
        processMainCycle();
    }
}

unsigned int NoiseSensor::readADC_mV() {
    // Leer ADC en milivoltios
    // Compatible con ESP32-C3, ESP32-S2, ESP32-S3
    
#if defined(ESP32) || defined(ESP32C3) || defined(ESP32S2) || defined(ESP32S3)
    // Usar analogReadMilliVolts() que devuelve directamente en mV
    return analogReadMilliVolts(config.adcPin);
#else
    // Fallback para otros microcontroladores
    int rawValue = analogRead(config.adcPin);
    // Asumir 10-bit ADC y referencia de 3.3V
    return map(rawValue, 0, 1023, 0, 3300);
#endif
}

void NoiseSensor::calculateLegalAverage() {
    Serial.print(" Legal time: ");
    Serial.println(millis() - legalStart);

    measurements.noiseAvgLegal = int(noiseSumLegal / loopsLegal);
    if (measurements.noiseAvgLegal > measurements.noiseAvgLegalMax) {
        measurements.noiseAvgLegalMax = measurements.noiseAvgLegal;
        Serial.print("  Noise legal current maximum: ");
        Serial.println(measurements.noiseAvgLegalMax);
    }

    Serial.print("   (Legal) noise_avg_legal: ");
    Serial.print(measurements.noiseAvgLegal);
    Serial.print(" noise_avg_legal_max: ");
    Serial.print(measurements.noiseAvgLegalMax);
    Serial.print(" samples: ");
    Serial.println(loopsLegal);
}

void NoiseSensor::processMainCycle() {
    Serial.print(" DutyCycle time: ");
    Serial.println(millis() - countStart);
    countStart = millis();

    // Cálculos de ruido
    measurements.noiseAvg = int(noiseSum / loops);
    Serial.print("  Noise average: ");
    Serial.println(measurements.noiseAvg);
    Serial.print("  Noise peak: ");
    Serial.println(measurements.noisePeak);
    Serial.print("  Noise sum: ");
    Serial.println(noiseSum);
    Serial.print("  Noise min: ");
    Serial.println(measurements.noiseMin);
    Serial.print("  Samples: ");
    Serial.println(loops);

    // Control de ciclos
    if (measurements.cycles > 99) {
        icycles = -1;
    } else if (measurements.cycles < 1) {
        // Reiniciar el LowNoiseLevel para evitar niveles bajos como 0
        measurements.lowNoiseLevel = measurements.noiseMin;
        icycles = +1;
    }
    measurements.cycles += icycles;

    // Modo de bajo ruido
    if (measurements.noiseAvg < measurements.lowNoiseLevel + config.noiseDiffSleep &&
        measurements.noiseAvgPre < measurements.lowNoiseLevel + config.noiseDiffSleep &&
        !config.indoor) {
        // Aquí podrías implementar sleep si lo necesitas
        measurements.cycles -= icycles;
        Serial.println("  Low noise mode detected");
    }
    
    measurements.noiseAvgPre = measurements.noiseAvg;
    
    // Marcar ciclo completado (NO resetear todavía!)
    cycleComplete = true;
}

void NoiseSensor::resetCycle() {
    // Reset para próximo ciclo
    measurements.noisePeak = 0;
    measurements.noiseMin = 1000;
    noiseSum = 0;
    loops = 0;
    measurements.noiseAvgLegalMax = 0;
    
    cycleComplete = false;
}

