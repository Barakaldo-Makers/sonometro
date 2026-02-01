#include "NoiseSensor.h"
#include <math.h>

// Implementación de métodos de logging
void NoiseSensor::log(LogLevel level, const char* message) const {
    if (shouldLog(level)) {
        Serial.println(message);
    }
}

void NoiseSensor::log(LogLevel level, const char* prefix, unsigned long value) const {
    if (shouldLog(level)) {
        Serial.print(prefix);
        Serial.println(value);
    }
}

void NoiseSensor::log(LogLevel level, const char* prefix, int value) const {
    if (shouldLog(level)) {
        Serial.print(prefix);
        Serial.println(value);
    }
}

void NoiseSensor::log(LogLevel level, const char* prefix, float value) const {
    if (shouldLog(level)) {
        Serial.print(prefix);
        Serial.println(value);
    }
}

void NoiseSensor::log(LogLevel level, const char* prefix, unsigned int value) const {
    if (shouldLog(level)) {
        Serial.print(prefix);
        Serial.println(value);
    }
}

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
    measurements.noiseDb = 0.0f;
    measurements.noiseAvgDb = 0.0f;
    measurements.noisePeakDb = 0.0f;
    measurements.noiseMinDb = 200.0f;
    measurements.noiseAvgLegalDb = 0.0f;
    measurements.noiseAvgLegalMaxDb = 0.0f;
    measurements.Ld = 0.0f;
    measurements.Le = 0.0f;
    measurements.Ln = 0.0f;
    
    noiseSum = 0;
    noiseSumLegal = 0;
    loops = 0;
    loopsLegal = 0;
    icycles = 1;
    noiseSumEnergy = 0.0;
    noiseSumLegalEnergy = 0.0;
    dayEnergySum = 0.0;
    eveningEnergySum = 0.0;
    nightEnergySum = 0.0;
    dayCount = 0;
    eveningCount = 0;
    nightCount = 0;
    currentHour = -1;
    
    log(LOG_INFO, "NoiseSensor initialized");
}

void NoiseSensor::update() {
    // Leer ruido cada milisegundo
    measurements.noise = readADC_mV();
    measurements.noiseDb = mvToDb((float)measurements.noise);

    // ++++++++++++++  Eliminar outlier ++++++++++++++++
    if (measurements.noise > config.outlierThreshold) {
        if (shouldLog(LOG_WARN)) {
            Serial.print("Outlier removed: ");
            Serial.println(measurements.noise);
        }
        return;
    } else {
        // Recalcular el LowNoiseLevel
        if (measurements.noise < measurements.lowNoiseLevel) {
            measurements.lowNoiseLevel = measurements.noise;
        }
        
        // ++++++++++++++  LAeq basado en muestras cada segundo ++++++++++++++++
        if (millis() - tmpIni > 1000) {
            noiseSum += measurements.noise;
            noiseSumEnergy += pow(10.0f, measurements.noiseDb / 10.0f);
            loops++;

            if (shouldLog(LOG_VERBOSE)) {
                Serial.print("Noise: ");
                Serial.print(measurements.noise);
                Serial.print(" dB: ");
                Serial.print(measurements.noiseDb, 2);
                Serial.print(" loop: ");
                Serial.print(loops);
                Serial.print(" cycle: ");
                Serial.print(measurements.cycles);
                Serial.print(" loops_legal: ");
                Serial.println(loopsLegal);
            }
            tmpIni = millis();
        }

        // ++++++++++++++  Cálculos de ruido legal ++++++++++++++++
        loopsLegal++;
        noiseSumLegal += measurements.noise;
        noiseSumLegalEnergy += pow(10.0f, measurements.noiseDb / 10.0f);
        if (millis() - legalStart > config.legalPeriod) {
            calculateLegalAverage();
            legalStart = millis();
            loopsLegal = 0;
            noiseSumLegal = 0;
            noiseSumLegalEnergy = 0.0;
        }

        // ++++++++++++++  Acumuladores Ld/Le/Ln ++++++++++++++++
        if (config.trackLdLeLn && currentHour >= 0 && currentHour < 24) {
            double energy = pow(10.0f, measurements.noiseDb / 10.0f);
            if (currentHour >= config.nightStartHour || currentHour < config.dayStartHour) {
                nightEnergySum += energy;
                nightCount++;
            } else if (currentHour >= config.eveningStartHour) {
                eveningEnergySum += energy;
                eveningCount++;
            } else {
                dayEnergySum += energy;
                dayCount++;
            }
        }

        // ++++++++++++++  Detección de máximo y mínimo ++++++++++++++++
        if (measurements.noise > measurements.noisePeak) {
            measurements.noisePeak = measurements.noise;
            measurements.noisePeakDb = measurements.noiseDb;
            log(LOG_DEBUG, "Noise peak: ", measurements.noisePeak);
        }
        if (measurements.noise < measurements.noiseMin && loops > 5) {
            measurements.noiseMin = measurements.noise;
            measurements.noiseMinDb = measurements.noiseDb;
            log(LOG_DEBUG, "Noise min: ", measurements.noiseMin);
        }
    }

    // Procesar ciclo principal
    if (millis() - countStart > config.dutyCycle) {
        processMainCycle();
    }
}

float NoiseSensor::mvToDb(float mv) const {
    if (mv <= 0.0f || config.refMv <= 0.0f) return 0.0f;
    return config.refDb + 20.0f * log10f(mv / config.refMv);
}

void NoiseSensor::setCurrentHour(uint8_t hour) {
    if (hour < 24) {
        currentHour = hour;
    } else {
        currentHour = -1;
    }
}

void NoiseSensor::resetLdLeLn() {
    dayEnergySum = 0.0;
    eveningEnergySum = 0.0;
    nightEnergySum = 0.0;
    dayCount = 0;
    eveningCount = 0;
    nightCount = 0;
    measurements.Ld = 0.0f;
    measurements.Le = 0.0f;
    measurements.Ln = 0.0f;
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
    log(LOG_DEBUG, " Legal time: ", millis() - legalStart);

    if (loopsLegal <= 0) return;
    measurements.noiseAvgLegal = int(noiseSumLegal / loopsLegal);
    float legalEnergy = (float)(noiseSumLegalEnergy / loopsLegal);
    measurements.noiseAvgLegalDb = legalEnergy > 0.0f ? 10.0f * log10f(legalEnergy) : 0.0f;
    if (measurements.noiseAvgLegal > measurements.noiseAvgLegalMax) {
        measurements.noiseAvgLegalMax = measurements.noiseAvgLegal;
        log(LOG_INFO, "  Noise legal current maximum: ", measurements.noiseAvgLegalMax);
    }
    if (measurements.noiseAvgLegalDb > measurements.noiseAvgLegalMaxDb) {
        measurements.noiseAvgLegalMaxDb = measurements.noiseAvgLegalDb;
    }

    if (shouldLog(LOG_DEBUG)) {
        Serial.print("   (Legal) noise_avg_legal: ");
        Serial.print(measurements.noiseAvgLegal);
        Serial.print(" dB: ");
        Serial.print(measurements.noiseAvgLegalDb, 2);
        Serial.print(" noise_avg_legal_max: ");
        Serial.print(measurements.noiseAvgLegalMax);
        Serial.print(" samples: ");
        Serial.println(loopsLegal);
    }
}

void NoiseSensor::processMainCycle() {
    log(LOG_INFO, " DutyCycle time: ", millis() - countStart);
    countStart = millis();

    // Cálculos de ruido
    if (loops <= 0) return;
    measurements.noiseAvg = int(noiseSum / loops);
    float avgEnergy = (float)(noiseSumEnergy / loops);
    measurements.noiseAvgDb = avgEnergy > 0.0f ? 10.0f * log10f(avgEnergy) : 0.0f;
    
    if (shouldLog(LOG_INFO)) {
        Serial.print("  Noise average: ");
        Serial.println(measurements.noiseAvg);
        Serial.print("  Noise average dB: ");
        Serial.println(measurements.noiseAvgDb, 2);
        Serial.print("  Noise peak: ");
        Serial.println(measurements.noisePeak);
        Serial.print("  Noise min: ");
        Serial.println(measurements.noiseMin);
        Serial.print("  Samples: ");
        Serial.println(loops);
    }
    
    if (shouldLog(LOG_DEBUG)) {
        Serial.print("  Noise sum: ");
        Serial.println(noiseSum);
    }

    if (config.trackLdLeLn) {
        if (dayCount > 0) {
            float dayEnergy = (float)(dayEnergySum / dayCount);
            measurements.Ld = dayEnergy > 0.0f ? 10.0f * log10f(dayEnergy) : 0.0f;
        }
        if (eveningCount > 0) {
            float eveningEnergy = (float)(eveningEnergySum / eveningCount);
            measurements.Le = eveningEnergy > 0.0f ? 10.0f * log10f(eveningEnergy) : 0.0f;
        }
        if (nightCount > 0) {
            float nightEnergy = (float)(nightEnergySum / nightCount);
            measurements.Ln = nightEnergy > 0.0f ? 10.0f * log10f(nightEnergy) : 0.0f;
        }
    }

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
        log(LOG_INFO, "  Low noise mode detected");
    }
    
    measurements.noiseAvgPre = measurements.noiseAvg;
    
    // Marcar ciclo completado (NO resetear todavía!)
    cycleComplete = true;
}

void NoiseSensor::resetCycle() {
    // Reset para próximo ciclo
    measurements.noisePeak = 0;
    measurements.noiseMin = 1000;
    measurements.noisePeakDb = 0.0f;
    measurements.noiseMinDb = 200.0f;
    noiseSum = 0;
    noiseSumEnergy = 0.0;
    loops = 0;
    measurements.noiseAvgLegalMax = 0;
    measurements.noiseAvgLegalMaxDb = 0.0f;
    
    cycleComplete = false;
}

