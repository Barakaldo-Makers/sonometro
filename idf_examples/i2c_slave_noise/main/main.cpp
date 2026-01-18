#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"
#include "esp_log.h"

#include "noisesensor.h"

static const char *TAG = "NoiseI2CSlave";

// I2C slave configuration
static constexpr i2c_port_t I2C_PORT = I2C_NUM_0;
// ESP32-C3 SuperMini: SDA en GPIO8 y SCL en GPIO10 (según tu hardware).
static constexpr gpio_num_t I2C_SDA = GPIO_NUM_8;
static constexpr gpio_num_t I2C_SCL = GPIO_NUM_10;
static constexpr uint8_t I2C_SLAVE_ADDR = 0x08;
static constexpr int I2C_RX_BUF = 128;
static constexpr int I2C_TX_BUF = 128;

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

static SensorData g_data{};
static bool g_data_ready = false;

static void i2c_slave_init() {
  i2c_config_t conf = {};
  conf.mode = I2C_MODE_SLAVE;
  conf.sda_io_num = I2C_SDA;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = I2C_SCL;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.slave.addr_10bit_en = 0;
  conf.slave.slave_addr = I2C_SLAVE_ADDR;

  ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &conf));
  ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, conf.mode, I2C_RX_BUF, I2C_TX_BUF, 0));

  ESP_LOGI(TAG, "I2C slave listo en 0x%02X (SDA=%d SCL=%d)", I2C_SLAVE_ADDR, I2C_SDA,
           I2C_SCL);
}

static void prepare_response(uint8_t command, uint8_t *tx_buf, size_t *tx_len) {
  *tx_len = 0;
  switch (command) {
    case CMD_GET_DATA:
      memcpy(tx_buf, &g_data, sizeof(g_data));
      *tx_len = sizeof(g_data);
      break;
    case CMD_GET_AVG:
      memcpy(tx_buf, &g_data.noiseAvg, sizeof(float));
      *tx_len = sizeof(float);
      break;
    case CMD_GET_PEAK:
      memcpy(tx_buf, &g_data.noisePeak, sizeof(float));
      *tx_len = sizeof(float);
      break;
    case CMD_GET_MIN:
      memcpy(tx_buf, &g_data.noiseMin, sizeof(float));
      *tx_len = sizeof(float);
      break;
    case CMD_GET_LEGAL:
      memcpy(tx_buf, &g_data.noiseAvgLegal, sizeof(float));
      *tx_len = sizeof(float);
      break;
    case CMD_GET_LEGAL_MAX:
      memcpy(tx_buf, &g_data.noiseAvgLegalMax, sizeof(float));
      *tx_len = sizeof(float);
      break;
    case CMD_GET_STATUS: {
      uint8_t status = g_data_ready ? 0x01 : 0x00;
      memcpy(tx_buf, &status, sizeof(status));
      *tx_len = sizeof(status);
      break;
    }
    case CMD_RESET:
      // El master puede pedir reset, pero el sensor se resetea en el loop.
      *tx_len = 0;
      break;
    default:
      *tx_len = 0;
      break;
  }
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "Inicializando NoiseSensor (ESP-IDF)");

  i2c_slave_init();

  NoiseSensor::Config config;
  config.adc_gpio = 4;
  config.logLevel = NoiseSensor::LOG_INFO;

  NoiseSensor sensor(config);
  ESP_ERROR_CHECK(sensor.begin());

  uint8_t rx_buf[4] = {0};
  uint8_t tx_buf[64] = {0};
  size_t tx_len = 0;

  uint64_t last_update = 0;

  while (true) {
    sensor.update();

    uint64_t now = esp_timer_get_time() / 1000ULL;
    if (now - last_update >= 1000) {
      last_update = now;
      const auto &m = sensor.getMeasurements();
      g_data.noise = m.noise;
      g_data.noiseAvg = m.noiseAvg;
      g_data.noisePeak = m.noisePeak;
      g_data.noiseMin = m.noiseMin;
      g_data.noiseAvgLegal = m.noiseAvgLegal;
      g_data.noiseAvgLegalMax = m.noiseAvgLegalMax;
      g_data.lowNoiseLevel = static_cast<uint16_t>(m.lowNoiseLevel);
      g_data.cycles = m.cycles;
      g_data_ready = true;

      if (sensor.isCycleComplete()) {
        sensor.resetCycle();
      }
    }

    int len = i2c_slave_read_buffer(I2C_PORT, rx_buf, sizeof(rx_buf), 10 / portTICK_PERIOD_MS);
    if (len > 0) {
      uint8_t cmd = rx_buf[0];
      prepare_response(cmd, tx_buf, &tx_len);
      if (tx_len > 0) {
        i2c_slave_write_buffer(I2C_PORT, tx_buf, tx_len, 10 / portTICK_PERIOD_MS);
      }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
