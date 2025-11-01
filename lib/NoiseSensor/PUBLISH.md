# Guía para Publicar NoiseSensor en PlatformIO Registry

## Archivos Necesarios

La librería está lista para publicar. Incluye:
- ✅ `library.json` - Metadata para PlatformIO
- ✅ `library.properties` - Compatibilidad con Arduino IDE
- ✅ `LICENSE` - Licencia MIT
- ✅ `README.md` - Documentación completa
- ✅ Código fuente (`.h` y `.cpp`)
- ✅ Ejemplos funcionales

## Paso 1: Preparar el Repositorio GitHub

1. Crea un repositorio en GitHub:
   ```
  "https://github.com/Barakaldo-Makers/sonometro"
   ```

2. Sube todos los archivos de la carpeta `lib/NoiseSensor/` al repositorio.

3. Actualiza en `library.json` y `library.properties`:
   - Reemplaza `YOUR_USERNAME` con tu nombre de usuario de GitHub
   - Ajusta la URL del repositorio si es necesario

## Paso 2: Crear Release en GitHub

1. Ve a tu repositorio en GitHub
2. Haz clic en "Releases" → "Create a new release"
3. Tag version: `v1.0.0`
4. Release title: `NoiseSensor v1.0.0`
5. Description:
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
6. Haz clic en "Publish release"

## Paso 3: Publicar en PlatformIO

### Opción A: Página Web (Recomendado)

1. Ve a https://registry.platformio.org
2. Inicia sesión con tu cuenta de GitHub
3. Haz clic en "Submit a Library"
4. Pega la URL de tu repositorio de GitHub
5. Selecciona la versión del release (v1.0.0)
6. Verifica que todos los archivos estén correctos
7. Haz clic en "Submit"

### Opción B: CLI

```bash
# Instalar pio (si no lo tienes)
pip install platformio

# Publicar la librería
pio package publish
```

## Verificación

Una vez publicada, cualquier usuario podrá instalar tu librería con:

```bash
pio lib install "NoiseSensor"
```

O añadiéndola al `platformio.ini`:
```ini
[env]
lib_deps = 
    NoiseSensor
```

## Próximas Versiones

Para nuevas versiones:
1. Actualiza la versión en `library.json` y `library.properties`
2. Crea un nuevo release en GitHub
3. Publica en PlatformIO nuevamente

## Contacto y Soporte

- Issues: GitHub Issues en tu repositorio
- Documentación: README.md del proyecto

