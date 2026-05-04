#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define MAX_CHILDREN 4

void showtree();

// ===================== ESTRUCTURA DE CONTROL =====================
// Se usa para sincronizar padre e hijos
typedef struct {
    int done[MAX_CHILDREN]; // Cada hijo marca cuando termina
    int lock;               // Exclusión mutua (tipo mutex casero)
    int ready;              // 0 = no listo, 1 = datos listos
} SharedData;

// ===================== PROTOTIPOS =====================
unsigned int sizeof_dm(int rows, int cols, size_t size_elements);
void create_index(void **matrix, int rows, int cols, size_t size_elements);
void create_shm(int *shm_id, void **mtx, int rows, int cols, size_t size_elements);

void copy_matrix(int **A, int **B, int rows, int cols);
void show_matrix(int **A, int rows, int cols);
void read_file(const char *file, int ***mtx, int *rows, int *cols, int *n_days);

int num_neighbors(int **mtx, int dx, int dy, int rows, int cols, int child_id);

// ===================== MAIN =====================
int main(int argc, char **argv){

    if(argc < 2){
        perror("Send file");
        return EXIT_SUCCESS;
    }

    pid_t root = getpid();
    int n_children = MAX_CHILDREN, child_id;

    int **data, **results, **temp;
    int n_days;
    int rows, cols;

    int shm_id, rst_shm_id, shared_shm_id;

    SharedData *shared;

    // Lectura inicial
    read_file(argv[1], &temp, &rows, &cols, &n_days);

    // ===================== MEMORIA COMPARTIDA =====================
    shared_shm_id = shmget(IPC_PRIVATE, sizeof(SharedData), 0666 | IPC_CREAT);
    shared = (SharedData*) shmat(shared_shm_id, NULL, 0);

    create_shm(&shm_id, (void **)&data, rows, cols, sizeof(int));
    create_shm(&rst_shm_id, (void **)&results, rows, cols, sizeof(int));

    // Inicialización
    copy_matrix(data, temp, rows, cols);

    for(int i=0; i < rows; i++) free(temp[i]);
    free(temp);

    shared->lock = 0;
    shared->ready = 0;

    // ===================== CREACIÓN DE HIJOS =====================
    for(child_id=0; child_id < n_children; child_id++){
        if(!fork()) break;
    }

    // ===================== PADRE =====================
    if(root == getpid()){

        showtree();

        // Inicializa matriz de resultados
        copy_matrix(results, data, rows, cols);

        // Simulación por días
        for(int d=0; d < n_days; d++){

            // Espera a que hijos terminen ronda anterior
            while(shared->ready == 1){
                usleep(50000);
            }

            // Reinicia estado de hijos
            for(int i=0; i < n_children; i++){
                shared->done[i] = 0;
            }

            printf("----- Day: %d -----\n", d);

            // Copia resultados a data (nuevo estado base)
            copy_matrix(data, results, rows, cols);

            show_matrix(data, rows, cols);

            // Señal: datos listos para procesar
            shared->ready = 1;
        }

        // Espera a hijos
        for(int i=0; i < n_children; i++) wait(NULL);

        // Liberación de memoria
        shmdt(data);
        shmctl(shm_id, IPC_RMID, NULL);

        shmdt(results);
        shmctl(rst_shm_id, IPC_RMID, NULL);

    // ===================== HIJOS =====================
    }else{

        int count_end = 0, end_round = 0, neig = 0;

        for(int d=0; d < n_days; d++){

            neig = count_end = 0;

            // Espera hasta que el padre indique que hay datos listos
            while(!shared->ready || end_round){
                end_round = 0;
                usleep(50000);
            }

            // ===================== LOCK (EXCLUSIÓN MUTUA) =====================
            while(shared->lock) usleep(50000);
            shared->lock = 1;

            // Cada hijo aplica una regla diferente
            for(int i=0; i < rows; i++){
                for(int j=0; j < cols; j++){

                    switch(child_id){

                        case 0: // baja → media
                            if(data[i][j] == 0){
                                neig = num_neighbors(data, i, j, rows, cols, child_id);
                                if(neig >= 4){
                                    results[i][j] = 1;
                                }
                            }
                            break;

                        case 1: // media → baja
                            if(data[i][j] == 1){
                                neig = num_neighbors(data, i, j, rows, cols, child_id);
                                float prob = 0.1 + (neig * 0.03);
                                if(rand() < prob){
                                    results[i][j] = 0;
                                }
                            }
                            break;

                        case 2: // media → alta
                            if(data[i][j] == 1){
                                neig = num_neighbors(data, i, j, rows, cols, child_id);
                                if(neig >= 3){
                                    results[i][j] = 2;
                                }
                            }
                            break;

                        case 3: // alta → media
                            if(data[i][j] == 2){
                                neig = num_neighbors(data, i, j, rows, cols, child_id);
                                float prob = 0.15 + (neig * 0.05);
                                if(rand() < prob){
                                    results[i][j] = 1;
                                }
                            }
                            break;
                    }
                }
            }

            // Marca que terminó su trabajo
            shared->done[child_id] = 1;

            end_round = 1;

            // Libera lock
            shared->lock = 0;

            // ===================== BARRERA =====================
            for(int i=0; i < n_children; i++){
                if(shared->done[i] == 1){
                    count_end++;
                }
            }

            // Último en terminar → libera al padre
            if(count_end == n_children){
                shared->ready = 0;
            }
        }

        shmdt(results);
        shmdt(data);
    }

    return EXIT_SUCCESS;
}

// ===================== FUNCIONES AUXILIARES =====================

// Cuenta vecinos dependiendo del tipo de hijo
int num_neighbors(int **mtx, int dx, int dy, int rows, int cols, int child_id){

    int count = 0;

    for(int i=dx-1; i <= dx+1; i++){
        for(int j=dy-1; j <= dy+1; j++){

            if(i == dx && j == dy) continue;

            if(i >= 0 && i < rows && j >= 0 && j < cols){

                switch(child_id){

                    case 0:
                        if(mtx[i][j] == 1 || mtx[i][j] == 2) count++;
                        break;

                    case 1:
                        if(mtx[i][j] == 0) count++;
                        break;

                    case 2:
                        if(mtx[i][j] == 2) count++;
                        break;

                    case 3:
                        if(mtx[i][j] == 0) count++;
                        break;
                }
            }
        }
    }

    return count;
}

// Mostrar matriz
void show_matrix(int **A, int rows, int cols){
    for(int i=0; i < rows; i++){
        for(int j=0; j < cols; j++){
            printf("%d ", A[i][j]);
        }
        printf("\n");
    }
}

// Leer archivo
void read_file(const char *file, int ***mtx, int *rows, int *cols, int *n_days){

    FILE *fl = fopen(file, "r");

    if(!fl){
        perror("Fail fopen\n");
        exit(1);
    }

    fscanf(fl, "%d", n_days);
    fscanf(fl, "%d", rows);
    fscanf(fl, "%d", cols);

    printf("n_days %d. Rows: %d - Cols: %d\n", *n_days, *rows, *cols);

    *mtx = (int **) malloc(*rows * sizeof(int*));

    for(int i=0; i < *rows; i++){
        (*mtx)[i] = (int*) malloc(*cols * sizeof(int));
    }

    for(int i=0; i < *rows; i++){
        for(int j=0; j < *cols; j++){
            fscanf(fl, "%d", &(*mtx)[i][j]);
        }
    }

    fclose(fl);
}

// Copiar matriz
void copy_matrix(int **A, int **B, int rows, int cols){
    for(int i=0; i < rows; i++){
        for(int j=0; j < cols; j++){
            A[i][j] = B[i][j];
        }
    }
}

// ===================== SHM =====================

void create_shm(int *shm_id, void **mtx, int rows, int cols, size_t size_elements){

    size_t size_mtx = sizeof_dm(rows, cols, size_elements);

    *shm_id = shmget(IPC_PRIVATE, size_mtx, 0666 | IPC_CREAT);

    if(*shm_id == -1){
        perror("Fail shmget");
        exit(1);
    }

    *mtx = shmat(*shm_id, NULL, 0);

    if(*mtx == NULL){
        perror("Fail Shmat");
        exit(1);
    }

    create_index((void*)*mtx, rows, cols, size_elements);
}

unsigned int sizeof_dm(int rows, int cols, size_t size_elements){
    size_t size = rows * sizeof(void *);
    size += (rows * cols * size_elements);
    return size;
}

void create_index(void **matrix, int rows, int cols, size_t size_elements){

    size_t size_rows = cols * size_elements;

    matrix[0] = matrix + rows;

    for(int i = 1; i < rows; i++){
        matrix[i] = (matrix[i-1] + size_rows);
    }
}

// Mostrar árbol de procesos
void showtree(){
    char cmd[20] = {""};
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);	
}