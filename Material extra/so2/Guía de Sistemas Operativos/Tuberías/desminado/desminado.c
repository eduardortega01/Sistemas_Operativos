#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct {
    int fila_inicio;
    int fila_fin; 
    int columna_inicio;
    int columna_fin;
} Cuandrante;

int **leer_martriz (const char *nombre, int *n, int *m) {
    FILE *archivo = fopen(nombre, "r");
    if (!archivo) {
        perror("Error al crear el archivo\n"); 
        exit(EXIT_FAILURE); 
    }

    // leer dimensianos
    fscanf(archivo, "%d", n);

    fscanf(archivo, "%d", m);


    // reservar memoria dinámica para matriz
    int **mat = (int**) malloc (*n * sizeof(int *));
    if (!mat) {
        perror("Error al asignar memoria");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < *n; i++) {
        mat[i] = (int*) malloc(*m * sizeof(int));
        if (!mat[i]) {
            perror("Error al asignar memoria para fila");
            exit(EXIT_FAILURE);
        } 
    }

    // lee cada línea de la matriz
    for (int i = 0; i < *n; i++) {
        for (int j = 0; j < *m; j++) {
            // leer un dígito individual
            char c;
            fscanf(archivo, " %c", &c);  // espacio para ignorar saltos o espacios
            mat[i][j] = c - '0'; // convertir carácter a número
        }
    }

    fclose(archivo);
    return mat; 
}

// Función para imprimir una matriz
void imprimir_matriz(int **matriz, int n, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++)
            printf("%d ", matriz[i][j]);
        printf("\n");
    }
}

int main(){

    int pipes_envio_padre[4][2];
    int pipes_envio_hijo[4][2];

    for (int i = 0; i < 4; i++) {
        pipe(pipes_envio_padre[i]);
        pipe(pipes_envio_hijo[i]);
    }

    pid_t hijo1, hijo2, hijo3, hijo4;

    hijo1 = fork(); 
    if (!hijo1) {

    }

    int nfilas, ncolumnas; 
    int **matriz = leer_martriz("matriz.txt", &nfilas, &ncolumnas);

    imprimir_matriz(matriz, nfilas, ncolumnas); 

    int mitad_filas = nfilas / 2; 
    int mitad_columnas = ncolumnas / 2; 

    for (int i = 0; i < 4; i++) {
        close(pipes_envio_hijo[i][1]);
        close(pipes_envio_padre[i][0]); 
    }

    Cuandrante c1; 
    c1.fila_inicio = 0;
    c1.fila_fin = mitad_filas;
    c1.columna_inicio = 0;
    c1.columna_fin = mitad_filas; 
    write(pipes_envio_padre[0][1], &c1, sizeof(Cuandrante)); 
    return 0; 
}

/* 

Matriz N filas x M columnas (ejemplo 5x10):

┌──────────────┬──────────────┐
│      C1      │      C2      │
│   (0,0)      │   (0,M/2)    │
│              │              │
├──────────────┼──────────────┤
│      C3      │      C4      │
│  (N/2,0)     │  (N/2,M/2)   │
│              │              │
└──────────────┴──────────────┘

int mitad_filas = N / 2;      // Divide las filas a la mitad
int mitad_columnas = M / 2;   // Divide las columnas a la mitad

// CUADRANTE 1 (Superior Izquierdo)
// filas: 0 hasta mitad_filas-1
// columnas: 0 hasta mitad_columnas-1
for(int i = 0; i < mitad_filas; i++) {
    for(int j = 0; j < mitad_columnas; j++) {
        // matriz[i][j]
    }
}

// CUADRANTE 2 (Superior Derecho)
// filas: 0 hasta mitad_filas-1
// columnas: mitad_columnas hasta M-1
for(int i = 0; i < mitad_filas; i++) {
    for(int j = mitad_columnas; j < M; j++) {
        // matriz[i][j]
    }
}

// CUADRANTE 3 (Inferior Izquierdo)
// filas: mitad_filas hasta N-1
// columnas: 0 hasta mitad_columnas-1
for(int i = mitad_filas; i < N; i++) {
    for(int j = 0; j < mitad_columnas; j++) {
        // matriz[i][j]
    }
}

// CUADRANTE 4 (Inferior Derecho)
// filas: mitad_filas hasta N-1
// columnas: mitad_columnas hasta M-1
for(int i = mitad_filas; i < N; i++) {
    for(int j = mitad_columnas; j < M; j++) {
        // matriz[i][j]
    }
}
```

## Ejemplo visual con matriz 5x10:
```
Matriz 5 filas x 10 columnas:
0 0 0 0 2|1 0 1 0 0
0 2 0 0 0|0 0 0 0 0
─────────┼─────────
1 0 0 0 0|0 0 0 2 1
0 0 0 1 0|0 0 0 0 0
0 0 0 0 0|0 1 0 0 0

Cuadrante 1 (2x5):    Cuadrante 2 (2x5):
0 0 0 0 2             1 0 1 0 0
0 2 0 0 0             0 0 0 0 0

Cuadrante 3 (3x5):    Cuadrante 4 (3x5):
1 0 0 0 0             0 0 0 2 1
0 0 0 1 0             0 0 0 0 0
0 0 0 0 0             0 1 0 0 0
*/