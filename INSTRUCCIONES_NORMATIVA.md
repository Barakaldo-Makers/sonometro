# Instrucciones de calibración y uso (cumplimiento mínimo Decreto 213/2012)

Este documento describe cómo calibrar y operar el sensor para obtener índices
acústicos básicos (LAeq, Ld, Le, Ln) con aproximación mínima al Decreto 213/2012.

## 1) Requisitos mínimos de hardware

- Módulo de micrófono con **salida analógica** y **ponderación A** integrada.
  - Si el módulo no es A‑weighted, los índices no son comparables con el decreto.
- Para el **SparkFun Sound Detector**:
  - La salida **ENVELOPE** es un seguidor de envolvente (picos), útil para eventos.
  - Para **LAeq/Ld/Le/Ln** es preferible la salida **AUDIO** (señal AC).
  - Si usas **ENVELOPE**, el cálculo es solo aproximado.
- ESP32‑C3 (SuperMini) con ADC calibrado.
- Fuente estable y entorno controlado para calibración.

## 2) Calibración (obligatoria)

Se utiliza un calibrador acústico a 1 kHz (94 dB o 114 dB).

1. Conecta el micrófono y deja 1–2 minutos de estabilización.
2. Coloca el calibrador sobre el micrófono.
3. Lee el valor medio en mV con el firmware:
   - Usa `measurements.noise` y/o `measurements.noiseAvg`.
4. Configura el firmware con los valores de referencia:
   - `config.refDb = 94.0f` (o 114.0f según el calibrador)
   - `config.refMv = <mV medidos>`
5. Guarda el valor `refMv` (debe ser estable y repetible).

### Ejemplo de calibración (Arduino)

```cpp
#include "NoiseSensor.h"

NoiseSensor sensor;

void setup() {
  Serial.begin(115200);
  sensor.begin();
}

void loop() {
  sensor.update();

  if (sensor.isCycleComplete()) {
    const auto &m = sensor.getMeasurements();
    Serial.print("mV medio: ");
    Serial.println(m.noiseAvg);
    Serial.println("Coloca el calibrador y toma este valor como refMv.");
    sensor.resetCycle();
  }
}
```

Después, fija los valores:

```cpp
NoiseSensor::Config config;
config.refDb = 94.0f;
config.refMv = 1000.0f; // reemplaza con el mV medido
```

## 3) Configuración mínima del firmware

```cpp
NoiseSensor::Config config;
config.adcPin = 4;
config.refDb = 94.0f;     // nivel de calibración
config.refMv = 1000.0f;   // mV medidos en calibración
config.assumeAWeighted = true;
config.dayStartHour = 7;
config.eveningStartHour = 19;
config.nightStartHour = 23;
config.trackLdLeLn = true;
```

### Atenuación ADC (ESP32)
Si la señal puede superar 1.1 V, configura la atenuación:

```cpp
#if defined(ESP32)
analogSetPinAttenuation(config.adcPin, ADC_11db);
#endif
```

## 4) Hora actual (Ld/Le/Ln)

Para calcular Ld/Le/Ln es imprescindible informar la hora:

```cpp
sensor.setCurrentHour(hora_0_23);
```

Se recomienda obtener la hora mediante RTC o NTP y actualizarla periódicamente.

## 5) Reset diario de Ld/Le/Ln

Los índices Ld/Le/Ln deben reiniciarse cada 24 h:

```cpp
sensor.resetLdLeLn();
```

## 6) Salidas recomendadas

- `noiseDb`: valor instantáneo dB(A) (calibrado)
- `noiseAvgDb`: LAeq del ciclo
- `noiseAvgLegalDb`: LAeq legal (periodo corto)
- `Ld`, `Le`, `Ln`: índices diurno, vespertino y nocturno

## 7) Limitaciones

- No se aplican correcciones tonales/impulsivas (Kt/Ki/Kf).
- El cumplimiento normativo **completo** requiere instrumentación tipo sonómetro
  clase 1/2 y procedimientos UNE‑ISO 1996‑1/2.
- Este sistema ofrece una aproximación de bajo coste para monitorización.
