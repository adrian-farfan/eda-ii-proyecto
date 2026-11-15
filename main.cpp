// Sistema de pedidos (consola). Sin STL: Hash + AVL + Max-Heap hechos a mano.
#include "sistema.h"

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
