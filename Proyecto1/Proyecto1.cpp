#define WIN32_LEAN_AND_MEAN 
#pragma comment(linker, "/SUBSYSTEM:Console") 

#define _CRT_SECURE_NO_WARNINGS 
#include "raylib.h"
#include <stdio.h> 

// =================================================================
// ESTADOS DEL JUEGO Y DIFICULTADES
// =================================================================
enum EstadoJuego { MENU_PRINCIPAL, JUGANDO, PANTALLA_ESTADISTICAS };
enum Dificultad { FACIL, MEDIO, DIFICIL };

struct Casilla {
    int tipo;
    float progreso;
};

struct Estadisticas {
    int victoriasJ1 = 0;
    int victoriasJ2 = 0;
    int empates = 0;
    int victoriasVsIAFacil = 0;
    int victoriasVsIAMedia = 0;
    int victoriasVsIADificil = 0;
    int derrotasVsIADificil = 0;
};

// =================================================================
// VARIABLES GLOBALES DE CONTROL
// =================================================================
EstadoJuego estadoActual = MENU_PRINCIPAL;
Dificultad dificultadActual = FACIL;
bool contraIA = false;

Casilla tablero[3][3];
int jugadorActual = 1;
bool juegoTerminado = false;
int estadoResultado = 0;
int nivelActual = 1;

int tipoVictoria = 0;
int indiceVictoria = 0;

Estadisticas stats;
bool estadisticaRegistrada = false;

// Control del List Box (Dropdown) de Dificultad
bool listBoxAbierto = false;
const char* textosDificultades[3] = { "FÁCIL", "MEDIA", "IMBATIBLE" };
Rectangle rectListBoxPrincipal = { 200, 490, 200, 45 };

// TEXTURAS GLOBALES PARA EL NIVEL 3
Texture2D texDragon;
Texture2D texFenix;

// Paleta Neón Cyberpunk Unificada
Color fondoOscuro = { 10, 5, 25, 255 };
Color rejillaColor = { 55, 20, 90, 80 };
Color tableroNeon = { 150, 50, 250, 255 };
Color tableroBrillo = { 220, 100, 255, 120 };

Color robotBase = { 0, 140, 255, 255 };
Color robotBrillo = { 130, 220, 255, 255 };

Color cyborgBase = { 255, 0, 50, 255 };
Color cyborgBrillo = { 255, 130, 150, 255 };

Color dragonBrillo = { 160, 255, 190, 255 };
Color fenixBrillo = { 255, 140, 140, 255 };
Color lineaGanadora = { 50, 255, 150, 255 };

// =================================================================
// PERSISTENCIA DE DATOS
// =================================================================
void GuardarEstadisticas() {
    unsigned int dataSize = sizeof(Estadisticas);
    SaveFileData("scores.dat", &stats, dataSize);
}

void CargarEstadisticas() {
    if (FileExists("scores.dat")) {
        int dataSize = 0;
        unsigned char* fileData = LoadFileData("scores.dat", &dataSize);

        if (fileData != NULL && dataSize == sizeof(Estadisticas)) {
            stats = *(Estadisticas*)fileData;
            UnloadFileData(fileData);
        }
    }
    else {
        GuardarEstadisticas();
    }
}

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
    tipoVictoria = 0;
    indiceVictoria = 0;
    estadisticaRegistrada = false;
}

bool ValidarTablero(int& outResultado, int& outTipo, int& outIndice) {
    for (int f = 0; f < 3; f++) {
        if (tablero[f][0].tipo != 0 && tablero[f][0].tipo == tablero[f][1].tipo && tablero[f][0].tipo == tablero[f][2].tipo) {
            outResultado = tablero[f][0].tipo; outTipo = 1; outIndice = f; return true;
        }
    }
    for (int c = 0; c < 3; c++) {
        if (tablero[0][c].tipo != 0 && tablero[0][c].tipo == tablero[1][c].tipo && tablero[0][c].tipo == tablero[2][c].tipo) {
            outResultado = tablero[0][c].tipo; outTipo = 2; outIndice = c; return true;
        }
    }
    if (tablero[0][0].tipo != 0 && tablero[0][0].tipo == tablero[1][1].tipo && tablero[0][0].tipo == tablero[2][2].tipo) {
        outResultado = tablero[0][0].tipo; outTipo = 3; outIndice = 0; return true;
    }
    if (tablero[0][2].tipo != 0 && tablero[0][2].tipo == tablero[1][1].tipo && tablero[0][2].tipo == tablero[2][0].tipo) {
        outResultado = tablero[0][2].tipo; outTipo = 4; outIndice = 0; return true;
    }
    bool lleno = true;
    for (int f = 0; f < 3; f++) {
        for (int c = 0; c < 3; c++) { if (tablero[f][c].tipo == 0) lleno = false; }
    }
    if (lleno) { outResultado = 3; outTipo = 0; outIndice = 0; return true; }
    return false;
}

// =================================================================
// INTELIGENCIA ARTIFICIAL (IA)
// =================================================================
void JugarEnCasilla(int f, int c) {
    tablero[f][c].tipo = 2;
    tablero[f][c].progreso = 0.0f;
    jugadorActual = 1;
}

void EjecutarIAFacil() {
    int vaciasF[9], vaciasC[9];
    int contador = 0;
    for (int f = 0; f < 3; f++) {
        for (int c = 0; c < 3; c++) {
            if (tablero[f][c].tipo == 0) {
                vaciasF[contador] = f;
                vaciasC[contador] = c;
                contador++;
            }
        }
    }
    if (contador > 0) {
        int r = GetRandomValue(0, contador - 1);
        JugarEnCasilla(vaciasF[r], vaciasC[r]);
    }
}

bool IntentarGanarOBloquear(int objetivoTipo) {
    for (int f = 0; f < 3; f++) {
        int cuenta = 0, vacioC = -1;
        for (int c = 0; c < 3; c++) {
            if (tablero[f][c].tipo == objetivoTipo) cuenta++;
            else if (tablero[f][c].tipo == 0) vacioC = c;
        }
        if (cuenta == 2 && vacioC != -1) { JugarEnCasilla(f, vacioC); return true; }
    }
    for (int c = 0; c < 3; c++) {
        int cuenta = 0, vacioF = -1;
        for (int f = 0; f < 3; f++) {
            if (tablero[f][c].tipo == objetivoTipo) cuenta++;
            else if (tablero[f][c].tipo == 0) vacioF = f;
        }
        if (cuenta == 2 && vacioF != -1) { JugarEnCasilla(vacioF, c); return true; }
    }
    int cuentaD1 = 0, vacioD1 = -1;
    for (int i = 0; i < 3; i++) {
        if (tablero[i][i].tipo == objetivoTipo) cuentaD1++;
        else if (tablero[i][i].tipo == 0) vacioD1 = i;
    }
    if (cuentaD1 == 2 && vacioD1 != -1) { JugarEnCasilla(vacioD1, vacioD1); return true; }

    int cuentaD2 = 0, vacioD2 = -1;
    for (int i = 0; i < 3; i++) {
        if (tablero[i][2 - i].tipo == objetivoTipo) cuentaD2++;
        else if (tablero[i][2 - i].tipo == 0) vacioD2 = i;
    }
    if (cuentaD2 == 2 && vacioD2 != -1) { JugarEnCasilla(vacioD2, 2 - vacioD2); return true; }

    return false;
}

void EjecutarIAMedia() {
    if (IntentarGanarOBloquear(2)) return;
    if (IntentarGanarOBloquear(1)) return;
    if (GetRandomValue(1, 10) <= 2) {
        EjecutarIAFacil();
        return;
    }
    EjecutarIAFacil();
}

int Minimax(bool esMaximizador) {
    int res = 0, tipo = 0, ind = 0;
    if (ValidarTablero(res, tipo, ind)) {
        if (res == 2) return 10;
        if (res == 1) return -10;
        if (res == 3) return 0;
    }

    if (esMaximizador) {
        int mejorPuntaje = -1000;
        for (int f = 0; f < 3; f++) {
            for (int c = 0; c < 3; c++) {
                if (tablero[f][c].tipo == 0) {
                    tablero[f][c].tipo = 2;
                    int puntaje = Minimax(false);
                    tablero[f][c].tipo = 0;
                    if (puntaje > mejorPuntaje) mejorPuntaje = puntaje;
                }
            }
        }
        return mejorPuntaje;
    }
    else {
        int mejorPuntaje = 1000;
        for (int f = 0; f < 3; f++) {
            for (int c = 0; c < 3; c++) {
                if (tablero[f][c].tipo == 0) {
                    tablero[f][c].tipo = 1;
                    int puntaje = Minimax(true);
                    tablero[f][c].tipo = 0;
                    if (puntaje < mejorPuntaje) mejorPuntaje = puntaje;
                }
            }
        }
        return mejorPuntaje;
    }
}

void EjecutarIADificil() {
    int mejorPuntaje = -1000;
    int mejorF = -1, mejorC = -1;

    for (int f = 0; f < 3; f++) {
        for (int c = 0; c < 3; c++) {
            if (tablero[f][c].tipo == 0) {
                tablero[f][c].tipo = 2;
                int puntaje = Minimax(false);
                tablero[f][c].tipo = 0;
                if (puntaje > mejorPuntaje) {
                    mejorPuntaje = puntaje;
                    mejorF = f;
                    mejorC = c;
                }
            }
        }
    }
    if (mejorF != -1 && mejorC != -1) {
        JugarEnCasilla(mejorF, mejorC);
    }
}

// =================================================================
// LÓGICA CENTRAL DE ACTUALIZACIÓN
// =================================================================
void ActualizarMenu() {
    Vector2 mousePos = GetMousePosition();
    bool clic = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (CheckCollisionPointRec(mousePos, { 150, 240, 300, 50 }) && clic && !listBoxAbierto) {
        contraIA = true;
        estadoActual = JUGANDO;
        InicializarJuego();
    }
    if (CheckCollisionPointRec(mousePos, { 150, 310, 300, 50 }) && clic && !listBoxAbierto) {
        contraIA = false;
        estadoActual = JUGANDO;
        InicializarJuego();
    }
    if (CheckCollisionPointRec(mousePos, { 150, 380, 300, 50 }) && clic && !listBoxAbierto) {
        estadoActual = PANTALLA_ESTADISTICAS;
    }

    // LÓGICA DEL LIST BOX (DROPDOWN DE DIFICULTAD)
    if (clic) {
        if (CheckCollisionPointRec(mousePos, rectListBoxPrincipal)) {
            listBoxAbierto = !listBoxAbierto;
        }
        else if (listBoxAbierto) {
            for (int i = 0; i < 3; i++) {
                Rectangle rectOpcion = {
                    rectListBoxPrincipal.x,
                    rectListBoxPrincipal.y + rectListBoxPrincipal.height + (i * rectListBoxPrincipal.height),
                    rectListBoxPrincipal.width,
                    rectListBoxPrincipal.height
                };
                if (CheckCollisionPointRec(mousePos, rectOpcion)) {
                    dificultadActual = (Dificultad)i;
                    listBoxAbierto = false;
                    break;
                }
            }
            if (!CheckCollisionPointRec(mousePos, rectListBoxPrincipal)) {
                listBoxAbierto = false;
            }
        }
    }

    if (CheckCollisionPointRec(mousePos, { 200, 590, 200, 45 }) && clic && !listBoxAbierto) {
        CloseWindow();
    }
}

void ActualizarJuego() {
    Vector2 mousePos = GetMousePosition();
    bool clic = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (juegoTerminado) {
        if (!estadisticaRegistrada) {
            if (estadoResultado == 1) {
                stats.victoriasJ1++;
                if (contraIA) {
                    if (dificultadActual == FACIL) stats.victoriasVsIAFacil++;
                    else if (dificultadActual == MEDIO) stats.victoriasVsIAMedia++;
                    else if (dificultadActual == DIFICIL) stats.victoriasVsIADificil++;
                }
            }
            else if (estadoResultado == 2) {
                stats.victoriasJ2++;
                if (contraIA && dificultadActual == DIFICIL) {
                    stats.derrotasVsIADificil++;
                }
            }
            else if (estadoResultado == 3) {
                stats.empates++;
            }

            GuardarEstadisticas();
            estadisticaRegistrada = true;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            if (nivelActual == 1) nivelActual = 2;
            else if (nivelActual == 2) nivelActual = 3;
            else nivelActual = 1;
            InicializarJuego();
        }
        if (IsKeyPressed(KEY_R)) {
            InicializarJuego();
        }
        if (CheckCollisionPointRec(mousePos, { 175, 430, 250, 35 }) && clic) {
            estadoActual = MENU_PRINCIPAL;
        }
        return;
    }

    bool figurasAnimandose = false;
    for (int f = 0; f < 3; f++) {
        for (int c = 0; c < 3; c++) {
            if (tablero[f][c].tipo != 0 && tablero[f][c].progreso < 1.0f) {
                tablero[f][c].progreso += 0.05f;
                figurasAnimandose = true;
                if (tablero[f][c].progreso > 1.0f) tablero[f][c].progreso = 1.0f;
            }
        }
    }

    if (!figurasAnimandose) {
        int tempResultado = 0, tempTipo = 0, tempIndice = 0;
        if (ValidarTablero(tempResultado, tempTipo, tempIndice)) {
            estadoResultado = tempResultado;
            tipoVictoria = tempTipo;
            indiceVictoria = tempIndice;
            juegoTerminado = true;
            return;
        }

        if (jugadorActual == 2 && contraIA) {
            if (dificultadActual == FACIL) { EjecutarIAFacil(); }
            else if (dificultadActual == MEDIO) { EjecutarIAMedia(); }
            else if (dificultadActual == DIFICIL) { EjecutarIADificil(); }
            return;
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !figurasAnimandose) {
        if (jugadorActual == 1 || (jugadorActual == 2 && !contraIA)) {
            Vector2 posicionMouse = GetMousePosition();
            if (posicionMouse.x >= 30 && posicionMouse.x <= 570 && posicionMouse.y >= 135 && posicionMouse.y <= 675) {
                int columna = ((int)posicionMouse.x - 30) / 180;
                int fila = ((int)posicionMouse.y - 135) / 180;

                if (fila >= 0 && fila < 3 && columna >= 0 && columna < 3) {
                    if (tablero[fila][columna].tipo == 0) {
                        tablero[fila][columna].tipo = jugadorActual;
                        tablero[fila][columna].progreso = 0.001f;
                        jugadorActual = (jugadorActual == 1) ? 2 : 1;
                    }
                }
            }
        }
    }
}

void ActualizarEstadisticas() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
        estadoActual = MENU_PRINCIPAL;
    }
}

// =================================================================
// RENDERIZADO
// =================================================================
void DibujarFicha(int tipo, int cx, int cy, float p) {
    float fx = (float)cx;
    float fy = (float)cy;

    if (nivelActual == 1) {
        if (tipo == 1) {
            float tamMax = 38.0f;
            float tamAct = tamMax * p;
            DrawLineEx({ fx - tamAct, fy - tamAct }, { fx + tamAct, fy + tamAct }, 12, robotBase);
            DrawLineEx({ fx - tamAct, fy - tamAct }, { fx + tamAct, fy + tamAct }, 4, robotBrillo);
            if (p > 0.5f) {
                float tamAct2 = tamMax * ((p - 0.5f) * 2.0f);
                DrawLineEx({ fx + tamAct2, fy - tamAct2 }, { fx - tamAct2, fy + tamAct2 }, 12, robotBase);
                DrawLineEx({ fx + tamAct2, fy - tamAct2 }, { fx - tamAct2, fy + tamAct2 }, 4, robotBrillo);
            }
        }
        else if (tipo == 2) {
            DrawRing({ fx, fy }, 35.0f, 47.0f, 0.0f, 360.0f * p, 45, cyborgBase);
            DrawRing({ fx, fy }, 39.0f, 43.0f, 0.0f, 360.0f * p, 45, cyborgBrillo);
        }
    }
    else if (nivelActual == 2) {
        if (tipo == 1) {
            Rectangle cuerpoRobot = { fx - 35.0f * p, fy - 25.0f * p, 70.0f * p, 60.0f * p };
            DrawRectangleLinesEx(cuerpoRobot, 5.0f * p, robotBase);
            DrawRectangleLinesEx(cuerpoRobot, 1.5f * p, robotBrillo);
            DrawLineEx({ fx - 15.0f * p, fy - 25.0f * p }, { fx - 25.0f * p, fy - 45.0f * p }, 4.0f * p, robotBase);
            DrawLineEx({ fx + 15.0f * p, fy - 25.0f * p }, { fx + 25.0f * p, fy - 45.0f * p }, 4.0f * p, robotBase);
            DrawCircle((int)(fx - 25.0f * p), (int)(fy - 45.0f * p), (int)(5.0f * p), robotBrillo);
            DrawCircle((int)(fx + 25.0f * p), (int)(fy - 45.0f * p), (int)(5.0f * p), robotBrillo);
            Rectangle visor = { fx - 22.0f * p, fy - 12.0f * p, 44.0f * p, 16.0f * p };
            DrawRectangleRec(visor, Fade(robotBase, 0.3f));
            DrawCircle((int)(fx - 10.0f * p), (int)(fy - 4.0f * p), (int)(4.0f * p), robotBrillo);
            DrawCircle((int)(fx + 10.0f * p), (int)(fy - 4.0f * p), (int)(4.0f * p), robotBrillo);
            DrawLineEx({ fx - 20.0f * p, fy + 20.0f * p }, { fx - 20.0f * p, fy + 30.0f * p }, 3.0f * p, robotBrillo);
            DrawLineEx({ fx - 20.0f * p, fy + 30.0f * p }, { fx + 20.0f * p, fy + 30.0f * p }, 3.0f * p, robotBrillo);
            DrawLineEx({ fx + 20.0f * p, fy + 30.0f * p }, { fx + 20.0f * p, fy + 20.0f * p }, 3.0f * p, robotBrillo);
        }
        else if (tipo == 2) {
            Vector2 centroFicha = { fx, fy + 5.0f * p };
            DrawCircleSector(centroFicha, 32.0f * p, 180.0f, 360.0f, 16, cyborgBase);
            DrawCircleSectorLines(centroFicha, 32.0f * p, 180.0f, 360.0f, 16, cyborgBrillo);
            DrawCircleSectorLines(centroFicha, 30.0f * p, 180.0f, 360.0f, 16, cyborgBrillo);
            DrawCircle((int)(fx + 12.0f * p), (int)(fy - 8.0f * p), (int)(6.0f * p), BLACK);
            DrawCircle((int)(fx + 12.0f * p), (int)(fy - 8.0f * p), (int)(4.0f * p), cyborgBrillo);
            DrawLineEx({ fx + 12.0f * p, fy - 8.0f * p }, { fx + 32.0f * p, fy - 8.0f * p }, 2.0f * p, cyborgBrillo);
            Rectangle mandibula = { fx - 25.0f * p, fy + 2.0f * p, 18.0f * p, 18.0f * p };
            DrawRectangleLinesEx(mandibula, 2.5f * p, cyborgBase);
            DrawLineEx({ fx - 25.0f * p, fy + 10.0f * p }, { fx - 7.0f * p, fy + 10.0f * p }, 1.5f * p, cyborgBrillo);
            DrawLineEx({ fx - 35.0f * p, fy + 35.0f * p }, { fx + 5.0f * p, fy + 35.0f * p }, 8.0f * p, cyborgBase);
            DrawLineEx({ fx - 35.0f * p, fy + 35.0f * p }, { fx + 5.0f * p, fy + 35.0f * p }, 2.5f * p, cyborgBrillo);
            Rectangle conector = { fx + 5.0f * p, fy + 28.0f * p, 12.0f * p, 14.0f * p };
            DrawRectangleLinesEx(conector, 2.0f * p, cyborgBrillo);
        }
    }
    else if (nivelActual == 3) {
        float tamFinal = 140.0f * p;

        if (tipo == 1) {
            if (texDragon.id > 0) {
                Rectangle origen = { 0.0f, 0.0f, (float)texDragon.width, (float)texDragon.height };
                Rectangle destino = { fx - (tamFinal / 2.0f), fy - (tamFinal / 2.0f), tamFinal, tamFinal };
                DrawTexturePro(texDragon, origen, destino, { 0.0f, 0.0f }, 0.0f, WHITE);
            }
            else {
                DrawTriangle({ fx, fy - 40.0f * p }, { fx - 40.0f * p, fy + 30.0f * p }, { fx + 40.0f * p, fy + 30.0f * p }, dragonBrillo);
            }
        }
        else if (tipo == 2) {
            if (texFenix.id > 0) {
                Rectangle origen = { 0.0f, 0.0f, (float)texFenix.width, (float)texFenix.height };
                Rectangle destino = { fx - (tamFinal / 2.0f), fy - (tamFinal / 2.0f), tamFinal, tamFinal };
                DrawTexturePro(texFenix, origen, destino, { 0.0f, 0.0f }, 0.0f, WHITE);
            }
            else {
                DrawCircle((int)fx, (int)fy, (int)(35.0f * p), fenixBrillo);
            }
        }
    }
}

void DibujarLineaGanadora() {
    if (tipoVictoria == 1) {
        float y = (float)indiceVictoria * 180.0f + 225.0f;
        DrawLineEx({ 40, y }, { 560, y }, 14, lineaGanadora);
    }
    else if (tipoVictoria == 2) {
        float x = (float)indiceVictoria * 180.0f + 120.0f;
        DrawLineEx({ x, 145 }, { x, 655 }, 14, lineaGanadora);
    }
    else if (tipoVictoria == 3) DrawLineEx({ 50, 155 }, { 550, 655 }, 14, lineaGanadora);
    else if (tipoVictoria == 4) DrawLineEx({ 550, 155 }, { 50, 655 }, 14, lineaGanadora);
}

void DibujarPantallaMenu() {
    BeginDrawing();
    ClearBackground(fondoOscuro);

    for (int i = -10; i <= 20; i++) DrawLineEx({ 300 + (float)i * 15, 400 }, { 300 + (float)i * 60, 700 }, 2, rejillaColor);

    DrawText("CYBERPUNK", 160, 50, 48, tableroBrillo);
    DrawText("TIC - TAC - TOE", 175, 110, 32, tableroNeon);

    Vector2 mousePos = GetMousePosition();

    Rectangle btn1P = { 150, 240, 300, 50 };
    bool hover1P = CheckCollisionPointRec(mousePos, btn1P);
    DrawRectangleRounded(btn1P, 0.2f, 16, hover1P ? Fade(robotBase, 0.25f) : Fade(robotBase, 0.08f));
    DrawRectangleRoundedLinesEx(btn1P, 0.2f, 16, hover1P ? 3.0f : 1.5f, robotBase);
    DrawText("1 JUGADOR (VS COMPU)", 188, 255, 18, robotBrillo);

    Rectangle btn2P = { 150, 310, 300, 50 };
    bool hover2P = CheckCollisionPointRec(mousePos, btn2P);
    DrawRectangleRounded(btn2P, 0.2f, 16, hover2P ? Fade(cyborgBase, 0.25f) : Fade(cyborgBase, 0.08f));
    DrawRectangleRoundedLinesEx(btn2P, 0.2f, 16, hover2P ? 3.0f : 1.5f, cyborgBase);
    DrawText("2 JUGADORES (LOCAL)", 195, 325, 18, cyborgBrillo);

    Rectangle btnStats = { 150, 380, 300, 50 };
    bool hoverStats = CheckCollisionPointRec(mousePos, btnStats);
    DrawRectangleRounded(btnStats, 0.2f, 16, hoverStats ? Fade(PURPLE, 0.25f) : Fade(PURPLE, 0.08f));
    DrawRectangleRoundedLinesEx(btnStats, 0.2f, 16, hoverStats ? 3.0f : 1.5f, PURPLE);
    DrawText("ESTADÍSTICAS DEL SISTEMA", 172, 395, 16, tableroBrillo);

    DrawLineEx({ 100, 450 }, { 500, 450 }, 1.5f, rejillaColor);
    DrawText("CONFIGURACIÓN DE IA / DIFICULTAD", 160, 460, 15, PURPLE);

    // BOTÓN DE SALIR (Se dibuja antes para que la List Box abierta flote por encima de él)
    Rectangle btnSalir = { 200, 590, 200, 45 };
    bool hoverSalir = CheckCollisionPointRec(mousePos, btnSalir);
    DrawRectangleRounded(btnSalir, 0.2f, 16, hoverSalir ? Fade(RED, 0.2f) : BLACK);
    DrawRectangleRoundedLinesEx(btnSalir, 0.2f, 16, 1.0f, hoverSalir ? RED : GRAY);
    DrawText("SALIR", 275, 603, 16, hoverSalir ? RED : GRAY);

    // =================================================================
    // GESTIÓN Y RENDERIZADO DEL LIST BOX (DROPDOWN)
    // =================================================================
    if (listBoxAbierto) {
        for (int i = 0; i < 3; i++) {
            Rectangle rectOpcion = {
                rectListBoxPrincipal.x,
                rectListBoxPrincipal.y + rectListBoxPrincipal.height + (i * rectListBoxPrincipal.height),
                rectListBoxPrincipal.width,
                rectListBoxPrincipal.height
            };

            bool hoverOpcion = CheckCollisionPointRec(mousePos, rectOpcion);
            DrawRectangleRec(rectOpcion, hoverOpcion ? Fade(PURPLE, 0.3f) : fondoOscuro);
            DrawRectangleLinesEx(rectOpcion, 1.0f, tableroNeon);

            Color colorTexto = (dificultadActual == i) ? GREEN : GRAY;
            if (hoverOpcion) colorTexto = RAYWHITE;

            DrawText(textosDificultades[i], (int)rectOpcion.x + 20, (int)rectOpcion.y + 15, 16, colorTexto);
        }
    }

    bool hoverPrincipal = CheckCollisionPointRec(mousePos, rectListBoxPrincipal);
    DrawRectangleRounded(rectListBoxPrincipal, 0.2f, 16, hoverPrincipal ? Fade(tableroNeon, 0.2f) : BLACK);
    DrawRectangleRoundedLinesEx(rectListBoxPrincipal, 0.2f, 16, 2.0f, listBoxAbierto ? GREEN : tableroNeon);
    DrawText(textosDificultades[dificultadActual], (int)rectListBoxPrincipal.x + 20, (int)rectListBoxPrincipal.y + 15, 16, RAYWHITE);
    DrawText(listBoxAbierto ? "^" : "v", (int)rectListBoxPrincipal.x + (int)rectListBoxPrincipal.width - 25, (int)rectListBoxPrincipal.y + 12, 16, PURPLE);
    // =================================================================

    EndDrawing();
}

void DibujarPantallaJuego() {
    BeginDrawing();
    ClearBackground(fondoOscuro);

    for (int i = -10; i <= 20; i++) DrawLineEx({ 300 + (float)i * 15, 400 }, { 300 + (float)i * 60, 700 }, 2, rejillaColor);
    float yHorizonte = 400.0f;
    for (int i = 0; i < 15; i++) {
        float yLinea = yHorizonte + (i * i * 1.4f);
        if (yLinea <= 700) DrawLineEx({ 0, yLinea }, { 600, yLinea }, 2, rejillaColor);
    }

    if (nivelActual == 1) DrawText("NIVEL 1: CLÁSICO NEÓN", 165, 8, 20, tableroNeon);
    else if (nivelActual == 2) DrawText("NIVEL 2: GUERRA TECNOLÓGICA", 145, 8, 20, tableroNeon);
    else if (nivelActual == 3) DrawText("NIVEL 3: CRIATURAS MÍTICAS", 155, 8, 20, tableroNeon);

    Rectangle cajaX = { 60.0f, 35.0f, 180.0f, 70.0f };
    Color colorP1 = robotBase;
    Color brilloP1 = robotBrillo;
    if (jugadorActual == 1 && !juegoTerminado) {
        DrawRectangleRounded(cajaX, 0.2f, 16, Fade(colorP1, 0.2f));
        DrawRectangleRoundedLinesEx(cajaX, 0.2f, 16, 3.0f, colorP1);
        if (nivelActual == 1) DrawText("JUGADOR X", 85, 52, 18, brilloP1);
        else if (nivelActual == 2) DrawText("ROBOT AZUL", 85, 52, 18, brilloP1);
        else DrawText("DRAGÓN VERDE", 85, 52, 18, dragonBrillo);
        DrawText("SU TURNO", 115, 76, 11, brilloP1);
    }
    else {
        DrawRectangleRoundedLinesEx(cajaX, 0.2f, 16, 2.0f, Fade(colorP1, 0.4f));
        if (nivelActual == 1) DrawText("JUGADOR X", 85, 53, 18, Fade(brilloP1, 0.5f));
        else if (nivelActual == 2) DrawText("ROBOT AZUL", 85, 53, 18, Fade(brilloP1, 0.5f));
        else DrawText("DRAGÓN VERDE", 85, 53, 18, Fade(dragonBrillo, 0.5f));
    }

    DrawText("VS", 285, 55, 24, PURPLE);

    Rectangle cajaO = { 360.0f, 35.0f, 180.0f, 70.0f };
    Color colorP2 = cyborgBase;
    Color brilloP2 = cyborgBrillo;
    if (jugadorActual == 2 && !juegoTerminado) {
        DrawRectangleRounded(cajaO, 0.2f, 16, Fade(colorP2, 0.2f));
        DrawRectangleRoundedLinesEx(cajaO, 0.2f, 16, 3.0f, colorP2);
        if (nivelActual == 1) DrawText("JUGADOR O", 392, 52, 18, cyborgBrillo);
        else if (nivelActual == 2) DrawText("CYBORG RED", 392, 52, 18, cyborgBrillo);
        else DrawText("FÉNIX ROJO", 398, 52, 18, fenixBrillo);
        DrawText(contraIA ? "COMPUTADORA" : "SU TURNO", 400, 76, 11, brilloP2);
    }
    else {
        DrawRectangleRoundedLinesEx(cajaO, 0.2f, 16, 2.0f, Fade(colorP2, 0.4f));
        if (nivelActual == 1) DrawText("JUGADOR O", 392, 53, 18, Fade(cyborgBrillo, 0.5f));
        else if (nivelActual == 2) DrawText("CYBORG RED", 392, 53, 18, Fade(cyborgBrillo, 0.5f));
        else DrawText("FÉNIX ROJO", 398, 53, 18, Fade(fenixBrillo, 0.5f));
    }

    Rectangle marcoTablero = { 30.0f, 135.0f, 540.0f, 540.0f };
    DrawRectangleRoundedLinesEx(marcoTablero, 0.06f, 32, 10.0f, tableroBrillo);
    DrawRectangleRoundedLinesEx(marcoTablero, 0.06f, 32, 4.0f, tableroNeon);

    DrawLineEx({ 210, 135 }, { 210, 675 }, 4, tableroNeon);
    DrawLineEx({ 390, 135 }, { 390, 675 }, 4, tableroNeon);
    DrawLineEx({ 30, 315 }, { 570, 315 }, 4, tableroNeon);
    DrawLineEx({ 30, 495 }, { 570, 495 }, 4, tableroNeon);

    for (int f = 0; f < 3; f++) {
        for (int c = 0; c < 3; c++) {
            int centroX = c * 180 + 120;
            int centroY = f * 180 + 225;
            if (tablero[f][c].tipo != 0) {
                DibujarFicha(tablero[f][c].tipo, centroX, centroY, tablero[f][c].progreso);
            }
        }
    }

    if (juegoTerminado && estadoResultado != 3) {
        DibujarLineaGanadora();
    }

    if (juegoTerminado) {
        Rectangle menuFinal = { 50.0f, 260.0f, 500.0f, 240.0f };
        DrawRectangleRec(menuFinal, Fade(BLACK, 0.95f));
        DrawRectangleRoundedLinesEx(menuFinal, 0.08f, 16, 4.0f, tableroBrillo);

        if (estadoResultado == 1) {
            if (nivelActual == 1) DrawText("¡GANÓ EL JUGADOR X!", 110, 280, 24, brilloP1);
            else if (nivelActual == 2) DrawText("¡GANÓ EL ROBOT AZUL!", 110, 280, 24, brilloP1);
            else DrawText("¡GANÓ EL DRAGÓN VERDE!", 100, 280, 24, dragonBrillo);
        }
        else if (estadoResultado == 2) {
            if (nivelActual == 1) DrawText("¡GANÓ EL JUGADOR O!", 120, 280, 24, cyborgBrillo);
            else if (nivelActual == 2) DrawText("¡GANÓ EL CYBORG ROJO!", 120, 280, 24, cyborgBrillo);
            else DrawText("¡GANÓ EL FÉNIX ROJO!", 125, 280, 24, fenixBrillo);
        }
        else if (estadoResultado == 3) {
            DrawText("¡EMPATE EN EL CUADRO!", 140, 280, 24, RAYWHITE);
        }

        DrawText("Presiona [ENTER] para ir al SIGUIENTE NIVEL", 95, 335, 16, GREEN);
        DrawText("Presiona [R] para reiniciar este nivel", 145, 375, 15, GRAY);

        Rectangle btnVolverMenu = { 175, 430, 250, 35 };
        Vector2 mPos = GetMousePosition();
        bool hMenu = CheckCollisionPointRec(mPos, btnVolverMenu);
        DrawRectangleRounded(btnVolverMenu, 0.2f, 16, hMenu ? Fade(PURPLE, 0.3f) : Fade(PURPLE, 0.1f));
        DrawRectangleRoundedLinesEx(btnVolverMenu, 0.2f, 16, 1.5f, PURPLE);
        DrawText("VOLVER AL MENÚ", 230, 440, 15, tableroBrillo);
    }
    EndDrawing();
}

void DibujarPantallaEstadisticas() {
    BeginDrawing();
    ClearBackground(fondoOscuro);

    DrawText("MÓDULO DE ESTADÍSTICAS", 110, 50, 32, tableroBrillo);
    DrawLineEx({ 50, 100 }, { 550, 100 }, 2, tableroNeon);

    DrawText(TextFormat("Victorias Totales Jugador 1: %d", stats.victoriasJ1), 80, 150, 20, robotBrillo);
    DrawText(TextFormat("Victorias Totales Jugador 2: %d", stats.victoriasJ2), 80, 190, 20, cyborgBrillo);
    DrawText(TextFormat("Empates Totales en Red:      %d", stats.empates), 80, 230, 20, GRAY);

    DrawLineEx({ 80, 290 }, { 520, 290 }, 1, rejillaColor);
    DrawText("RENDIMIENTO CONTRA INTELIGENCIA ARTIFICIAL:", 80, 320, 16, PURPLE);

    DrawText(TextFormat("  Victorias vs IA Fácil:    %d", stats.victoriasVsIAFacil), 80, 360, 18, GREEN);
    DrawText(TextFormat("  Victorias vs IA Media:    %d", stats.victoriasVsIAMedia), 80, 400, 18, ORANGE);
    DrawText(TextFormat("  Victorias vs IA Imbatible: %d", stats.victoriasVsIADificil), 80, 440, 18, RED);
    DrawText(TextFormat("  Derrotas vs IA Imbatible:  %d", stats.derrotasVsIADificil), 80, 480, 18, RED);

    DrawText("Presiona cualquier tecla o haz clic para regresar", 110, 600, 15, GRAY);
    EndDrawing();
}

// =================================================================
// PUNTO DE ENTRADA PRINCIPAL
// =================================================================
int main() {
    const int anchoPantalla = 600;
    const int altoPantalla = 710;

    InitWindow(anchoPantalla, altoPantalla, "Cyberpunk Tic-Tac-Toe: Advanced Edition");
    SetTargetFPS(60);

    texDragon = LoadTexture("assets/dragon.png");
    if (texDragon.id == 0) texDragon = LoadTexture("assets/dragon.PNG");

    texFenix = LoadTexture("assets/fenix.png");
    if (texFenix.id == 0) texFenix = LoadTexture("assets/fenix.PNG");

    CargarEstadisticas();
    InicializarJuego();

    while (!WindowShouldClose()) {
        switch (estadoActual) {
        case MENU_PRINCIPAL:
            ActualizarMenu();
            DibujarPantallaMenu();
            break;
        case JUGANDO:
            ActualizarJuego();
            DibujarPantallaJuego();
            break;
        case PANTALLA_ESTADISTICAS:
            ActualizarEstadisticas();
            DibujarPantallaEstadisticas();
            break;
        }
    }

    UnloadTexture(texDragon);
    UnloadTexture(texFenix);

    CloseWindow();
    return 0;
}