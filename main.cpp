#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

struct Pedido {
    char id[16], dni[12], tipo[12], fecha[11], hora[9], estado[16];
    int prio;
    long long centavos;
    long long clave;
    int idxHeap;
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

enum Color { ROJO, NEGRO };

struct NodoRB {
    Pedido* p;
    Color color;
    NodoRB *izq, *der, *padre;
    NodoRB(Pedido* d) : p(d), color(ROJO), izq(nullptr), der(nullptr), padre(nullptr) {}
};

int cmp(const Pedido* a, const Pedido* b) {
    if (a->clave != b->clave) return a->clave < b->clave ? -1 : 1;
    return strcmp(a->id, b->id);
}

struct ArbolRedBlack {
    NodoRB* raiz;
    NodoRB* TNULL;
    ArbolRedBlack() { TNULL = new NodoRB(NULL); TNULL->color = NEGRO; raiz = TNULL; }
    ~ArbolRedBlack() { destruirArbol(raiz); delete TNULL; }
    NodoRB* minimo(NodoRB* n) {
        while (n->izq != TNULL) n = n->izq;
        return n;
    }
    void rotacionIzquierda(NodoRB* x) {
        NodoRB* y = x->der;
        x->der = y->izq;
        if (y->izq != TNULL) y->izq->padre = x;
        y->padre = x->padre;
        if (x->padre == nullptr) raiz = y;
        else if (x == x->padre->izq) x->padre->izq = y;
        else x->padre->der = y;
        y->izq = x;
        x->padre = y;
    }
    void rotacionDerecha(NodoRB* x) {
        NodoRB* y = x->izq;
        x->izq = y->der;
        if (y->der != TNULL) y->der->padre = x;
        y->padre = x->padre;
        if (x->padre == nullptr) raiz = y;
        else if (x == x->padre->der) x->padre->der = y;
        else x->padre->izq = y;
        y->der = x;
        x->padre = y;
    }
    void transplantar(NodoRB* u, NodoRB* v) {
        if (u->padre == nullptr) raiz = v;
        else if (u == u->padre->izq) u->padre->izq = v;
        else u->padre->der = v;
        v->padre = u->padre;
    }
    NodoRB* insertarBST(Pedido* p) {
        NodoRB* nodo = new NodoRB(p);
        nodo->izq = TNULL;
        nodo->der = TNULL;
        NodoRB* y = nullptr;
        NodoRB* x = raiz;
        while (x != TNULL) {
            y = x;
            if (cmp(nodo->p, x->p) < 0) x = x->izq;
            else x = x->der;
        }
        nodo->padre = y;
        if (y == nullptr) raiz = nodo;
        else if (cmp(nodo->p, y->p) < 0) y->izq = nodo;
        else y->der = nodo;
        return nodo;
    }
    void repararInsercion(NodoRB* k) {
        NodoRB* u;
        while (k->padre->color == ROJO) {
            if (k->padre == k->padre->padre->der) {
                u = k->padre->padre->izq;
                if (u->color == ROJO) {
                    u->color = NEGRO;
                    k->padre->color = NEGRO;
                    k->padre->padre->color = ROJO;
                    k = k->padre->padre;
                } else {
                    if (k == k->padre->izq) {
                        k = k->padre;
                        rotacionDerecha(k);
                    }
                    k->padre->color = NEGRO;
                    k->padre->padre->color = ROJO;
                    rotacionIzquierda(k->padre->padre);
                }
            } else {
                u = k->padre->padre->der;
                if (u->color == ROJO) {
                    u->color = NEGRO;
                    k->padre->color = NEGRO;
                    k->padre->padre->color = ROJO;
                    k = k->padre->padre;
                } else {
                    if (k == k->padre->der) {
                        k = k->padre;
                        rotacionIzquierda(k);
                    }
                    k->padre->color = NEGRO;
                    k->padre->padre->color = ROJO;
                    rotacionDerecha(k->padre->padre);
                }
            }
            if (k == raiz) break;
        }
        raiz->color = NEGRO;
    }
    void insertar(Pedido* p) {
        NodoRB* nuevo = insertarBST(p);
        if (nuevo->padre == nullptr) { nuevo->color = NEGRO; return; }
        repararInsercion(nuevo);
    }
    void repararEliminacion(NodoRB* x) {
        NodoRB* s;
        while (x != raiz && x->color == NEGRO) {
            if (x == x->padre->izq) {
                s = x->padre->der;
                if (s->color == ROJO) {
                    s->color = NEGRO;
                    x->padre->color = ROJO;
                    rotacionIzquierda(x->padre);
                    s = x->padre->der;
                }
                if (s->izq->color == NEGRO && s->der->color == NEGRO) {
                    s->color = ROJO;
                    x = x->padre;
                } else {
                    if (s->der->color == NEGRO) {
                        s->izq->color = NEGRO;
                        s->color = ROJO;
                        rotacionDerecha(s);
                        s = x->padre->der;
                    }
                    s->color = x->padre->color;
                    x->padre->color = NEGRO;
                    s->der->color = NEGRO;
                    rotacionIzquierda(x->padre);
                    x = raiz;
                }
            } else {
                s = x->padre->izq;
                if (s->color == ROJO) {
                    s->color = NEGRO;
                    x->padre->color = ROJO;
                    rotacionDerecha(x->padre);
                    s = x->padre->izq;
                }
                if (s->izq->color == NEGRO && s->der->color == NEGRO) {
                    s->color = ROJO;
                    x = x->padre;
                } else {
                    if (s->izq->color == NEGRO) {
                        s->der->color = NEGRO;
                        s->color = ROJO;
                        rotacionIzquierda(s);
                        s = x->padre->izq;
                    }
                    s->color = x->padre->color;
                    x->padre->color = NEGRO;
                    s->izq->color = NEGRO;
                    rotacionDerecha(x->padre);
                    x = raiz;
                }
            }
        }
        x->color = NEGRO;
    }
    void eliminarNodoHelper(NodoRB* nodo, const Pedido* p) {
        NodoRB* z = TNULL;
        NodoRB *x, *y;
        while (nodo != TNULL) {
            if (nodo->p == p) z = nodo;
            if (cmp(nodo->p, p) <= 0) nodo = nodo->der;
            else nodo = nodo->izq;
        }
        if (z == TNULL) return;
        y = z;
        Color y_color_original = y->color;
        if (z->izq == TNULL) {
            x = z->der;
            transplantar(z, z->der);
        } else if (z->der == TNULL) {
            x = z->izq;
            transplantar(z, z->izq);
        } else {
            y = minimo(z->der);
            y_color_original = y->color;
            x = y->der;
            if (y->padre == z) x->padre = y;
            else {
                transplantar(y, y->der);
                y->der = z->der;
                y->der->padre = y;
            }
            transplantar(z, y);
            y->izq = z->izq;
            y->izq->padre = y;
            y->color = z->color;
        }
        delete z;
        if (y_color_original == NEGRO) repararEliminacion(x);
    }
    void eliminar(const Pedido* p) { eliminarNodoHelper(raiz, p); }
    void destruirArbol(NodoRB* n) {
        if (n != TNULL) {
            destruirArbol(n->izq);
            destruirArbol(n->der);
            delete n->p;
            delete n;
        }
    }
    void inorden(NodoRB* n, int& cont, int max) {
        if (n == TNULL || cont >= max) return;
        inorden(n->izq, cont, max);
        if (cont < max) { cont++; printf("%3d. ", cont); mostrar(n->p); }
        inorden(n->der, cont, max);
    }
    void rango(NodoRB* n, long long d, long long h, int& cant, long long& suma, int& visitados) {
        if (n == TNULL) return;
        visitados++;
        long long k = n->p->clave;
        if (k >= d) rango(n->izq, d, h, cant, suma, visitados);
        if (k >= d && k <= h) { cant++; suma += n->p->centavos; }
        if (k <= h) rango(n->der, d, h, cant, suma, visitados);
    }
};

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
    void eliminarEn(int i) {
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

TablaHash hash;
ArbolRedBlack arbol;
MaxHeap heap;
long long montoTotal = 0;

bool registrar(Pedido* p) {
    if (!hash.insertar(p)) return false;
    arbol.insertar(p);
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
    arbol.eliminar(p);
    montoTotal -= p->centavos;
    hash.eliminar(id);
    delete p;
    return true;
}

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

int main() {
    int op;
    char buf[64], buf2[64];
    for (;;) {
        printf("\n=== LOGITRACK SIMPLE === pedidos: %d | pendientes: %d | monto total: $%lld.%02lld\n", hash.n, heap.n,
               montoTotal / 100, montoTotal % 100);
        printf("1 Generar CSV  2 Cargar CSV  3 Registrar  4 Buscar por ID  5 Despachar N\n");
        printf("6 Ver N mas antiguos  7 Rango de fechas  8 Cambiar estado  9 Cancelar  0 Salir\n> ");
        if (scanf("%d", &op) != 1) break;
        clock_t t0 = clock();
        if (op == 0) break;
        else if (op == 1) { int n; printf("Cantidad: "); scanf("%d", &n); generar("pedidos.csv", n); }
        else if (op == 2) { printf("Archivo: "); scanf("%63s", buf); cargar(buf); }
        else if (op == 3) {
            char linea[200];
            printf("Formato: id,dni,tipo,prioridad,monto,AAAA-MM-DD,HH:MM:SS,estado\n> ");
            scanf(" %199[^\n]", linea);
            Pedido* p = new Pedido;
            if (!parsear(linea, p)) { delete p; printf("Datos invalidos.\n"); }
            else if (!registrar(p)) { delete p; printf("ID duplicado.\n"); }
            else printf("Registrado.\n");
        }
        else if (op == 4) {
            printf("ID: "); scanf("%63s", buf);
            Pedido* p = hash.buscar(buf);
            if (p) mostrar(p); else printf("No existe.\n");
        }
        else if (op == 5) {
            int n; printf("N: "); scanf("%d", &n);
            for (int i = 0; i < n; i++) {
                Pedido* p = heap.extraerMax();
                if (!p) { printf("Cola vacia.\n"); break; }
                strcpy(p->estado, "EnTransito");
                mostrar(p);
            }
        }
        else if (op == 6) { int n, c = 0; printf("N: "); scanf("%d", &n); arbol.inorden(arbol.raiz, c, n); }
        else if (op == 7) {
            printf("Desde y hasta (AAAA-MM-DD AAAA-MM-DD): "); scanf("%63s %63s", buf, buf2);
            int a, m, d, cant = 0, vis = 0; long long suma = 0;
            sscanf(buf, "%d-%d-%d", &a, &m, &d);
            long long desde = ((long long)a * 10000 + m * 100 + d) * 1000000LL;
            sscanf(buf2, "%d-%d-%d", &a, &m, &d);
            long long hasta = ((long long)a * 10000 + m * 100 + d) * 1000000LL + 235959;
            arbol.rango(arbol.raiz, desde, hasta, cant, suma, vis);
            printf("Pedidos: %d | Monto total: $%lld.%02lld | nodos visitados: %d de %d\n", cant,
                   suma / 100, suma % 100, vis, hash.n);
        }
        else if (op == 8) {
            printf("ID: "); scanf("%63s", buf);
            printf("Nuevo estado: "); scanf(" %15[^\n]", buf2);
            Pedido* p = hash.buscar(buf);
            if (p) { cambiarEstado(p, buf2); mostrar(p); } else printf("No existe.\n");
        }
        else if (op == 9) {
            printf("ID: "); scanf("%63s", buf);
            if (cancelar(buf)) printf("Cancelado. Buscar de nuevo: %s\n", hash.buscar(buf) ? "ERROR" : "ya no existe");
            else printf("No existe.\n");
        }
        printf("(tiempo: %.4f ms)\n", (clock() - t0) * 1000.0 / CLOCKS_PER_SEC);
    }
    return 0;
}
