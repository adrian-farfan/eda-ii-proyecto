// Sistema de pedidos (consola). Sin STL: Hash + AVL + Max-Heap hechos a mano.
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



int main() {
    int op;
    char buf[64], buf2[64];
    for (;;) {
        printf("\n=== LOGITRACK SIMPLE === pedidos: %d | pendientes: %d\n", hash.n, heap.n);
        printf("1 Generar CSV  2 Cargar CSV  3 Registrar  4 Buscar por ID  5 Despachar N\n");
        printf("6 Ver N mas antiguos  7 Rango de fechas  8 Cambiar estado  9 Cancelar  10 Estadisticas  0 Salir\n> ");
        if (scanf("%d", &op) != 1) break;
        clock_t t0 = clock();
        if (op == 0) break;
        else if (op == 1) { int n; printf("Archivo y cantidad: "); scanf("%63s %d", buf, &n); generar(buf, n); }
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
        else if (op == 6) { int n, c = 0; printf("N: "); scanf("%d", &n); inorden(raiz, c, n); }
        else if (op == 7) {
            printf("Desde y hasta (AAAA-MM-DD AAAA-MM-DD): "); scanf("%63s %63s", buf, buf2);
            int a, m, d, cant = 0, vis = 0; long long suma = 0;
            sscanf(buf, "%d-%d-%d", &a, &m, &d);
            long long desde = ((long long)a * 10000 + m * 100 + d) * 1000000LL;
            sscanf(buf2, "%d-%d-%d", &a, &m, &d);
            long long hasta = ((long long)a * 10000 + m * 100 + d) * 1000000LL + 235959;
            rango(raiz, desde, hasta, cant, suma, vis);
            printf("Pedidos: %d | Monto total: $%lld.%02lld | nodos visitados: %d de %d\n", cant,
                   suma / 100, suma % 100, vis, hash.n);
        }
        else if (op == 8) {
            printf("ID: "); scanf("%63s", buf);
            printf("Nuevo estado (puede tener espacios): "); scanf(" %63[^\n]", buf2);
            Pedido* p = hash.buscar(buf);
            if (p) { cambiarEstado(p, buf2); mostrar(p); } else printf("No existe.\n");
        }
        else if (op == 9) {
            printf("ID: "); scanf("%63s", buf);
            if (cancelar(buf)) printf("Cancelado. Buscar de nuevo: %s\n", hash.buscar(buf) ? "ERROR" : "ya no existe");
            else printf("No existe.\n");
        }
        else if (op == 10) estadisticas();
        printf("(tiempo: %.4f ms)\n", (clock() - t0) * 1000.0 / CLOCKS_PER_SEC);
    }
    return 0;
}

