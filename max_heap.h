#ifndef MAX_HEAP_H
#define MAX_HEAP_H
#include "pedido.h"

// ---------------- MAX-HEAP (prioridad, luego mas antiguo) ----------------
struct MaxHeap {
    Pedido** a; int cap, n;
    MaxHeap() : cap(1024), n(0) { a = new Pedido*[cap]; }
    bool antes(Pedido* x, Pedido* y) {
        if (x->prio != y->prio) return x->prio > y->prio;
        if (x->clave != y->clave) return x->clave < y->clave;
        return strcmp(x->id, y->id) < 0;
    }
    void poner(int i, Pedido* p) { a[i] = p; p->idxHeap = i; }
    void swapIdx(int i, int j) { Pedido* t = a[i]; poner(i, a[j]); poner(j, t); }
    void subir(int i) { while (i > 0 && antes(a[i], a[(i - 1) / 2])) { swapIdx(i, (i - 1) / 2); i = (i - 1) / 2; } }
    void bajar(int i) {
        for (;;) {
            int l = 2 * i + 1, r = l + 1, m = i;
            if (l < n && antes(a[l], a[m])) m = l;
            if (r < n && antes(a[r], a[m])) m = r;
            if (m == i) return;
            swapIdx(i, m); i = m;
        }
    }
    void insertar(Pedido* p) {
        if (n == cap) {
            Pedido** na = new Pedido*[cap * 2];
            for (int i = 0; i < n; i++) na[i] = a[i];
            delete[] a; a = na; cap *= 2;
        }
        poner(n, p); subir(n); n++;
    }
    void eliminarEn(int i) {            // borrado por indice (cancelar / cambio de estado)
        Pedido* sale = a[i];
        n--;
        if (i != n) { poner(i, a[n]); subir(i); bajar(i); }
        sale->idxHeap = -1;
    }
    Pedido* extraerMax() {
        if (!n) return NULL;
        Pedido* top = a[0];
        eliminarEn(0);
        return top;
    }
};

#endif
