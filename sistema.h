#ifndef SISTEMA_H
#define SISTEMA_H
#include "tabla_hash.h"
#include "arbol_avl.h"
#include "max_heap.h"

// ---------------- SISTEMA ----------------
TablaHash hash;
NodoAVL* raiz = NULL;
MaxHeap heap;
long long montoTotal = 0;   // monto acumulado (centavos), se mantiene en O(1)

bool registrar(Pedido* p) {
    if (!hash.insertar(p)) return false;
    raiz = avlInsertar(raiz, p);
    p->idxHeap = -1;
    montoTotal += p->centavos;
    if (strcmp(p->estado, "Pendiente") == 0) heap.insertar(p);
    return true;
}

void cambiarEstado(Pedido* p, const char* nuevo) {
    strcpy(p->estado, nuevo);
    bool pend = strcmp(nuevo, "Pendiente") == 0;
    if (pend && p->idxHeap < 0) heap.insertar(p);
    else if (!pend && p->idxHeap >= 0) heap.eliminarEn(p->idxHeap);
}

bool cancelar(const char* id) {
    Pedido* p = hash.buscar(id);
    if (!p) return false;
    if (p->idxHeap >= 0) heap.eliminarEn(p->idxHeap);
    raiz = avlEliminar(raiz, p);
    montoTotal -= p->centavos;
    hash.eliminar(id);
    delete p;
    return true;
}

// Linea CSV: id,dni,tipo,prioridad,monto,fecha,hora,estado
bool parsear(char* linea, Pedido* p) {
    char* c[8]; int k = 0;
    c[k++] = linea;
    for (char* s = linea; *s && k < 9; s++)
        if (*s == ',') { *s = 0; if (k < 8) c[k] = s + 1; k++; }
    if (k != 8) return false;
    strncpy(p->id, c[0], 15); p->id[15] = 0;
    strncpy(p->dni, c[1], 11); p->dni[11] = 0;
    strncpy(p->tipo, c[2], 11); p->tipo[11] = 0;
    p->prio = atoi(c[3]);
    p->centavos = (long long)(atof(c[4]) * 100 + 0.5);
    strncpy(p->fecha, c[5], 10); p->fecha[10] = 0;
    strncpy(p->hora, c[6], 8); p->hora[8] = 0;
    strncpy(p->estado, c[7], 15); p->estado[15] = 0;
    for (char* s = p->estado; *s; s++) if (*s == '\r' || *s == '\n') *s = 0;
    if (p->prio < 1 || p->prio > 10 || strlen(p->fecha) != 10 || strlen(p->hora) != 8) return false;
    p->clave = calcularClave(p->fecha, p->hora);
    p->idxHeap = -1;
    return true;
}

void cargar(const char* ruta) {
    FILE* f = fopen(ruta, "r");
    if (!f) { printf("No se pudo abrir %s\n", ruta); return; }
    clock_t t0 = clock();
    char linea[256]; int ok = 0, dup = 0, mal = 0, primera = 1;
    while (fgets(linea, sizeof linea, f)) {
        if (primera) { primera = 0; if (strncmp(linea, "idPedido", 8) == 0) continue; }
        Pedido* p = new Pedido;
        if (!parsear(linea, p)) { delete p; mal++; continue; }
        if (registrar(p)) ok++; else { delete p; dup++; }
    }
    fclose(f);
    printf("Cargados: %d | duplicados: %d | invalidos: %d | %.2f ms\n", ok, dup, mal,
           (clock() - t0) * 1000.0 / CLOCKS_PER_SEC);
}

void estadisticas() {
    printf("Pedidos: %d | Pendientes: %d | Monto total acumulado: $%lld.%02lld\n", hash.n, heap.n,
           montoTotal / 100, montoTotal % 100);
}

void generar(const char* ruta, int n) {
    FILE* f = fopen(ruta, "w");
    if (!f) return;
    const char* tipos[] = {"Normal", "Express", "SameDay"};
    srand(1);
    fprintf(f, "idPedido,dniCliente,tipoEnvio,nivelPrioridad,montoDolares,fecha,hora,estado\n");
    for (int i = 1; i <= n; i++) {
        int t = rand() % 3;
        fprintf(f, "PD-%06d,%08d,%s,%d,%d.%02d,2026-%02d-%02d,%02d:%02d:%02d,Pendiente\n", i,
                10000000 + rand() % 90000000, tipos[t], 1 + rand() % 10, 5 + rand() % 2000, rand() % 100,
                1 + rand() % 6, 1 + rand() % 28, rand() % 24, rand() % 60, rand() % 60);
    }
    fclose(f);
    printf("Archivo %s generado con %d pedidos.\n", ruta, n);
}

#endif
