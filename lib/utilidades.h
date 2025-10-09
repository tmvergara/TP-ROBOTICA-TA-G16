#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

#include <config.h>
#include <types.h>

extern Servo servoElevador;
extern Servo servoIzquierdo;
extern Servo servoDerecho;

Posicion obtenerPosicionActual();

void esperarPor(int delayMs);

void homeXY();

void movimientoLineal(Posicion &position);

void movimientoArco(Posicion centro, bool sentidoHorario = true, Posicion *fin = nullptr);

void habilitarLapiz(bool habilitar = true);

void posicionEnsamblaje();

bool estaOcupado();

void actualizarPosicionLapiz();

void establecerVelocidad(float nuevaVelocidad);

float obtenerVelocidad();