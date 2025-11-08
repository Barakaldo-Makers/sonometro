#include <Arduino.h>
#include "NoiseSensor.h"

NoiseSensor sensor;

void setup() {
    Serial.begin(115200);
    sensor.begin();
}

void loop() {
    sensor.update();

    if (sensor.isCycleComplete()) {
        const auto &measurements = sensor.getMeasurements();
        Serial.printf("Average noise: %.2f\n", measurements.noiseAvg);
        Serial.printf("Peak noise: %u\n", measurements.noisePeak);
        Serial.printf("Min noise: %u\n", measurements.noiseMin);
        sensor.resetCycle();
    }

    delay(10);
}
