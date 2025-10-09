
#include <../lib/utilidades.h>

struct Arco
{
    Posicion centro;
    float radio;
    int direccion;
    float deltaAngulo;
};

Servo servoElevador;
Servo servoIzquierdo;
Servo servoDerecho;

// Posición inicial del lápiz
Posicion posicionActual = POSICION_INICIO;

// Objetivos
Posicion *posicionObjetivoLineal = nullptr;
Arco *objetivoArco = nullptr;
int esperarHasta;

// Última actualización de tiempo
int ultimaActualizacion;

// Velocidad
float velocidad = VELOCIDAD_DEFECTO;

// Funciones estáticas
/**
 * @brief Devuelve el ángulo entre dos puntos en radianes
 */
float anguloPuntos(const Posicion &p1, const Posicion &p2)
{
    return atan2(p2.y - p1.y, p2.x - p1.x);
}

/**
 * @brief Devuelve la distancia entre dos puntos
 */
float distanciaPuntos(const Posicion &p1, const Posicion &p2)
{
    return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

// Función para calcular ángulos de servos
bool calcularAngulosServos(const Posicion &posicion, Angulos &angulos)
{
    // Invertir eje x
    int x = -posicion.x;
    int y = posicion.y;

    if (x < X_MIN || x > X_MAX || y < Y_MIN || y > Y_MAX)
    {
        return false;
    }
    // Calcular las distancias horizontales desde cada servo
    float x1 = x + L1 / 2; // Desplazamiento para el servo izquierdo
    float x2 = x - L1 / 2; // Desplazamiento para el servo derecho

    // Calcular la distancia desde cada base de servo al punto objetivo
    float D1 = sqrt(pow(x1, 2) + pow(y, 2));
    float D2 = sqrt(pow(x2, 2) + pow(y, 2));

    // Verificar si el punto es alcanzable
    if (D1 > (L2 + L3) || D2 > (L2 + L3) || D1 < fabs(L2 - L3) || D2 < fabs(L2 - L3))
    {
        return false; // Objetivo inalcanzable
    }

    // Calcular los ángulos para el servo izquierdo (θ1)
    float gamma1 = atan2(x1, y); // Ángulo desde el servo al punto
    float theta1 = acos((pow(L2, 2) + pow(D1, 2) - pow(L3, 2)) / (2 * L2 * D1));
    angulos.izquierdo = degrees(theta1 - gamma1); // Rotación antihoraria para servo izquierdo

    // Calcular los ángulos para el servo derecho (θ2)
    float gamma2 = atan2(x2, y); // Ángulo desde el servo al punto
    float theta2 = acos((pow(L2, 2) + pow(D2, 2) - pow(L3, 2)) / (2 * L2 * D2));
    angulos.derecho = 180 - degrees(gamma2 + theta2); // Rotación antihoraria para servo derecho

    return true; // Cálculo exitoso
}

bool sonPosicionesIguales(const Posicion &pos1, const Posicion &pos2)
{
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    return abs(dx) <= DELTA_MAX && abs(dy) <= DELTA_MAX;
}

// Funciones de actuación
bool establecerAngulosServos(Angulos &angulos)
{
    int izquierdo = map(angulos.izquierdo * 100, 0, 18000, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
    int derecho = map(angulos.derecho * 100, 0, 18000, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);

    if (izquierdo < MIN_PULSE_WIDTH || izquierdo > MAX_PULSE_WIDTH || derecho < MIN_PULSE_WIDTH || derecho > MAX_PULSE_WIDTH)
        return false;

    servoIzquierdo.write(izquierdo);
    servoDerecho.write(derecho);

    return true;
}

void establecerPosicionLapiz(const Posicion &posicion)
{
    Angulos angulos;

    bool puedeMoverse = calcularAngulosServos(posicion, angulos);
    bool puedeEstablecer = establecerAngulosServos(angulos);

    // En caso de posición inalcanzable, reiniciar el ESP
    if (!puedeMoverse || !puedeEstablecer)
    {
        ESP.restart();
    }

    posicionActual = posicion;
}

void actualizarMovimientoLineal(float delta)
{
    if (!posicionObjetivoLineal)
        return;

    float distanciaViaje = velocidad * delta;

    // Calcular la distancia al punto objetivo
    float dx = posicionActual.x - posicionObjetivoLineal->x;
    float dy = posicionActual.y - posicionObjetivoLineal->y;
    float angulo = atan2(dy, dx);
    float distanciaMaxima = sqrt(pow(dx, 2) + pow(dy, 2));
    float distancia = min(distanciaViaje, distanciaMaxima);

    // Mover el lápiz a la nueva posición
    Posicion nuevaPosicion;
    nuevaPosicion.x = posicionActual.x - cos(angulo) * distancia;
    nuevaPosicion.y = posicionActual.y - sin(angulo) * distancia;

    establecerPosicionLapiz(nuevaPosicion);

    if (sonPosicionesIguales(posicionActual, *posicionObjetivoLineal))
    {
        posicionObjetivoLineal = nullptr;
    }
}

void actualizarMovimientoArco(float delta)
{
    if (!objetivoArco)
        return;

    Arco &arco = *objetivoArco;

    float deltaAngularRad = velocidad * delta / arco.radio;

    float anguloActualRad = anguloPuntos(arco.centro, posicionActual);
    float incrementoAngulo = arco.direccion * deltaAngularRad;
    arco.deltaAngulo += incrementoAngulo;
    anguloActualRad += incrementoAngulo;

    if (arco.deltaAngulo >= 2 * PI)
    {
        objetivoArco = nullptr;
        return;
    }

    Posicion nuevaPosicion;
    nuevaPosicion.x = arco.centro.x + arco.radio * cos(anguloActualRad);
    nuevaPosicion.y = arco.centro.y + arco.radio * sin(anguloActualRad);

    establecerPosicionLapiz(nuevaPosicion);
}

void actualizarPosicionLapiz()
{
    int tiempoActual = millis();
    float delta = (tiempoActual - ultimaActualizacion) / 1000.0;
    ultimaActualizacion = tiempoActual;

    if (tiempoActual < esperarHasta)
        return;

    if (posicionObjetivoLineal)
    {
        actualizarMovimientoLineal(delta);
    }
    else if (objetivoArco)
    {
        actualizarMovimientoArco(delta);
    }
}

void reiniciarObjetivos()
{
    posicionObjetivoLineal = nullptr;
    objetivoArco = nullptr;
}

// Métodos públicos
Posicion obtenerPosicionActual()
{
    return posicionActual;
}

void esperarPor(int delayMs)
{
    esperarHasta = millis() + delayMs;
}

void homeXY()
{
    reiniciarObjetivos();
    habilitarLapiz(false);
    establecerPosicionLapiz(POSICION_INICIO);
}

void movimientoLineal(Posicion &posicion)
{
    reiniciarObjetivos();
    posicionObjetivoLineal = new Posicion();
    posicionObjetivoLineal->x = posicion.x;
    posicionObjetivoLineal->y = posicion.y;
}

void movimientoArco(Posicion centro, bool sentidoHorario, Posicion *fin)
{
    reiniciarObjetivos();

    static Arco arco;
    arco.centro = centro;
    arco.radio = distanciaPuntos(centro, posicionActual);
    arco.direccion = sentidoHorario ? 1 : -1;
    arco.deltaAngulo = 0;

    objetivoArco = &arco;
}

void habilitarLapizInterno(bool habilitar)
{
    servoElevador.write(habilitar ? ANGULO_ELEVADOR_ABAJO : ANGULO_ELEVADOR_ARRIBA);
}

void habilitarLapiz(bool habilitar)
{
    reiniciarObjetivos();

    habilitarLapizInterno(habilitar);
    esperarPor(ESPERA_ELEVADOR);
}

void posicionEnsamblaje()
{
    habilitarLapizInterno(true);
    Angulos angulos = {0, 180};
    establecerAngulosServos(angulos);
}

bool estaOcupado()
{
    return posicionObjetivoLineal || objetivoArco || millis() < esperarHasta;
}

float obtenerVelocidad()
{
    return velocidad;
}

void establecerVelocidad(float nuevaVelocidad)
{
    velocidad = constrain(nuevaVelocidad, VELOCIDAD_MIN, VELOCIDAD_MAX);
}
