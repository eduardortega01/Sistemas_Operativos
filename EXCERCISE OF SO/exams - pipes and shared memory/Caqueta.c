#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

#define MAX_MINES 100

// Estructura para enviar resultados al padre
typedef struct {
    int dx[MAX_MINES], dy[MAX_MINES]; // Coordenadas donde se detectaron minas
    int count; // Cantidad de minas encontradas
    pid_t child; // Identificador del hijo que envía el resultado
} Map;

void showtree();

// Lee archivo y construye matriz dinámica
void read_file(const char *file, int ***mtx, int *rows, int *cols);

// Verifica si una celda (dx, dy) está cerca de una mina
void is_mine(int **mtx, int dx, int dy, int rows, int cols, Map *map);

int main(int argc, char **argv){

    // Validación de argumentos
    if(argc < 2){
        perror("Send a file\n");
        return EXIT_FAILURE;
    }

    pid_t root = getpid(); // Guarda PID del padre

    int rows, cols;
    int chunk; // Tamaño del bloque de trabajo por hijo

    int **mtx;
    int n_children = 4, child_id;

    int n_pipes = 4;
    int pipes[n_pipes][2]; // Un pipe por cada hijo

    // Lectura de la matriz desde archivo
    read_file(argv[1], &mtx, &rows, &cols);

    // Creación de pipes ANTES de fork (para que todos los hijos los hereden)
    for(int i=0; i < n_pipes; i++){
        pipe(pipes[i]);
    }

    // División del trabajo (filas por hijo)
    chunk = rows / n_children;

    // Creación de procesos hijos
    for(child_id=0 ; child_id < n_children; child_id++){
        if(!fork()) break; // Cada hijo sale con su child_id
    }

    // ===================== PROCESO PADRE =====================
    if(root == getpid()){

        // El padre SOLO LEE resultados → cierra extremos de escritura
        for(int i=0; i < n_pipes; i++) close(pipes[i][1]);

        // Espera a que todos los hijos terminen
        for(int i=0; i < n_children; i++) wait(NULL);

        Map results;

        // Lectura de resultados desde cada pipe
        for(int i=0; i < n_children; i++){
            read(pipes[i][0], &results, sizeof(Map));

            printf("Child [%d] found a mine in the position: ", results.child);

            // Imprime coordenadas encontradas por ese hijo
            for(int j=0; j < results.count; j++){
                printf("[%d][%d] ", results.dx[j], results.dy[j]);
            }
            printf("\n");
        }

        // Cierre de pipes de lectura
        for(int i=0; i < n_pipes; i++) close(pipes[i][0]);

    // ===================== PROCESOS HIJOS =====================
    }else{

        // Los hijos SOLO ESCRIBEN → cierran extremos de lectura
        for(int i=0; i < n_pipes; i++) close(pipes[i][0]);

        int idx_start, idx_end;
        Map map;

        map.count = 0;
        map.child = child_id; // Identifica qué hijo envía datos

        // Definición del rango de filas que procesa cada hijo
        idx_start = chunk * child_id;

        // El último hijo toma el resto si la división no es exacta
        idx_end = (child_id == n_children-1) ? rows : chunk * (child_id+1);

        // Recorrido de la submatriz asignada
        for(int i=idx_start; i < idx_end; i++){
            for(int j=0; j < cols; j++){

                if(mtx[i][j] == 1){ // posible mina

                    // Verifica si realmente es una mina (vecindad)
                    is_mine(mtx, i, j, rows, cols, &map);
                }
            }
        }

        // Envío de resultados al padre usando su pipe exclusivo
        write(pipes[child_id][1], &map, sizeof(map));

        // Cierre de pipes de escritura
        for(int i=0; i < n_pipes; i++) close(pipes[i][1]);
    }

    // Liberación de memoria (cada proceso tiene su copia tras fork)
    for(int i=0; i < rows; i++){
        free(mtx[i]);
    }
    free(mtx);

    return EXIT_SUCCESS;
}

// ===================== FUNCIÓN DE VALIDACIÓN =====================
void is_mine(int **mtx, int dx, int dy, int rows, int cols, Map *map){

    // Recorre vecinos (3x3 alrededor de la celda)
    for(int i=dx-1; i <= dx+1; i++){
        for(int j=dy-1; j <= dy+1; j++){

            if(i == dx && j == dy) continue; // Ignora la celda actual

            // Verifica límites de la matriz
            if(i >= 0 && i < rows && j >= 0 && j < cols){

                if(mtx[i][j] == 2){ // Se detecta mina real

                    // Guarda coordenadas
                    map->dx[map->count] = dx;
                    map->dy[map->count] = dy;
                    map->count++;

                    return; // Sale para no duplicar conteo
                }
            }
        }
    }
}

// ===================== LECTURA DE ARCHIVO =====================
void read_file(const char *file, int ***mtx, int *rows, int *cols){

    FILE *fl = fopen(file, "r");
    if(!fl){
        perror("Fail fopen\n");
        exit(1);
    }

    // Lectura de dimensiones
    fscanf(fl, "%d", rows);
    fscanf(fl, "%d", cols);

    printf("Rows: %d - Cols: %d\n", *rows, *cols);

    // Reserva memoria para filas
    *mtx = (int **) malloc(*rows * sizeof(int*));
    if(!(*mtx)){
        perror("Fail malloc\n");
        exit(1);
    }

    // Reserva memoria para columnas
    for(int i=0; i < *rows; i++){
        (*mtx)[i] = (int*) malloc(*cols * sizeof(int));
        if(!(*mtx)[i]){
            perror("Fail malloc cols\n");
            exit(1);
        }
    }

    // Lectura de la matriz
    for(int i=0; i < *rows; i++){
        for(int j=0; j < *cols; j++){
            fscanf(fl, "%d", &(*mtx)[i][j]);
        }
    }

    // Impresión (debug)
    for(int i=0; i < *rows; i++){
        for(int j=0; j < *cols; j++){
            printf("%d ", (*mtx)[i][j]);
        }
        printf("\n");
    }

    fclose(fl);
}

// ===================== ÁRBOL DE PROCESOS =====================
void showtree(){
    char cmd[20] = {""};

    sprintf(cmd, "pstree -cAlp %d", getpid());

    system(cmd);	
}