// Configuración de WiFi y servidor
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// Librerías propias
#include <config.h>
#include <gcode.h>

AsyncWebServer servidor(80);

const char WIFI_SSID[] = "G16_Plotter_AP";
const char WIFI_PASSWORD[] = "tototo123";

void setup()
{
  Serial.begin(115200);

  // Inicializar SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("Error al montar SPIFFS");
    return;
  }

  servoElevador.attach(pinServoElevador);
  servoIzquierdo.attach(pinServoIzquierdo);
  servoDerecho.attach(pinServoDerecho);

  // Configurar punto de acceso WiFi
  Serial.println("Configurando punto de acceso WiFi...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD); // SSID y contraseña del AP

  Serial.println("\nPunto de acceso WiFi creado.");
  Serial.print("Nombre de red (SSID): ");
  Serial.println(WIFI_SSID);
  Serial.print("Contraseña: ");
  Serial.println(WIFI_PASSWORD);
  Serial.print("Dirección IP del AP: ");
  Serial.println(WiFi.softAPIP());

  Serial.println("Iniciando posición XY.");
  homeXY();

  // Definir manejadores de rutas
  // Servir archivos estáticos desde SPIFFS
  servidor.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // También permitir acceso directo a archivos específicos
  servidor.onNotFound([](AsyncWebServerRequest *solicitud)
                      {
    if (solicitud->method() == HTTP_GET) {
      solicitud->send(404, "text/plain", "Archivo no encontrado");
    } else {
      solicitud->send(405, "text/plain", "Método no permitido");
    } });

  servidor.on("/gcode", HTTP_POST, [](AsyncWebServerRequest *solicitud) {}, NULL, [](AsyncWebServerRequest *solicitud, uint8_t *data, size_t len, size_t index, size_t total)
              {
    String gcode = "";
    for(size_t i = 0; i < len; i++) {
      gcode += (char)data[i];
    }
    Serial.println("G-code recibido:");
    Serial.println(gcode);
    setGCode(gcode);
    solicitud->send(200, "text/plain", "OK"); });

  servidor.on("/status", HTTP_GET, [](AsyncWebServerRequest *solicitud)
              {
      AsyncResponseStream *respuesta = solicitud->beginResponseStream("application/json");

      JsonDocument doc;

      Posicion pos = obtenerPosicionActual();
      doc["x"] = pos.x;
      doc["y"] = pos.y;
      doc["ocupado"] = estaOcupado();
      doc["elevado"] = servoElevador.read() == ANGULO_ELEVADOR_ARRIBA;

      serializeJson(doc, *respuesta);

      solicitud->send(respuesta); });

  servidor.on("/config", HTTP_GET, [](AsyncWebServerRequest *solicitud)
              {
      AsyncResponseStream *respuesta = solicitud->beginResponseStream("application/json");

      JsonDocument doc;

      Posicion pos = obtenerPosicionActual();
      doc["xMin"] = X_MIN;
      doc["xMax"] = X_MAX;
      doc["yMin"] = Y_MIN;
      doc["yMax"] = Y_MAX;
      doc["inicioX"] = POSICION_INICIO.x;
      doc["inicioY"] = POSICION_INICIO.y;
      doc["velocidad"] = obtenerVelocidad();
      doc["velocidadMin"] = VELOCIDAD_MIN;
      doc["velocidadMax"] = VELOCIDAD_MAX;

      serializeJson(doc, *respuesta);

      solicitud->send(respuesta); });

  servidor.on("/assembly", HTTP_POST, [](AsyncWebServerRequest *solicitud)
              {
              Serial.println("Ensamblaje...");
              posicionEnsamblaje();
              solicitud->send(200, "text/plain", "OK"); });

  servidor.on("/restart", HTTP_POST, [](AsyncWebServerRequest *solicitud)
              {
              Serial.println("Reiniciando ESP...");
              ESP.restart(); });

  servidor.begin();
}

void loop()
{
  machineLoop();
  actualizarPosicionLapiz();
}