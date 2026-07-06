#include "raylib.h"

struct Casilla { int tipo; float progreso; };

Casilla tablero[3][3];
int jugadorActual = 1;
bool juegoTerminado = false;
int estadoResultado = 0;

void InicializarJuego() {
    for (int f = 0; f < 3; f++)
        for (int c = 0; c < 3; c++)
            tablero[f][c].tipo = 0, tablero[f][c].progreso = 0.0f;
    jugadorActual = 1;
    juegoTerminado = false;
    estadoResultado = 0;
}

void ComprobarGanador() {
    for (int f = 0; f < 3; f++)
        if (tablero[f][0].tipo != 0 && tablero[f][0].tipo == tablero[f][1].tipo && tablero[f][0].tipo == tablero[f][2].tipo)
            { estadoResultado = tablero[f][0].tipo; juegoTerminado = true; return; }
    for (int c = 0; c < 3; c++)
        if (tablero[0][c].tipo != 0 && tablero[0][c].tipo == tablero[1][c].tipo && tablero[0][c].tipo == tablero[2][c].tipo)
            { estadoResultado = tablero[0][c].tipo; juegoTerminado = true; return; }
    if (tablero[0][0].tipo != 0 && tablero[0][0].tipo == tablero[1][1].tipo && tablero[0][0].tipo == tablero[2][2].tipo)
        { estadoResultado = tablero[0][0].tipo; juegoTerminado = true; return; }
    if (tablero[0][2].tipo != 0 && tablero[0][2].tipo == tablero[1][1].tipo && tablero[0][2].tipo == tablero[2][0].tipo)
        { estadoResultado = tablero[0][2].tipo; juegoTerminado = true; return; }
    for (int f = 0; f < 3; f++)
        for (int c = 0; c < 3; c++)
            if (tablero[f][c].tipo == 0) return;
    estadoResultado = 3;
    juegoTerminado = true;
}

void ActualizarJuego() {
    if (juegoTerminado && IsKeyPressed(KEY_R)) { InicializarJuego(); return; }
    for (int f = 0; f < 3; f++)
        for (int c = 0; c < 3; c++)
            if (tablero[f][c].tipo != 0 && tablero[f][c].progreso < 1.0f) {
                tablero[f][c].progreso += 0.03f;
                if (tablero[f][c].progreso > 1.0f) tablero[f][c].progreso = 1.0f;
            }
    if (juegoTerminado) return;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 pos = GetMousePosition();
        int columna = (int)pos.x / 200, fila = (int)pos.y / 200;
        if (fila >= 0 && fila < 3 && columna >= 0 && columna < 3 && tablero[fila][columna].tipo == 0) {
            tablero[fila][columna].tipo = jugadorActual;
            tablero[fila][columna].progreso = 0.0f;
            ComprobarGanador();
            if (!juegoTerminado) jugadorActual = (jugadorActual == 1) ? 2 : 1;
        }
    }
}

void DibujarJuego() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawLineEx({ 200, 0 }, { 200, 600 }, 5, GRAY);
    DrawLineEx({ 400, 0 }, { 400, 600 }, 5, GRAY);
    DrawLineEx({ 0, 200 }, { 600, 200 }, 5, GRAY);
    DrawLineEx({ 0, 400 }, { 600, 400 }, 5, GRAY);
    for (int f = 0; f < 3; f++) {
        for (int c = 0; c < 3; c++) {
            int cx = c * 200 + 100, cy = f * 200 + 100;
            float p = tablero[f][c].progreso;
            if (tablero[f][c].tipo == 1) {
                float tamAct = 40.0f * p;
                DrawLineEx({ (float)cx - tamAct, (float)cy - tamAct }, { (float)cx + tamAct, (float)cy + tamAct }, 8, BLUE);
                if (p > 0.6f) {
                    float tamAct2 = 40.0f * ((p - 0.6f) * 2.5f);
                    if (tamAct2 > 40.0f) tamAct2 = 40.0f;
                    DrawLineEx({ (float)cx + tamAct2, (float)cy - tamAct2 }, { (float)cx - tamAct2, (float)cy + tamAct2 }, 8, BLUE);
                }
            } else if (tablero[f][c].tipo == 2) {
                DrawRing({ (float)cx, (float)cy }, 41.0f, 49.0f, 0.0f, 360.0f * p, 16, RED);
            }
        }
    }
    if (juegoTerminado) {
        DrawRectangle(50, 220, 500, 160, Fade(BLACK, 0.85f));
        if (estadoResultado == 1) DrawText("¡GANÓ EL JUGADOR X!", 110, 250, 32, BLUE);
        else if (estadoResultado == 2) DrawText("¡GANÓ EL JUGADOR O!", 110, 250, 32, RED);
        else if (estadoResultado == 3) DrawText("¡EMPATE EN EL TABLERO!", 100, 250, 32, LIGHTGRAY);
        DrawText("Presiona 'R' para volver a jugar", 140, 310, 20, RAYWHITE);
    }
    EndDrawing();
}

int main() {
    InitWindow(600, 600, "3 en Raya Completo - Raylib");
    SetTargetFPS(30);
    InicializarJuego();
    while (!WindowShouldClose()) { ActualizarJuego(); DibujarJuego(); }
    CloseWindow();
    return 0;
}
