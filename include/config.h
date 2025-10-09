#pragma once

#include <Arduino.h>
#include <types.h>

// Constantes de geometría del robot
const float L1 = 25.8; // Distancia entre los dos servos (mm)
const float L2 = 60.0; // Longitud del primer brazo conectado al servo (mm)
const float L3 = 70.0; // Longitud del segundo brazo conectado al lápiz (mm)

// Posiciones de elevación (para que se levante el lápiz)
const int ANGULO_ELEVADOR_ABAJO = 120;  // Ángulo de bajada del elevador
const int ANGULO_ELEVADOR_ARRIBA = 172; // Ángulo de subida del elevador
const int ESPERA_ELEVADOR = 300;        // Tiempo de espera del elevador

// Restricciones para la posición del lápiz
const float Y_MIN = 25;  // Posición Y mínima
const float Y_MAX = 125; // Posición Y máxima
const float X_MIN = -50; // Posición X mínima
const float X_MAX = 50;  // Posición X máxima

const Posicion POSICION_INICIO = {x : 0, y : Y_MIN}; // Posición de inicio

// Velocidad
const float VELOCIDAD_MIN = 10;      // Velocidad mínima mm/s
const float VELOCIDAD_MAX = 300;     // Velocidad máxima mm/s
const float VELOCIDAD_DEFECTO = 100; // Velocidad del lápiz mm/s

// Tolerancia para comparar posiciones
const float DELTA_MAX = 0.1;

// Pines de los servos
static const int pinServoElevador = GPIO_NUM_0;
static const int pinServoIzquierdo = GPIO_NUM_2;
static const int pinServoDerecho = GPIO_NUM_1;