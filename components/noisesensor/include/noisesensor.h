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
  };

  NoiseSensor();
  explicit NoiseSensor(const Config &config);

  esp_err_t begin();
  void update();
  const Measurements &getMeasurements() const { return measurements; }
  bool isCycleComplete() const { return cycleComplete; }
  void resetCycle();

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

  adc_oneshot_unit_handle_t adcHandle = nullptr;
  adc_unit_t adcUnit = ADC_UNIT_1;
  adc_channel_t adcChannel = ADC_CHANNEL_0;
  bool adcCalibrated = false;
  void *adcCaliHandle = nullptr;

  uint32_t readADC_mV();
  void calculateLegalAverage();
  void processMainCycle();

  void log(LogLevel level, const char *message) const;
  void log(LogLevel level, const char *prefix, uint64_t value) const;
  void log(LogLevel level, const char *prefix, int value) const;
  void log(LogLevel level, const char *prefix, float value) const;

  bool shouldLog(LogLevel level) const { return level <= config.logLevel; }
};
