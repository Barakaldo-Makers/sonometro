# Basic Usage Example

This example demonstrates the basic usage of the NoiseSensor library.

## Behavior

- Reads noise levels from GPIO 4
- Displays measurements every 120 seconds (duty cycle)
- Shows average, peak, minimum, legal max, and cycle information
- Automatically manages measurement cycles

## Requirements

- ESP32-C3, ESP32-S2, or ESP32-S3 board
- Sound sensor or microphone connected to GPIO 4

## Wiring

Connect your sound sensor/microphone to:
- ADC pin: GPIO 4

## Serial Output

The example prints detailed measurement information to the serial monitor at 115200 baud.

## Important Notes

1. Always call `sensor.resetCycle()` AFTER reading the data
2. The readings are reset automatically when `resetCycle()` is called
3. The legal measurements run independently every 5 seconds

