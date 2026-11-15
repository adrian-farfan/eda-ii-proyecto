#ifndef PEDIDO_H
#define PEDIDO_H
// Registro de pedido compartido por las tres estructuras.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

struct Pedido {
    char id[16], dni[12], tipo[12], fecha[11], hora[9], estado[16];
    int prio;
    long long centavos;
    long long clave;   // YYYYMMDDHHMMSS, clave del AVL
    int idxHeap;       // posicion en el heap, -1 si no esta
};

long long calcularClave(const char* f, const char* h) {
    int a, m, d, hh, mm, ss;
    sscanf(f, "%d-%d-%d", &a, &m, &d);
    sscanf(h, "%d:%d:%d", &hh, &mm, &ss);
    return ((long long)a * 10000 + m * 100 + d) * 1000000LL + hh * 10000 + mm * 100 + ss;
}

void mostrar(const Pedido* p) {
    printf("%-10s %-9s %-8s prio=%-2d $%lld.%02lld %s %s %s\n", p->id, p->dni, p->tipo, p->prio,
           p->centavos / 100, p->centavos % 100, p->fecha, p->hora, p->estado);
}
#endif
