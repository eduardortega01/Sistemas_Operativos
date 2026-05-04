#include <bits/posix2_lim.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct {
    int posicioni; 
    int posicionj;
} Ubicacion;

typedef struct{
    int nFilas;
    int nColumnas; 
    int **matriz;
} Cuadrante;

// leer y crear la matriz dinámica
int **leer_matriz(char *nombre, int *n, int *m) {
    FILE *archivo = fopen(nombre, "r");
    if (!archivo) {
        perror("Error al abrir el archivo\n"); 
        exit(EXIT_FAILURE); 
    }

    // leer número de filas
    fscanf(archivo, "%d", &*n);

    // leer número de columnas
    fscanf(archivo, "%d", &*m);

    if (n == NULL) {
        perror("Error: el tamaño de las filas no puede ser 0\n");
        exit(EXIT_FAILURE);
    } else if (m == NULL) { 
        perror("Error: el tamaño de las columnas no puede ser 0\n");
        exit(EXIT_FAILURE);
    }

    int **matriz = (int**) malloc(*n * sizeof(int*));
    for (int i = 0; i < *n; i++) {
        matriz[i] = (int*) malloc(*m * sizeof(int)); 
    }

    for (int i = 0; i < *n; i++) {
        for (int j = 0; j < *m; j++) {
            fscanf(archivo,  "%d", &matriz[i][j]);
        }
    }

    fclose(archivo);
    return matriz; 
}

// Función para crear una matriz dinámica de tamaño n x m
int **crear_matriz(int n, int m, int *nFilas, int *nColumnas) {
    *nFilas = n;
    *nColumnas = m;  
    int **matriz = malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
        matriz[i] = malloc(m * sizeof(int));
    return matriz;
}

// función para liberar memoría
void liberar_memoria(int **matriz, int n) {
    for (int i = 0; i < n; i++)
        free(matriz[i]);
    free(matriz);
}

// Función para imprimir la matríz
void imprimir_matriz(int **matriz, int n, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++)
            printf("%d ", matriz[i][j]);
        printf("\n");
    }
}

// Función que divide una matriz dinámica en cuatro cuadrantes
void dividir_en_cuadrantes(int **matriz, int n, int m, Cuadrante *q1, Cuadrante *q2, Cuadrante *q3, Cuadrante *q4) {
    int mitad_filas = n / 2;
    int mitad_columnas = m / 2;

    // Crear las 4 matrices cuadrantes
    *q1->matriz = *crear_matriz(mitad_filas, mitad_columnas, &q1->nFilas, &q1->nColumnas);
    *q2->matriz = *crear_matriz(mitad_filas, m - mitad_columnas, &q2->nFilas, &q2->nColumnas);
    *q3->matriz = *crear_matriz(n - mitad_filas, mitad_columnas, &q3->nFilas, &q3->nColumnas);
    *q4->matriz = *crear_matriz(n - mitad_filas, m - mitad_columnas, &q4->nFilas, &q4->nColumnas);

    // Copiar los elementos correspondientes
    for (int i = 0; i < mitad_filas; i++) {
        for (int j = 0; j < mitad_columnas; j++)
            q1->matriz[i][j] = matriz[i][j];
        for (int j = mitad_columnas; j < m; j++)
            q2->matriz[i][j - mitad_columnas] = matriz[i][j];
    }

    for (int i = mitad_filas; i < n; i++) {
        for (int j = 0; j < mitad_columnas; j++)
            q3->matriz[i - mitad_filas][j] = matriz[i][j];
        for (int j = mitad_columnas; j < m; j++)
            q4->matriz[i - mitad_filas][j - mitad_columnas] = matriz[i][j];
    }
}

int main(int argc, char *argv[]) {

    if (argc != 20) {
        fprintf(stderr, "Uso: %s <archivo>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t hijo1, hijo2, hijo3, hijo4; 

    int n, m; 
    int **matriz = leer_matriz(argv[1], &n , &m); 

    Cuadrante q1, q2, q3, q4; 

    dividir_en_cuadrantes(matriz, n,  m, &q1, &q2, &q3, &q4); 

    int pipes[8][2]; 
    for (int i = 0; i < 8; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Error al crear pipe"); 
            exit(EXIT_FAILURE); 
        }
    }

    hijo1 = fork(); 
    if (hijo1 == -1) {
        perror("Error al crear hijo1\n");
        exit(EXIT_FAILURE);
    }

    if (!hijo1) {

        close(pipes[0][1]); 
        close(pipes[1][0]); close(pipes[1][1]);
        close(pipes[2][0]); close(pipes[2][1]);
        close(pipes[3][0]); close(pipes[3][1]);
        close(pipes[4][0]); 
        close(pipes[5][0]); close(pipes[5][1]);
        close(pipes[6][0]); close(pipes[6][1]);
        close(pipes[7][0]); close(pipes[7][1]);
        
        Cuadrante cuadrante;

        read(pipes[0][0], &cuadrante, sizeof(Cuadrante));
        close(pipes[0][0]);

        Ubicacion ubicacion;
        ubicacion.posicioni = 0;
        ubicacion.posicionj = 0; 
        _Bool dos = 0;  
        for (int i = 0; i < cuadrante.nFilas; i++) {
            for (int j = 0; j < cuadrante.nColumnas; j++) {
                if (cuadrante.matriz[i][j] == 1) {
                    ubicacion.posicioni = i; 
                    ubicacion.posicionj = j;
                }
                if (cuadrante.matriz[i][j] == 2) {
                    dos = 1; 
                }
            }
        }

        if (ubicacion.posicioni != 0 && ubicacion.posicionj != 0 && (dos)) {
            write(pipes[4][1], &ubicacion, sizeof(Ubicacion));
            close(pipes[4][1]);
        } else {
            close(pipes[4][1]);
        }
        
        exit(EXIT_SUCCESS);
    }

    hijo2 = fork(); 
    if (hijo2 == -1) {
        perror("Error al crear hijo2\n");
        exit(EXIT_FAILURE); 
    }

    if (!hijo2) {

        close(pipes[0][0]); close(pipes[0][1]);
        close(pipes[1][1]); // cerrar escritura padre -> hijo2 
        close(pipes[2][0]); close(pipes[2][1]);
        close(pipes[3][0]); close(pipes[3][1]);
        close(pipes[4][0]); close(pipes[4][1]);
        close(pipes[5][0]); // cerrar lectura hijo2 -> padre
        close(pipes[6][0]); close(pipes[6][1]);
        close(pipes[7][0]); close(pipes[7][1]);

        Cuadrante c2; 
        read(pipes[1][0], &c2, sizeof(Cuadrante));
        close(pipes[1][0]); 

        Ubicacion ubi2; 
        ubi2.posicioni = 0; 
        ubi2.posicionj = 0; 
        _Bool dos2 = 0; 
        for (int i = 0; i < c2.nFilas; i++){
            for (int j = 0; j < c2.nColumnas; j++) {
                if (c2.matriz[i][j] == 1) {
                    ubi2.posicioni = i; 
                    ubi2.posicionj = j;
                }
                if (c2.matriz[i][j] == 2) {
                    dos2 = 1;
                }
            }
        }

        if (ubi2.posicioni != 0 && ubi2.posicionj != 0 && (dos2)) {
            write(pipes[5][1], &ubi2, sizeof(Ubicacion));
            close(pipes[5][1]);
        } else {
            close(pipes[5][1]);
        }

        exit(EXIT_SUCCESS);
    }

    hijo3 = fork(); 
    if (hijo3 == -1) {
        perror("Error al crear hijo3\n");
        exit(EXIT_FAILURE);
    }

    if (!hijo3) {

        close(pipes[0][0]); close(pipes[0][1]);
        close(pipes[1][0]); close(pipes[1][1]);
        close(pipes[2][1]); // cerrar escritura padre -> hijo3
        close(pipes[3][0]); close(pipes[3][1]);
        close(pipes[4][0]); close(pipes[4][1]);
        close(pipes[5][0]); close(pipes[5][1]);
        close(pipes[6][0]); // cerrar lectura hijo3 -> padre
        close(pipes[7][0]); close(pipes[7][1]);

        
        Cuadrante c3; 
        read(pipes[2][0], &c3, sizeof(Cuadrante));
        close(pipes[2][0]); 

        Ubicacion ubi3; 
        ubi3.posicioni = 0; 
        ubi3.posicionj = 0; 
        _Bool dos3 = 0; 
        for (int i = 0; i < c3.nFilas; i++){
            for (int j = 0; j < c3.nColumnas; j++) {
                if (c3.matriz[i][j] == 1) {
                    ubi3.posicioni = i; 
                    ubi3.posicionj = j;
                }
                if (c3.matriz[i][j] == 2) {
                    dos3 = 1;
                }
            }
        }

        if (ubi3.posicioni != 0 && ubi3.posicionj != 0 && (dos3)) {
            write(pipes[6][1], &ubi3, sizeof(Ubicacion));
            close(pipes[6][1]);
        } else {
            close(pipes[6][1]);
        }

        exit(EXIT_SUCCESS);
    }

    hijo4 = fork(); 
    if (hijo4 == -1) {
        perror("Error al crear hijo4\n"); 
        exit(EXIT_FAILURE);
    }

    if (!hijo4) {

        close(pipes[0][0]); close(pipes[0][1]);
        close(pipes[1][0]); close(pipes[1][1]);
        close(pipes[2][0]); close(pipes[2][1]);
        close(pipes[3][1]); // cerrar escritura padre -> hijo4
        close(pipes[4][0]); close(pipes[4][1]);
        close(pipes[5][0]); close(pipes[5][1]);
        close(pipes[6][0]); close(pipes[6][1]);
        close(pipes[7][0]); // cerrar lectura hijo4 -> padre

        Cuadrante c4; 
        read(pipes[3][0], &c4, sizeof(Cuadrante));
        close(pipes[3][0]); 

        Ubicacion ubi4; 
        ubi4.posicioni = 0; 
        ubi4.posicionj = 0; 
        _Bool dos4 = 0; 
        for (int i = 0; i < c4.nFilas; i++){
            for (int j = 0; j < c4.nColumnas; j++) {
                if (c4.matriz[i][j] == 1) {
                    ubi4.posicioni = i; 
                    ubi4.posicionj = j;
                }
                if (c4.matriz[i][j] == 2) {
                    dos4 = 1;
                }
            }
        }

        if (ubi4.posicioni != 0 && ubi4.posicionj != 0 && (dos4)) {
            write(pipes[7][1], &ubi4, sizeof(Ubicacion));
            close(pipes[7][1]);
        } else {
            close(pipes[7][1]);
        }

        exit(EXIT_SUCCESS);
    }

    // padre

    close(pipes[0][0]); // cerrar lectura padre -> hijo1 
    close(pipes[1][0]); // cerrar lectura padre -> hijo2 
    close(pipes[2][0]); // cerrar lectura padre -> hijo3
    close(pipes[3][0]); // cerrar lectura padre -> hijo4
    close(pipes[4][1]); // cerrar escritura hijo1 -> padre
    close(pipes[5][1]); // cerrar escritura hijo2 -> padre
    close(pipes[6][1]); // cerrar escritura hijo3 -> padre
    close(pipes[7][1]); // cerrar escritura hijo4 -> padre

    // enviar el primer cuadrante a hijo 1
    write(pipes[0][1], &q1, sizeof(Cuadrante)); 
    close(pipes[0][1]); // cerrar escritura padre -> hijo1

    write(pipes[1][1], &q2, sizeof(Cuadrante));
    close(pipes[1][1]); 

    write(pipes[2][1], &q3, sizeof(Cuadrante));
    close(pipes[2][1]); 

    write(pipes[3][1], &q4, sizeof(Cuadrante));
    close(pipes[3][1]); 

    Ubicacion h1;
    read(pipes[4][0], &h1, sizeof(Ubicacion));
    close(pipes[4][0]);
    Ubicacion h2; 
    read(pipes[5][0], &h2, sizeof(Ubicacion));
    close(pipes[5][0]);
    Ubicacion h3; 
    read(pipes[6][0], &h3, sizeof(Ubicacion));
    close(pipes[6][0]);
    Ubicacion h4; 
    read(pipes[7][0], &h4, sizeof(Ubicacion));
    close(pipes[7][0]);

    wait(NULL);
    wait(NULL);
    wait(NULL); 
    wait(NULL);

    if (h1.posicioni != 0) { 
        printf("Mina ubicicada en el primer cuadrante en la posición [%d][%d]\n", h1.posicioni, h1.posicionj);
    }

    if (h2.posicioni != 0) { 
        printf("Mina ubicicada en el segundo cuadrante en la posición [%d][%d]\n", h2.posicioni, h2.posicionj);
    }

    if (h3.posicioni != 0) { 
        printf("Mina ubicicada en el tercer cuadrante en la posición [%d][%d]\n", h3.posicioni, h3.posicionj);
    }

    if (h4.posicioni != 0) { 
        printf("Mina ubicicada en el cuarto cuadrante en la posición [%d][%d]\n", h4.posicioni, h4.posicionj);
    }
    return 0;
}