#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_USERS 10000

// ===================== ESTRUCTURAS =====================

// Guarda resultados por usuario
typedef struct {
    int video_id;
    int user_id;
    int face_score, audio_score, sync_score;
    int origin;

    int face_risk;
    int audio_risk;
    int sync_risk;
    int count_suspisous;
    int is_suspisous;
} Result;

// Control de sincronización entre procesos
typedef struct {
    int done[3];     // cada hijo marca cuando termina
    int end;         // señal de finalización
    int block_num;   // indica nuevo bloque disponible
} SharedData;

// ===================== PROTOTIPOS =====================

void showtree();
unsigned int sizeof_dm(int rows, int cols, size_t size_elements);
void create_index(void **matrix, int rows, int cols, size_t size_elements);
void create_shm(int *shm_id, void **mtx, int rows, int cols, size_t size_elements);

// ===================== MAIN =====================

int main(int argc, char **argv){

    // Validación de argumentos
    if(argc < 2){
        perror("Send file");
        return EXIT_SUCCESS;
    }

    pid_t root = getpid(); // identificar padre

    int n_children = 3, child_id;

    int **mtx; // buffer compartido (bloque de datos)
    int cols = 6;
    int B, N;

    int shm_id, shared_shm_id, results_shm_id;

    SharedData *shared;
    Result *results;

    // Leer tamaño de bloque y total
    FILE *fl = fopen(argv[1], "r");
    fscanf(fl, "%d %d", &B, &N);
    fclose(fl);

    // ===================== CREACIÓN DE SHM =====================

    create_shm(&shm_id, (void **)&mtx, B, cols, sizeof(int));

    // Memoria compartida para sincronización
    shared_shm_id = shmget(IPC_PRIVATE, sizeof(SharedData), 0666 | IPC_CREAT);

    // Memoria compartida para resultados globales
    results_shm_id = shmget(IPC_PRIVATE, MAX_USERS * sizeof(Result), 0666 | IPC_CREAT);

    // Conexión a memoria compartida
    shared = (SharedData*) shmat(shared_shm_id, NULL, 0);
    results = (Result*) shmat(results_shm_id, NULL, 0);

    // Inicializar estructuras
    memset(shared, 0, sizeof(SharedData));
    memset(results, 0, MAX_USERS * sizeof(Result));

    // ===================== CREAR HIJOS =====================

    for(child_id=0; child_id < n_children; child_id++){
        if(!fork()) break;
    }

    // ===================== PADRE =====================

    if(root == getpid()){
        showtree();

        FILE *fl = fopen(argv[1], "r");

        fscanf(fl, "%d %d", &B, &N);
        printf("B: %d, N: %d\n", B, N);

        int total_susp = 0, total_bann = 0;

        shared->end = 0;
        shared->block_num = 0;

        // Procesamiento por bloques
        for(int i=0; i < N/B; i++){

            // Cargar bloque en memoria compartida
            for(int j=0; j < B; j++){
                fscanf(fl, "%d %d %d %d %d %d",
                    &mtx[j][0],
                    &mtx[j][1],
                    &mtx[j][2],
                    &mtx[j][3],
                    &mtx[j][4],
                    &mtx[j][5]
                );
            }

            // Reiniciar estado de hijos
            shared->done[0] = 0;
            shared->done[1] = 0;
            shared->done[2] = 0;

            // Avisar nuevo bloque
            shared->block_num++;

            // Esperar a que todos los hijos terminen
            while(!shared->done[0] || !shared->done[1] || !shared->done[2]){
                usleep(5000);
            }

            // Evaluación final del bloque
            for(int j=0; j < B; j++){
                int user = mtx[j][1];

                if((results[user].face_risk +
                     results[user].audio_risk +
                     results[user].sync_risk) >= 2){

                    results[user].count_suspisous++;
                    results[user].is_suspisous = 1;
                    total_susp++;
                }
            }
        }

        // Señal de fin
        shared->end = 1;

        // Determinar usuarios baneados
        for(int i=0; i < MAX_USERS; i++){
            if(results[i].count_suspisous > 3){
                printf("User %d, Count: %d\n", i, results[i].count_suspisous);
                total_bann++;
            }
        }

        // Esperar hijos
        for(int i=0; i < n_children; i++) wait(NULL);

        printf("All banned: %d\n", total_bann);
        printf("All suspisous: %d\n", total_susp);

        fclose(fl);

        // Liberar memoria compartida
        shmdt(mtx);
        shmdt(shared);
        shmdt(results);

        shmctl(shared_shm_id, IPC_RMID, NULL);
        shmctl(shm_id, IPC_RMID, NULL);
        shmctl(results_shm_id, IPC_RMID, NULL);

    // ===================== HIJOS =====================

    }else{

        int last_block = 0;

        printf("Child %d\n", child_id);

        while(!shared->end){

            // Esperar nuevo bloque
            while(last_block == shared->block_num){

                if(shared->end){
                    shmdt(shared);
                    shmdt(mtx);
                    shmdt(results);
                    exit(0);
                }

                usleep(2000);
            }

            last_block = shared->block_num;

            // Procesar bloque
            for(int i=0; i < B; i++){

                int user = mtx[i][1];

                switch(child_id){
                    case 0:
                        results[user].face_risk = (mtx[i][2] >= 80);
                        break;

                    case 1:
                        results[user].audio_risk = (mtx[i][3] >= 75);
                        break;

                    case 2:
                        results[user].sync_risk = (mtx[i][4] <= 35);
                        break;
                }
            }

            // Marcar que terminó
            shared->done[child_id] = 1;
        }

        // Liberar memoria
        shmdt(shared);
        shmdt(mtx);
        shmdt(results);
    }

    return EXIT_SUCCESS;
}

// ===================== FUNCIONES SHM =====================

// Calcula tamaño necesario de la matriz
unsigned int sizeof_dm(int rows, int cols, size_t size_elements){
    size_t size = rows * sizeof(void *);
    size += rows * cols * size_elements;
    return size;
}

// Crea índices de filas (matriz continua)
void create_index(void **matrix, int rows, int cols, size_t size_elements){
    size_t size_rows = cols * size_elements;

    matrix[0] = matrix + rows;

    for(int i = 1; i < rows; i++){
        matrix[i] = (matrix[i-1] + size_rows);
    }
}

// Reserva y conecta memoria compartida
void create_shm(int *shm_id, void **mtx, int rows, int cols, size_t size_elements){

    size_t size_mtx = sizeof_dm(rows, cols, size_elements);

    *shm_id = shmget(IPC_PRIVATE, size_mtx, 0666 | IPC_CREAT);
    if(*shm_id == -1){
        perror("Fail shmget");
        exit(1);
    }

    *mtx = shmat(*shm_id, NULL, 0);
    if(*mtx == NULL){
        perror("Fail shmat");
        exit(1);
    }

    create_index(*mtx, rows, cols, size_elements);
}

// Mostrar árbol de procesos
void showtree(){
    char cmd[50];
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);
}