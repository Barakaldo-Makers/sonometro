#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"

class NoiseSensor {
 public:
  enum LogLevel {
    LOG_NONE = 0,
    LOG_ERROR = 1,
    LOG_WARN = 2,
    LOG_INFO = 3,
    LOG_DEBUG = 4,
    LOG_VERBOSE = 5
  };

  struct Config {
    int adc_gpio = 4;                   // GPIO para ADC
    uint32_t dutyCycle = 120000;        // Periodo de transmisión (ms)
    uint32_t legalPeriod = 5000;        // Periodo de medición legal (ms)
    int lowNoiseLevel = 36;             // Nivel base de ruido bajo
    int noiseDiffSleep = 0;             // Diferencias para modo sleep
    uint32_t sleep4NoNoise = 300000;    // Tiempo de sleep cuando hay poco ruido
    bool indoor = false;                // Si true, nunca entra en sleep
    int outlierThreshold = 4095;        // Umbral para descartar valores anómalos
    LogLevel logLevel = LOG_NONE;       // Nivel de logging
    float refDb = 94.0f;                // Nivel de referencia en dB para calibración
    float refMv = 1000.0f;              // mV medidos en el nivel de referencia
    bool assumeAWeighted = true;        // El sensor entrega señal ya ponderada A
    uint8_t dayStartHour = 7;           // Inicio del periodo diurno (Ld)
    uint8_t eveningStartHour = 19;      // Inicio del periodo vespertino (Le)
    uint8_t nightStartHour = 23;        // Inicio del periodo nocturno (Ln)
    bool trackLdLeLn = true;            // Calcular Ld/Le/Ln si hay hora válida
    adc_atten_t adcAtten = ADC_ATTEN_DB_11;
    adc_bitwidth_t adcWidth = ADC_BITWIDTH_12;
  };

  struct Measurements {
    uint32_t noise = 0;       // Valor actual de ruido
    float noiseAvg = 0.0f;    // Promedio de ruido
    float noiseAvgPre = 0.0f; // Promedio previo
    uint32_t noisePeak = 0;   // Valor pico de ruido
    uint32_t noiseMin = 1000; // Valor mínimo de ruido
    int lowNoiseLevel = 0;    // Nivel base dinámico
    float noiseAvgLegal = 0;  // Promedio legal actual
    float noiseAvgLegalMax = 0;
    uint32_t cycles = 50;     // Contador de ciclos
    float noiseDb = 0.0f;     // Valor instantáneo en dB
    float noiseAvgDb = 0.0f;  // LAeq del ciclo en dB
    float noisePeakDb = 0.0f; // Pico en dB
    float noiseMinDb = 200.0f;// Mínimo en dB
    float noiseAvgLegalDb = 0.0f;
    float noiseAvgLegalMaxDb = 0.0f;
    float Ld = 0.0f;
    float Le = 0.0f;
    float Ln = 0.0f;
  };

  NoiseSensor();
  explicit NoiseSensor(const Config &config);

  esp_err_t begin();
  void update();
  const Measurements &getMeasurements() const { return measurements; }
  bool isCycleComplete() const { return cycleComplete; }
  void resetCycle();
  void setCurrentHour(uint8_t hour);
  void resetLdLeLn();

 private:
  Config config;
  Measurements measurements;

  uint64_t tmpIniMs = 0;
  uint64_t countStartMs = 0;
  uint64_t legalStartMs = 0;

  int loops = 0;
  int loopsLegal = 0;
  uint64_t noiseSum = 0;
  uint64_t noiseSumLegal = 0;

  float icycles = 1;
  bool cycleComplete = false;

  int currentHour = -1;
  double noiseSumEnergy = 0.0;
  double noiseSumLegalEnergy = 0.0;
  double dayEnergySum = 0.0;
  double eveningEnergySum = 0.0;
  double nightEnergySum = 0.0;
  unsigned long dayCount = 0;
  unsigned long eveningCount = 0;
  unsigned long nightCount = 0;

  adc_oneshot_unit_handle_t adcHandle = nullptr;
  adc_unit_t adcUnit = ADC_UNIT_1;
  adc_channel_t adcChannel = ADC_CHANNEL_0;
  bool adcCalibrated = false;
  void *adcCaliHandle = nullptr;

  uint32_t readADC_mV();
  void calculateLegalAverage();
  void processMainCycle();
  float mvToDb(float mv) const;

  void log(LogLevel level, const char *message) const;
  void log(LogLevel level, const char *prefix, uint64_t value) const;
  void log(LogLevel level, const char *prefix, int value) const;
  void log(LogLevel level, const char *prefix, float value) const;

  bool shouldLog(LogLevel level) const { return level <= config.logLevel; }
};
