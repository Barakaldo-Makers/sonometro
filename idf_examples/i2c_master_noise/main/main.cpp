#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "NoiseI2CMaster";

static constexpr i2c_port_t I2C_PORT = I2C_NUM_0;
// ESP32-C3 SuperMini (maestro): SDA en GPIO8 y SCL en GPIO10.
static constexpr gpio_num_t I2C_SDA = GPIO_NUM_8;
static constexpr gpio_num_t I2C_SCL = GPIO_NUM_10;
static constexpr uint8_t I2C_SLAVE_ADDR = 0x08;

// I2C commands
enum I2CCommand : uint8_t {
  CMD_GET_DATA = 0x01,
  CMD_GET_AVG = 0x02,
  CMD_GET_PEAK = 0x03,
  CMD_GET_MIN = 0x04,
  CMD_GET_LEGAL = 0x05,
  CMD_GET_LEGAL_MAX = 0x06,
  CMD_GET_STATUS = 0x07,
  CMD_RESET = 0x08,
};

struct SensorData {
  uint32_t noise;
  float noiseAvg;
  float noisePeak;
  float noiseMin;
  float noiseAvgLegal;
  float noiseAvgLegalMax;
  uint16_t lowNoiseLevel;
  uint32_t cycles;
};

static void i2c_master_init() {
  i2c_config_t conf = {};
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = I2C_SDA;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = I2C_SCL;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = 100000;

  ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &conf));
  ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0));

  ESP_LOGI(TAG, "I2C master listo (SDA=%d SCL=%d)", I2C_SDA, I2C_SCL);
}

static esp_err_t send_command(uint8_t cmd) {
  return i2c_master_write_to_device(I2C_PORT, I2C_SLAVE_ADDR, &cmd, 1,
                                    100 / portTICK_PERIOD_MS);
}

static esp_err_t request_data(uint8_t cmd, uint8_t *out, size_t len) {
  return i2c_master_write_read_device(I2C_PORT, I2C_SLAVE_ADDR, &cmd, 1, out, len,
                                      200 / portTICK_PERIOD_MS);
}

extern "C" void app_main(void) {
  i2c_master_init();

  while (true) {
    uint8_t status = 0;
    if (request_data(CMD_GET_STATUS, &status, sizeof(status)) != ESP_OK || status == 0x00) {
      ESP_LOGW(TAG, "Esclavo no disponible o sin datos");
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      continue;
    }

    SensorData data{};
    if (request_data(CMD_GET_DATA, reinterpret_cast<uint8_t *>(&data), sizeof(data)) == ESP_OK) {
      ESP_LOGI(TAG, "Datos del sensor:");
      ESP_LOGI(TAG, "  Ruido actual: %lu mV", static_cast<unsigned long>(data.noise));
      ESP_LOGI(TAG, "  Promedio: %.2f mV", data.noiseAvg);
      ESP_LOGI(TAG, "  Pico: %.2f mV", data.noisePeak);
      ESP_LOGI(TAG, "  Minimo: %.2f mV", data.noiseMin);
      ESP_LOGI(TAG, "  Promedio legal: %.2f mV", data.noiseAvgLegal);
      ESP_LOGI(TAG, "  Max legal: %.2f mV", data.noiseAvgLegalMax);
      ESP_LOGI(TAG, "  Nivel base: %u mV", data.lowNoiseLevel);
      ESP_LOGI(TAG, "  Ciclos: %lu", static_cast<unsigned long>(data.cycles));
    } else {
      ESP_LOGE(TAG, "Error leyendo datos del esclavo");
    }

    float avg = 0.0f;
    if (request_data(CMD_GET_AVG, reinterpret_cast<uint8_t *>(&avg), sizeof(avg)) == ESP_OK) {
      ESP_LOGI(TAG, "Promedio individual: %.2f mV", avg);
    }

    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}
