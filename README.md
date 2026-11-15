# eda-ii-proyecto
Proyecto del curso de Estructuras de Datos II

## Contenido
- `pedido.h`, `tabla_hash.h`, `arbol_avl.h`, `max_heap.h`: estructuras (Tabla Hash + AVL + Max-Heap) sin STL.
- `sistema.h`: registro, cancelacion, carga/generacion de CSV y estadisticas.
- `main.cpp`: menu de consola.
- `datos/`: `pedidos_pequeno.csv` (30) y `pedidos_masivo.csv` (10 000).

## Compilar y ejecutar
g++ -std=c++11 -O2 main.cpp -o logistica && ./logistica
