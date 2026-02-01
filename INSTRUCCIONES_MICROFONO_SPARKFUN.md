# Micrófono SparkFun Sound Detector (SEN-12642/SEN-14262)

Esta guía resume cómo usar el módulo SparkFun Sound Detector con el ESP32‑C3
para mejorar la estabilidad y la calibración.

## 1) Alimentación

- El módulo está diseñado para **3.5–5.5 V**, y **5 V es lo ideal**.
- Es **sensible al ruido** de la fuente.
  - Evita alimentar desde USB ruidoso.
  - Si es posible, usa un regulador limpio o batería.

## 2) Salidas del módulo

- **AUDIO**: señal AC con offset ~Vcc/2 (mejor para LAeq).
- **ENVELOPE**: seguidor de envolvente (picos), útil para detección, no RMS real.
- **GATE**: salida digital con umbral (solo eventos).

Para cumplimiento acústico mínimo:
**usa AUDIO si es posible**. ENVELOPE solo da una aproximación.

## 3) Protección del ADC (muy importante)

El ESP32‑C3 **no admite 5 V** en la entrada ADC.

Si alimentas el módulo a 5 V:
- Usa **divisor resistivo** en la salida a GPIO4.
  - Ejemplo: 10k (arriba) y 20k (abajo) → ~3.3 V máx.

```
ENVELOPE/AUDIO ---- 10k ----+---- GPIO4
                            |
                           20k
                            |
                           GND
```

## 4) Ganancia del preamplificador

La sensibilidad se ajusta con **R17/R3**:
- Para **menos sensibilidad**, añade R17 en paralelo (22k–100k).
- Para **más sensibilidad**, quita R3 y usa R17 (220k–1M).

Consulta la tabla del documento SparkFun para seleccionar el valor.

## 5) Calibración rápida

1. Usa calibrador acústico de 94 dB/1 kHz.
2. Mide el valor mV con el firmware.
3. Ajusta:
   - `refDb = 94.0`
   - `refMv = <mV medidos>`

## 6) Recomendaciones de montaje

- Evita vibraciones (usa espuma o soporte flexible).
- Evita viento directo sobre el micrófono.
- Mantén cables cortos para reducir ruido.
