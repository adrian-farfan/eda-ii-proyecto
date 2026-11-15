#ifndef ARBOL_AVL_H
#define ARBOL_AVL_H
#include "pedido.h"

// ---------------- ARBOL AVL (fecha+hora, desempate por id) ----------------
struct NodoAVL { Pedido* p; NodoAVL *izq, *der; int h; };

int alt(NodoAVL* n) { return n ? n->h : 0; }
void act(NodoAVL* n) { n->h = 1 + (alt(n->izq) > alt(n->der) ? alt(n->izq) : alt(n->der)); }
int cmp(const Pedido* a, const Pedido* b) {
    if (a->clave != b->clave) return a->clave < b->clave ? -1 : 1;
    return strcmp(a->id, b->id);
}
NodoAVL* rotDer(NodoAVL* y) { NodoAVL* x = y->izq; y->izq = x->der; x->der = y; act(y); act(x); return x; }
NodoAVL* rotIzq(NodoAVL* x) { NodoAVL* y = x->der; x->der = y->izq; y->izq = x; act(x); act(y); return y; }
NodoAVL* balancear(NodoAVL* n) {
    act(n);
    int fb = alt(n->izq) - alt(n->der);
    if (fb > 1) { if (alt(n->izq->izq) < alt(n->izq->der)) n->izq = rotIzq(n->izq); return rotDer(n); }
    if (fb < -1) { if (alt(n->der->der) < alt(n->der->izq)) n->der = rotDer(n->der); return rotIzq(n); }
    return n;
}
NodoAVL* avlInsertar(NodoAVL* n, Pedido* p) {
    if (!n) return new NodoAVL{p, NULL, NULL, 1};
    if (cmp(p, n->p) < 0) n->izq = avlInsertar(n->izq, p);
    else n->der = avlInsertar(n->der, p);
    return balancear(n);
}
NodoAVL* avlEliminar(NodoAVL* n, const Pedido* p) {
    if (!n) return NULL;
    int c = cmp(p, n->p);
    if (c < 0) n->izq = avlEliminar(n->izq, p);
    else if (c > 0) n->der = avlEliminar(n->der, p);
    else {
        if (!n->izq || !n->der) { NodoAVL* hijo = n->izq ? n->izq : n->der; delete n; return hijo; }
        NodoAVL* s = n->der;
        while (s->izq) s = s->izq;
        n->p = s->p;
        n->der = avlEliminar(n->der, s->p);
    }
    return balancear(n);
}
// In-order, se detiene al llegar a max
void inorden(NodoAVL* n, int& cont, int max) {
    if (!n || cont >= max) return;
    inorden(n->izq, cont, max);
    if (cont < max) { cont++; printf("%3d. ", cont); mostrar(n->p); }
    inorden(n->der, cont, max);
}
// Rango con poda: solo baja por ramas que pueden tener claves en [d, h]
void rango(NodoAVL* n, long long d, long long h, int& cant, long long& suma, int& visitados) {
    if (!n) return;
    visitados++;
    long long k = n->p->clave;
    if (k >= d) rango(n->izq, d, h, cant, suma, visitados);
    if (k >= d && k <= h) { cant++; suma += n->p->centavos; }
    if (k <= h) rango(n->der, d, h, cant, suma, visitados);
}

#endif
