#ifndef NOISE_SENSOR_H
#define NOISE_SENSOR_H

#include <Arduino.h>

class NoiseSensor {
public:
    // Niveles de logging
    enum LogLevel {
        LOG_NONE = 0,      // Sin logs
        LOG_ERROR = 1,     // Solo errores
        LOG_WARN = 2,      // Warnings y errores
        LOG_INFO = 3,      // Información importante
        LOG_DEBUG = 4,     // Debug detallado
        LOG_VERBOSE = 5    // Todo (muy verboso)
    };

    // Configuración
    struct Config {
        int adcPin = 4;                          // GPIO para ADC
        unsigned long dutyCycle = 120000;        // Periodo de transmisión (ms)
        unsigned long legalPeriod = 5000;        // Periodo de medición legal (ms)
        int lowNoiseLevel = 36;                  // Nivel base de ruido bajo
        int noiseDiffSleep = 0;                  // Diferencias para modo sleep
        unsigned long sleep4NoNoise = 300000;    // Tiempo de sleep cuando hay poco ruido
        bool indoor = false;                     // Si true, nunca entra en sleep
        int outlierThreshold = 4095;             // Umbral para descartar valores anómalos
        LogLevel logLevel = LOG_NONE;            // Nivel de logging (por defecto: INFO)
        float refDb = 94.0f;                     // Nivel de referencia en dB para calibración
        float refMv = 1000.0f;                   // mV medidos en el nivel de referencia
        bool assumeAWeighted = true;             // El sensor entrega señal ya ponderada A
        uint8_t dayStartHour = 7;                // Inicio del periodo diurno (Ld)
        uint8_t eveningStartHour = 19;           // Inicio del periodo vespertino (Le)
        uint8_t nightStartHour = 23;             // Inicio del periodo nocturno (Ln)
        bool trackLdLeLn = true;                 // Calcular Ld/Le/Ln si hay hora válida
    };

    // Resultados de mediciones
    struct Measurements {
        unsigned int noise;                      // Valor actual de ruido
        float noiseAvg;                          // Promedio de ruido
        float noiseAvgPre;                       // Promedio previo
        unsigned int noisePeak;                  // Valor pico de ruido
        unsigned int noiseMin;                   // Valor mínimo de ruido
        int lowNoiseLevel;                       // Nivel base dinámico
        float noiseAvgLegal;                     // Promedio legal actual
        float noiseAvgLegalMax;                  // Máximo promedio legal
        unsigned int cycles;                     // Contador de ciclos
        float noiseDb;                           // Valor instantáneo en dB
        float noiseAvgDb;                        // LAeq del ciclo en dB
        float noisePeakDb;                       // Pico en dB
        float noiseMinDb;                        // Mínimo en dB
        float noiseAvgLegalDb;                   // Promedio legal en dB
        float noiseAvgLegalMaxDb;                // Máximo legal en dB
        float Ld;                                // Índice diurno
        float Le;                                // Índice vespertino
        float Ln;                                // Índice nocturno
    };

    // Constructor
    NoiseSensor();
    NoiseSensor(const Config& config);

    // Inicialización
    void begin();

    // Actualizar mediciones (llamar desde loop)
    void update();

    // Obtener mediciones actuales
    const Measurements& getMeasurements() const { return measurements; }

    // Verificar si es momento de enviar datos (ciclo completado)
    bool isCycleComplete() const { return cycleComplete; }

    // Preparar para nuevo ciclo (llamar después de procesar datos)
    void resetCycle();

    // Informar la hora actual (0-23) para cálculo Ld/Le/Ln
    void setCurrentHour(uint8_t hour);

    // Reiniciar acumuladores de Ld/Le/Ln (recomendado cada 24h)
    void resetLdLeLn();

private:
    Config config;
    Measurements measurements;

    // Control de tiempo
    unsigned long tmpIni;
    unsigned long countStart;
    unsigned long legalStart;

    // Contadores y sumas
    int loops;
    int loopsLegal;
    unsigned long noiseSum;
    unsigned long noiseSumLegal;

    // Control de ciclos
    float icycles;
    bool cycleComplete;

    // Estado temporal para Ld/Le/Ln
    int currentHour = -1;

    // Acumuladores en energía para LAeq
    double noiseSumEnergy;
    double noiseSumLegalEnergy;
    double dayEnergySum;
    double eveningEnergySum;
    double nightEnergySum;
    unsigned long dayCount;
    unsigned long eveningCount;
    unsigned long nightCount;

    // Leer ADC en milivoltios (compatible con ESP32-C3/S2/S3)
    unsigned int readADC_mV();

    // Calcular promedio legal
    void calculateLegalAverage();

    // Procesar ciclo principal
    void processMainCycle();

    // Conversión mV -> dB (calibrada)
    float mvToDb(float mv) const;

    // Métodos de logging optimizados
    void log(LogLevel level, const char* message) const;
    void log(LogLevel level, const char* prefix, unsigned long value) const;
    void log(LogLevel level, const char* prefix, int value) const;
    void log(LogLevel level, const char* prefix, float value) const;
    void log(LogLevel level, const char* prefix, unsigned int value) const;
    inline bool shouldLog(LogLevel level) const { return level <= config.logLevel; }
};

#endif // NOISE_SENSOR_H

