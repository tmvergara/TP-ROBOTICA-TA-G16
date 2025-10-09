#include <../lib/gcode.h>

String gCode = "";

GCodeLine parseGCodeLine(const String &line)
{
    GCodeLine gcode;
    char lineBuffer[line.length() + 1];
    line.toCharArray(lineBuffer, line.length() + 1);

    char *token = strtok(lineBuffer, " ");
    if (token != nullptr)
    {
        gcode.cmd = token;
    }

    while ((token = strtok(nullptr, " ")) != nullptr)
    {
        String param(token);
        if (param.length() > 1)
        {
            String key = param.substring(0, 1);
            String valueStr = param.substring(1);
            gcode.params[key] = valueStr.toFloat();
        }
    }

    return gcode;
}

void executeLine(GCodeLine &gline)
{
    if (gline.cmd == "G1")
    {
        Posicion pos;
        pos.x = gline.params["X"];
        pos.y = gline.params["Y"];

        Serial.printf("G1: Movimiento lineal X: %.2f, Y: %.2f\n", pos.x, pos.y);
        movimientoLineal(pos);
    }
    else if (gline.cmd == "G2" || gline.cmd == "G3")
    {
        Posicion center;
        center.x = gline.params["I"];
        center.y = gline.params["J"];

        Posicion end;
        end.x = gline.params["X"];
        end.y = gline.params["Y"];

        bool clockwise = gline.cmd == "G2";

        Serial.printf("%s: Movimiento de arco a X: %.2f, Y: %.2f, I: %.2f, J: %.2f\n", gline.cmd, end.x, end.y, center.x, center.y);
        movimientoArco(center, clockwise, &end);
    }
    else if (gline.cmd == "G28")
    {
        Serial.println("G28: Homing XY");
        homeXY();
    }
    else if (gline.cmd == "M3")
    {
        Serial.println("M3: Bajar lápiz");
        habilitarLapiz(true);
    }
    else if (gline.cmd == "M5")
    {
        Serial.println("M5: Subir lápiz");
        habilitarLapiz(false);
    }
    else if (gline.cmd == "M999")
    {
        Serial.println("M999: Reiniciando ESP...");
        ESP.restart();
    }
    else if (gline.cmd == "M203")
    {
        float newSpeed = gline.params["X"];
        Serial.printf("M203: Estableciendo velocidad a %.2f\n", newSpeed);
        establecerVelocidad(newSpeed);
    }
    else
    {
        Serial.printf("Comando desconocido: %s\n", gline.cmd.c_str());
    }
}

// Metodos Publicos
void setGCode(String newGCode)
{
    gCode = newGCode;
}

void machineLoop()
{
    if (estaOcupado() || gCode.isEmpty())
        return;

    int lineEnd = gCode.indexOf('\n');

    String line = (lineEnd == -1) ? gCode : gCode.substring(0, lineEnd);
    gCode = (lineEnd == -1) ? "" : gCode.substring(lineEnd + 1);

    line.trim();

    if (line.isEmpty())
        return;

    GCodeLine gline = parseGCodeLine(line);
    executeLine(gline);
}