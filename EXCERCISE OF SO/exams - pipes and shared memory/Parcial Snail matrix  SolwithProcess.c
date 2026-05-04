#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <math.h>

void showtree();

// Calcula el tamaño total necesario para una matriz dinámica en memoria compartida
unsigned int sizeof_dm(int rows, int cols, size_t size_elements);

// Crea el índice de filas (punteros) dentro del bloque continuo de memoria
void create_index(void **matrix, int rows, int cols, size_t size_elements);

// Reserva memoria compartida y construye la matriz
void create_shm(int *shm_id, void **mtx, int rows, int cols, size_t size_elements);

// Imprime matriz
void show_matrix(int **mtx, int rows, int cols);

// Multiplicación de matrices: calcula una celda C[dx][dy]
int mult_matx(int **A, int **B, int size, int dx, int dy);

int main(int argc, char **argv){

    // Validación de argumento (tamaño de la matriz)
    if(argc < 2){
        perror("Send k");
        return EXIT_SUCCESS;
    }

    int k = atoi(argv[1]); // Tamaño de la matriz (kxk)

    int **A, **B, **C; // Matrices en memoria compartida
    int rows, cols;

    int shm_A_id, shm_B_id, shm_C_id; // IDs de memoria compartida

    rows = cols = k;

    pid_t root = getpid(); // PID del padre

    int n_children, child_id;

    // ===================== CREACIÓN DE MEMORIA COMPARTIDA =====================
    // Todos los procesos accederán a las mismas matrices (NO copia como fork normal)
    create_shm(&shm_A_id, (void **)&A, rows, cols, sizeof(int));
    create_shm(&shm_B_id, (void **)&B, rows, cols, sizeof(int));
    create_shm(&shm_C_id, (void **)&C, rows, cols, sizeof(int));

    // Número de hijos (trabajan por “capas” de la matriz)
    n_children = (k % 2 == 0) ?  k/2 : k/2+1;
    printf("N_children: %d\n", n_children);

    // Creación de procesos hijos
    for(child_id=0; child_id < n_children; child_id++){
        if(!fork()) break; // Cada hijo obtiene su ID lógico
    }

    // ===================== PROCESO PADRE =====================
    if(root == getpid()){

        int n1 = 2, n2 = 3;

        // Inicializa matrices A y B
        for(int i=0; i < rows; i++){
            for(int j=0; j < cols; j++){
                A[i][j] = pow(n1, i); // valores por fila
                B[i][j] = pow(n2, i);
            }
        } 

        // Espera a que los hijos terminen cálculos
        for(int i=0; i < n_children; i++) wait(NULL);

        printf("\n");

        // Imprime matriz resultado
        show_matrix(C, rows, cols);
        
        // ===================== LIBERACIÓN DE SHM =====================
        shmdt(A);
        shmdt(B);
        shmdt(C);

        shmctl(shm_A_id, IPC_RMID, NULL);
        shmctl(shm_B_id, IPC_RMID, NULL);
        shmctl(shm_C_id, IPC_RMID, NULL);

    // ===================== PROCESOS HIJOS =====================
    }else{

        sleep(child_id+2); // sincronización simple

        printf("Child %d\n", child_id);
        
        // -------- PROCESAMIENTO POR "CAPAS" --------
        // Cada hijo calcula una capa externa de la matriz C

        // FILAS (superior e inferior)
        for(int i=child_id; i < k-child_id; i++){

            // Fila superior
            C[child_id][i] = mult_matx(A, B, k, child_id, i);

            // Fila inferior
            C[k-child_id-1][i] = mult_matx(A, B, k, k-child_id-1, i);
        }

        // COLUMNAS (izquierda y derecha)
        for(int j=child_id+1; j < k-child_id-1; j++){

            // Columna izquierda
            C[j][child_id] =  mult_matx(A, B, k, j, child_id);

            // Columna derecha
            C[j][k-child_id-1] =  mult_matx(A, B, k, j, k-child_id-1);
        }

        // Desconexión de memoria compartida
        shmdt(A);
        shmdt(B);
        shmdt(C);
    }

    return EXIT_SUCCESS;
}

// ===================== MULTIPLICACIÓN MATRIZ =====================
int mult_matx(int **A, int **B, int size, int dx, int dy){

    int value = 0;

    // Producto punto fila(A) * columna(B)
    for(int i=0; i < size; i++){
        value += A[dx][i] * B[i][dy];
    }

    return value;
}

// ===================== MOSTRAR MATRIZ =====================
void show_matrix(int **mtx, int rows, int cols){
    for(int i=0; i < rows; i++){
        for(int j=0; j < cols; j++){
            printf("[%d]\t ", mtx[i][j]);
        }
        printf("\n");
    }

    printf("\n");
}

// ===================== CREAR MEMORIA COMPARTIDA =====================
void create_shm(int *shm_id, void **mtx, int rows, int cols, size_t size_elements){

    size_t size_mtx;

    // Calcula tamaño total del bloque
    size_mtx = sizeof_dm(rows, cols, sizeof(size_elements));

    // Crea segmento de memoria compartida
    *shm_id = shmget(IPC_PRIVATE, size_mtx, 0666 | IPC_CREAT);
    if(*shm_id == -1){
        perror("Fail shmget");
        exit(1);
    }

    // Asocia el segmento al espacio de direcciones del proceso
    *mtx = shmat(*shm_id, NULL, 0);
    if(*mtx == NULL){
        perror("Fail Shmat");
        exit(1);
    }

    // Construye la estructura tipo matriz (índices)
    create_index((void*)*mtx, rows, cols, size_elements);
}

// ===================== TAMAÑO MATRIZ DINÁMICA =====================
unsigned int sizeof_dm(int rows, int cols, size_t size_elements){

    size_t size = rows * sizeof(void *); // punteros a filas
    size += (rows * cols * size_elements); // datos reales

    return size;
}

// ===================== CREAR ÍNDICES DE MATRIZ =====================
void create_index(void **matrix, int rows, int cols, size_t size_elements){

    int i;
    size_t size_rows = cols * size_elements;

    // Primera fila apunta después del bloque de punteros
    matrix[0] = matrix + rows;

    // Construcción de filas contiguas en memoria
    for(i = 1; i < rows; i++){
        matrix[i] = (matrix[i-1] + size_rows);
    }
}

// ===================== ÁRBOL DE PROCESOS =====================
void showtree(){
    char cmd[20] = {""};
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);	
}