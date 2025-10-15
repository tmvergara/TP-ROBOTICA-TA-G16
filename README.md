# Carpeta Data

Esta carpeta contiene los archivos de la interfaz web del Robot Plotter.

## Estructura

- `index.html` - Página principal de la interfaz web

## Importante

Los archivos web deben ser copiados a la carpeta `data/` en la raíz del proyecto para que puedan ser subidos al sistema de archivos SPIFFS del ESP32.

## Cómo subir archivos al ESP32

1. Asegúrate de que los archivos estén en la carpeta `data/`
2. Ejecuta el comando: `pio run --target uploadfs`
3. Luego sube el código principal: `pio run --target upload`

## Acceso a la interfaz

Una vez programado el ESP32:

1. Conecta tu dispositivo a la red WiFi: `G16_Plotter_AP`
2. Contraseña: `tototo123`
3. Abre un navegador y ve a: `http://192.168.4.1`
