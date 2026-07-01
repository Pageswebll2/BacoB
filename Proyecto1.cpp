#include "raylib.h"

// 1. Estructura para guardar el estado y la animacion de cada casilla
struct Casilla {
    int tipo;          // 0 = vacio, 1 = X, 2 = O
    float progreso;    // Progreso de animacion: de 0.0f a 1.0f
};

// 2. Variables Globales
Casilla tablero[3][3];
int jugadorActual = 1;
bool juegoTerminado = false;
int estadoResultado = 0; // 0 = jugando, 1 = gano X, 2 = gano O, 3 = empate

// 3. Inicializar o resetear el tablero
void InicializarJuego() {
    for (int f = 0; f < 3; f++) {
        for (int c = 0; c < 3; c++) {
            tablero[f][c].tipo = 0;
            tablero[f][c].progreso = 0.0f;
        }
    }
    jugadorActual = 1;
    juegoTerminado = false;
    estadoResultado = 0;
}

// Función auxiliar para comprobar las 8 combinaciones posibles
void ComprobarGanador() {
    // Verificar Filas
    for (int f = 0; f < 3; f++) {
        if (tablero[f][0].tipo != 0 && tablero[f][0].tipo == tablero[f][1].tipo && tablero[f][0].tipo == tablero[f][2].tipo) {
            estadoResultado = tablero[f][0].tipo;
            juegoTerminado = true;
            return;
        }
    }

    // Verificar Columnas
    for (int c = 0; c < 3; c++) {
        if (tablero[0][c].tipo != 0 && tablero[0][c].tipo == tablero[1][c].tipo && tablero[0][c].tipo == tablero[2][c].tipo) {
            estadoResultado = tablero[0][c].tipo;
            juegoTerminado = true;
            return;
        }
    }

    // Verificar Diagonal Principal
    if (tablero[0][0].tipo != 0 && tablero[0][0].tipo == tablero[1][1].tipo && tablero[0][0].tipo == tablero[2][2].tipo) {
        estadoResultado = tablero[0][0].tipo;
        juegoTerminado = true;
        return;
    }

    // Verificar Diagonal Secundaria
    if (tablero[0][2].tipo != 0 && tablero[0][2].tipo == tablero[1][1].tipo && tablero[0][2].tipo == tablero[2][0].tipo) {
        estadoResultado = tablero[0][2].tipo;
        juegoTerminado = true;
        return;
    }

    // Verificar si hay empate (si el tablero se lleno y nadie gano)
    bool lleno = true;
    for (int f = 0; f < 3; f++) {
        for (int c = 0; c < 3; c++) {
            if (tablero[f][c].tipo == 0) lleno = false;
        }
    }

    if (lleno) {
        estadoResultado = 3; // Estado de empate
        juegoTerminado = true;
    }
}

// 4. Logica de actualizacion (Eventos, Animaciones y Teclas)
void ActualizarJuego() {
    // Si el juego termino y presionan 'R', se reinicia el tablero por completo
    if (juegoTerminado && IsKeyPressed(KEY_R)) {
        InicializarJuego();
        return;
    }

    // Avanzar las animaciones de todas las fichas en cada fotograma
    for (int f = 0; f < 3; f++) {
        for (int c = 0; c < 3; c++) {
            if (tablero[f][c].tipo != 0 && tablero[f][c].progreso < 1.0f) {
                tablero[f][c].progreso += 0.03f;
                if (tablero[f][c].progreso > 1.0f) tablero[f][c].progreso = 1.0f;
            }
        }
    }

    if (juegoTerminado) return;

    // Detectar clics del mouse
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 posicionMouse = GetMousePosition();
        int columna = posicionMouse.x / 200;
        int fila = posicionMouse.y / 200;

        if (fila >= 0 && fila < 3 && columna >= 0 && columna < 3) {
            if (tablero[fila][columna].tipo == 0) {
                tablero[fila][columna].tipo = jugadorActual;
                tablero[fila][columna].progreso = 0.0f;

                // Revisar de inmediato si este tiro provoco una victoria o empate
                ComprobarGanador();

                // Cambiar de turno si el juego sigue activo
                if (!juegoTerminado) {
                    jugadorActual = (jugadorActual == 1) ? 2 : 1;
                }
            }
        }
    }
}

// 5. Logica de Dibujado
void DibujarJuego() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // A. Dibujar las lineas divisorias (Gris)
    DrawLineEx({ 200, 0 }, { 200, 600 }, 5, GRAY);
    DrawLineEx({ 400, 0 }, { 400, 600 }, 5, GRAY);
    DrawLineEx({ 0, 200 }, { 600, 200 }, 5, GRAY);
    DrawLineEx({ 0, 400 }, { 600, 400 }, 5, GRAY);

    // B. Dibujar las fichas con su estado actual de progreso
    for (int f = 0; f < 3; f++) {
        for (int c = 0; c < 3; c++) {
            int centroX = c * 200 + 100;
            int centroY = f * 200 + 100;
            float p = tablero[f][c].progreso;

            if (tablero[f][c].tipo == 1) {
                float tamMax = 40.0f;
                float tamAct = tamMax * p;

                DrawLineEx({ (float)centroX - tamAct, (float)centroY - tamAct },
                    { (float)centroX + tamAct, (float)centroY + tamAct }, 8, BLUE);

                if (p > 0.6f) {
                    float p2 = (p - 0.6f) * 2.5f;
                    if (p2 > 1.0f) p2 = 1.0f;
                    float tamAct2 = tamMax * p2;
                    DrawLineEx({ (float)centroX + tamAct2, (float)centroY - tamAct2 },
                        { (float)centroX - tamAct2, (float)centroY + tamAct2 }, 8, BLUE);
                }
            }
            else if (tablero[f][c].tipo == 2) {
                float gradosMax = 360.0f;
                float gradosAct = gradosMax * p;

                DrawRing({ (float)centroX, (float)centroY }, 41.0f, 49.0f, 0.0f, gradosAct, 36, RED);
            }
        }
    }

    // C. Mostrar el cartel final si el juego ha terminado
    if (juegoTerminado) {
        // Dibujamos un rectangulo negro semi-transparente que cubra un area central
        DrawRectangle(50, 220, 500, 160, Fade(BLACK, 0.85f));

        // Dependiendo del resultado, preparamos el texto
        if (estadoResultado == 1) {
            DrawText("¡GANÓ EL JUGADOR X!", 110, 250, 32, BLUE);
        }
        else if (estadoResultado == 2) {
            DrawText("¡GANÓ EL JUGADOR O!", 110, 250, 32, RED);
        }
        else if (estadoResultado == 3) {
            DrawText("¡EMPATE EN EL TABLERO!", 100, 250, 32, LIGHTGRAY);
        }

        DrawText("Presiona 'R' para volver a jugar", 140, 310, 20, RAYWHITE);
    }

    EndDrawing();
}

// 6. Funcion Principal
int main() {
    // Inicializar la ventana
    InitWindow(600, 600, "3 en Raya Completo - Raylib");
    SetTargetFPS(60);

    InicializarJuego();

    // Bucle Principal
    while (!WindowShouldClose()) {
        ActualizarJuego();
        DibujarJuego();
    }

    CloseWindow();
    return 0;
}