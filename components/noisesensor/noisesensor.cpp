#include "noisesensor.h"

#include <math.h>

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "NoiseSensor";

static uint64_t now_ms() { return esp_timer_get_time() / 1000ULL; }

static int bitwidth_to_max(adc_bitwidth_t width) {
  switch (width) {
    case ADC_BITWIDTH_9:
      return (1 << 9) - 1;
    case ADC_BITWIDTH_10:
      return (1 << 10) - 1;
    case ADC_BITWIDTH_11:
      return (1 << 11) - 1;
    case ADC_BITWIDTH_12:
      return (1 << 12) - 1;
    default:
      return (1 << 12) - 1;
  }
}

static bool init_adc_calibration(adc_unit_t unit, adc_atten_t atten, adc_bitwidth_t bitwidth,
                                 void **out_handle) {
  if (out_handle == nullptr) return false;
  *out_handle = nullptr;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = unit,
      .atten = atten,
      .bitwidth = bitwidth,
  };
  if (adc_cali_create_scheme_curve_fitting(&cali_config,
                                           reinterpret_cast<adc_cali_handle_t *>(out_handle)) ==
      ESP_OK) {
    return true;
  }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  adc_cali_line_fitting_config_t cali_config = {
      .unit_id = unit,
      .atten = atten,
      .bitwidth = bitwidth,
  };
  if (adc_cali_create_scheme_line_fitting(&cali_config,
                                          reinterpret_cast<adc_cali_handle_t *>(out_handle)) ==
      ESP_OK) {
    return true;
  }
#endif

  return false;
}

NoiseSensor::NoiseSensor() : cycleComplete(false) { measurements.cycles = 50; }

NoiseSensor::NoiseSensor(const Config &config) : config(config), cycleComplete(false) {
  measurements.cycles = 50;
}

esp_err_t NoiseSensor::begin() {
  esp_err_t err = adc_oneshot_io_to_channel(static_cast<gpio_num_t>(config.adc_gpio), &adcUnit,
                                            &adcChannel);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "ADC GPIO %d no es válido para ADC (%s)", config.adc_gpio,
             esp_err_to_name(err));
    return err;
  }

  adc_oneshot_unit_init_cfg_t unit_cfg = {
      .unit_id = adcUnit,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };
  err = adc_oneshot_new_unit(&unit_cfg, &adcHandle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "No se pudo inicializar ADC oneshot (%s)", esp_err_to_name(err));
    return err;
  }

  adc_oneshot_chan_cfg_t chan_cfg = {
      .atten = config.adcAtten,
      .bitwidth = config.adcWidth,
  };
  err = adc_oneshot_config_channel(adcHandle, adcChannel, &chan_cfg);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "No se pudo configurar ADC channel (%s)", esp_err_to_name(err));
    return err;
  }

  adcCalibrated = init_adc_calibration(adcUnit, config.adcAtten, config.adcWidth, &adcCaliHandle);
  if (adcCalibrated) {
    ESP_LOGI(TAG, "ADC calibrado correctamente");
  } else {
    ESP_LOGW(TAG, "ADC sin calibración, usando conversión aproximada");
  }

  tmpIniMs = now_ms();
  countStartMs = now_ms();
  legalStartMs = now_ms();

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

  log(LOG_INFO, "NoiseSensor (ESP-IDF) inicializado");
  return ESP_OK;
}

void NoiseSensor::update() {
  measurements.noise = readADC_mV();
  measurements.noiseDb = mvToDb((float)measurements.noise);

  if (measurements.noise > static_cast<uint32_t>(config.outlierThreshold)) {
    if (shouldLog(LOG_WARN)) {
      ESP_LOGW(TAG, "Outlier removed: %lu", static_cast<unsigned long>(measurements.noise));
    }
    return;
  }

  if (measurements.noise < static_cast<uint32_t>(measurements.lowNoiseLevel)) {
    measurements.lowNoiseLevel = measurements.noise;
  }

  uint64_t now = now_ms();

  if (now - tmpIniMs > 1000) {
    noiseSum += measurements.noise;
    noiseSumEnergy += pow(10.0f, measurements.noiseDb / 10.0f);
    loops++;

    if (shouldLog(LOG_VERBOSE)) {
      ESP_LOGD(TAG, "Noise:%lu dB:%.2f loop:%d cycle:%lu loops_legal:%d",
               static_cast<unsigned long>(measurements.noise), measurements.noiseDb, loops,
               static_cast<unsigned long>(measurements.cycles), loopsLegal);
    }
    tmpIniMs = now;
  }

  loopsLegal++;
  noiseSumLegal += measurements.noise;
  noiseSumLegalEnergy += pow(10.0f, measurements.noiseDb / 10.0f);
  if (now - legalStartMs > config.legalPeriod) {
    calculateLegalAverage();
    legalStartMs = now;
    loopsLegal = 0;
    noiseSumLegal = 0;
    noiseSumLegalEnergy = 0.0;
  }

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

  if (now - countStartMs > config.dutyCycle) {
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

uint32_t NoiseSensor::readADC_mV() {
  int raw = 0;
  if (adc_oneshot_read(adcHandle, adcChannel, &raw) != ESP_OK) {
    return 0;
  }

  if (adcCalibrated && adcCaliHandle != nullptr) {
    int voltage = 0;
    if (adc_cali_raw_to_voltage(reinterpret_cast<adc_cali_handle_t>(adcCaliHandle), raw,
                                &voltage) == ESP_OK) {
      return static_cast<uint32_t>(voltage);
    }
  }

  int max_val = bitwidth_to_max(config.adcWidth);
  return static_cast<uint32_t>((raw * 3300) / max_val);
}

void NoiseSensor::calculateLegalAverage() {
  log(LOG_DEBUG, " Legal time: ", static_cast<uint64_t>(now_ms() - legalStartMs));

  if (loopsLegal == 0) return;
  measurements.noiseAvgLegal = static_cast<float>(noiseSumLegal / loopsLegal);
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
    ESP_LOGD(TAG, "(Legal) noise_avg_legal: %.2f dB:%.2f noise_avg_legal_max: %.2f samples:%d",
             measurements.noiseAvgLegal, measurements.noiseAvgLegalDb,
             measurements.noiseAvgLegalMax, loopsLegal);
  }
}

void NoiseSensor::processMainCycle() {
  log(LOG_INFO, " DutyCycle time: ", static_cast<uint64_t>(now_ms() - countStartMs));
  countStartMs = now_ms();

  if (loops == 0) return;
  measurements.noiseAvg = static_cast<float>(noiseSum / loops);
  float avgEnergy = (float)(noiseSumEnergy / loops);
  measurements.noiseAvgDb = avgEnergy > 0.0f ? 10.0f * log10f(avgEnergy) : 0.0f;

  if (shouldLog(LOG_INFO)) {
    ESP_LOGI(TAG, "Noise average: %.2f", measurements.noiseAvg);
    ESP_LOGI(TAG, "Noise average dB: %.2f", measurements.noiseAvgDb);
    ESP_LOGI(TAG, "Noise peak: %lu", static_cast<unsigned long>(measurements.noisePeak));
    ESP_LOGI(TAG, "Noise min: %lu", static_cast<unsigned long>(measurements.noiseMin));
    ESP_LOGI(TAG, "Samples: %d", loops);
  }

  if (shouldLog(LOG_DEBUG)) {
    ESP_LOGD(TAG, "Noise sum: %llu", static_cast<unsigned long long>(noiseSum));
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

  if (measurements.cycles > 99) {
    icycles = -1;
  } else if (measurements.cycles < 1) {
    measurements.lowNoiseLevel = measurements.noiseMin;
    icycles = +1;
  }
  measurements.cycles += icycles;

  if (measurements.noiseAvg < measurements.lowNoiseLevel + config.noiseDiffSleep &&
      measurements.noiseAvgPre < measurements.lowNoiseLevel + config.noiseDiffSleep &&
      !config.indoor) {
    measurements.cycles -= icycles;
    log(LOG_INFO, "  Low noise mode detected");
  }

  measurements.noiseAvgPre = measurements.noiseAvg;
  cycleComplete = true;
}

void NoiseSensor::resetCycle() {
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

void NoiseSensor::log(LogLevel level, const char *message) const {
  if (!shouldLog(level)) return;
  switch (level) {
    case LOG_ERROR:
      ESP_LOGE(TAG, "%s", message);
      break;
    case LOG_WARN:
      ESP_LOGW(TAG, "%s", message);
      break;
    case LOG_INFO:
      ESP_LOGI(TAG, "%s", message);
      break;
    case LOG_DEBUG:
      ESP_LOGD(TAG, "%s", message);
      break;
    case LOG_VERBOSE:
      ESP_LOGV(TAG, "%s", message);
      break;
    default:
      break;
  }
}

void NoiseSensor::log(LogLevel level, const char *prefix, uint64_t value) const {
  if (!shouldLog(level)) return;
  switch (level) {
    case LOG_ERROR:
      ESP_LOGE(TAG, "%s%llu", prefix, static_cast<unsigned long long>(value));
      break;
    case LOG_WARN:
      ESP_LOGW(TAG, "%s%llu", prefix, static_cast<unsigned long long>(value));
      break;
    case LOG_INFO:
      ESP_LOGI(TAG, "%s%llu", prefix, static_cast<unsigned long long>(value));
      break;
    case LOG_DEBUG:
      ESP_LOGD(TAG, "%s%llu", prefix, static_cast<unsigned long long>(value));
      break;
    case LOG_VERBOSE:
      ESP_LOGV(TAG, "%s%llu", prefix, static_cast<unsigned long long>(value));
      break;
    default:
      break;
  }
}

void NoiseSensor::log(LogLevel level, const char *prefix, int value) const {
  if (!shouldLog(level)) return;
  switch (level) {
    case LOG_ERROR:
      ESP_LOGE(TAG, "%s%d", prefix, value);
      break;
    case LOG_WARN:
      ESP_LOGW(TAG, "%s%d", prefix, value);
      break;
    case LOG_INFO:
      ESP_LOGI(TAG, "%s%d", prefix, value);
      break;
    case LOG_DEBUG:
      ESP_LOGD(TAG, "%s%d", prefix, value);
      break;
    case LOG_VERBOSE:
      ESP_LOGV(TAG, "%s%d", prefix, value);
      break;
    default:
      break;
  }
}

void NoiseSensor::log(LogLevel level, const char *prefix, float value) const {
  if (!shouldLog(level)) return;
  switch (level) {
    case LOG_ERROR:
      ESP_LOGE(TAG, "%s%.2f", prefix, value);
      break;
    case LOG_WARN:
      ESP_LOGW(TAG, "%s%.2f", prefix, value);
      break;
    case LOG_INFO:
      ESP_LOGI(TAG, "%s%.2f", prefix, value);
      break;
    case LOG_DEBUG:
      ESP_LOGD(TAG, "%s%.2f", prefix, value);
      break;
    case LOG_VERBOSE:
      ESP_LOGV(TAG, "%s%.2f", prefix, value);
      break;
    default:
      break;
  }
}
