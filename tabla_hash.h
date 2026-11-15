#ifndef TABLA_HASH_H
#define TABLA_HASH_H
#include "pedido.h"

// ---------------- TABLA HASH (djb2 + encadenamiento) ----------------
struct NodoHash { Pedido* p; NodoHash* sig; };

struct TablaHash {
    NodoHash** b; int cap, n;
    TablaHash() : cap(1009), n(0) { b = new NodoHash*[cap](); }
    unsigned long djb2(const char* s) const {
        unsigned long h = 5381;
        while (*s) h = h * 33 + (unsigned char)*s++;
        return h;
    }
    Pedido* buscar(const char* id) const {
        for (NodoHash* x = b[djb2(id) % cap]; x; x = x->sig)
            if (strcmp(x->p->id, id) == 0) return x->p;
        return NULL;
    }
    void rehash() {
        int nc = cap * 2 + 1;
        NodoHash** nb = new NodoHash*[nc]();
        for (int i = 0; i < cap; i++)
            for (NodoHash* x = b[i]; x;) {
                NodoHash* sig = x->sig;
                int j = djb2(x->p->id) % nc;
                x->sig = nb[j]; nb[j] = x; x = sig;
            }
        delete[] b; b = nb; cap = nc;
    }
    bool insertar(Pedido* p) {
        if (buscar(p->id)) return false;
        if (n + 1 > cap * 3 / 4) rehash();
        int i = djb2(p->id) % cap;
        b[i] = new NodoHash{p, b[i]};
        n++;
        return true;
    }
    void eliminar(const char* id) {
        int i = djb2(id) % cap;
        NodoHash* ant = NULL;
        for (NodoHash* x = b[i]; x; ant = x, x = x->sig)
            if (strcmp(x->p->id, id) == 0) {
                if (ant) ant->sig = x->sig; else b[i] = x->sig;
                delete x; n--; return;
            }
    }
};

#endif
