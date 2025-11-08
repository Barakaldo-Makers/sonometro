# Guía para Publicar NoiseSensor en PlatformIO Registry

## Archivos Necesarios

La librería está lista para publicar. Incluye:
- ✅ `library.json` – metadata de PlatformIO
- ✅ `library.properties` – compatibilidad Arduino IDE
- ✅ `LICENSE` – licencia GPL-3.0-or-later
- ✅ `README.md` – documentación
- ✅ Código fuente en `src/`
- ✅ Ejemplo en `examples/BasicUsage`

## Paso 1: Preparar el repositorio en GitHub

1. Crea un repositorio, por ejemplo:
   ```
   https://github.com/Barakaldo-Makers/sonometro
   ```
2. Sube todos los archivos del proyecto (mantén la estructura actual).
3. Asegúrate de que `library.json` y `library.properties` apuntan a tu repositorio.

## Paso 2: Crear un release en GitHub

1. Ve a tu repositorio y crea un release nuevo.
2. Tag sugerido: `v1.0.0`
3. Título: `NoiseSensor v1.0.0`
4. Descripción básica:
   ```
   Initial release of NoiseSensor library for ESP32

   Features:
   - Noise level measurements on ESP32-C3, ESP32-S2, ESP32-S3
   - LAeq calculations
   - Peak and minimum detection
   - Legal average measurements (PMI)
   - Dynamic noise level adjustment
   - Independent measurement cycles
   ```
5. Publica el release.

## Paso 3: Publicar en PlatformIO

### Opción A: Web (recomendado)
1. Visita https://registry.platformio.org
2. Inicia sesión con GitHub
3. Click en “Submit a Library”
4. Indica la URL de tu repositorio
5. Selecciona la versión correspondiente
6. Verifica los datos y envía

### Opción B: CLI
```bash
# Instalar PlatformIO Core si no lo tienes
ython -m pip install platformio

# Publicar la librería
pio package publish
```

## Verificación
Tras publicarla, se puede instalar con:
```bash
pio lib install "NoiseSensor"
```

O añadirlo al `platformio.ini` de un proyecto:
```ini
[env]
lib_deps = 
    NoiseSensor
```

## Próximas versiones
1. Actualiza versiones en `library.json` y `library.properties`
2. Crea un nuevo release
3. Publica nuevamente en PlatformIO

## Contacto
- Issues y soporte: GitHub Issues del repositorio
- Documentación: README.md del proyecto

