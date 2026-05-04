#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <math.h>
#include <signal.h>

void showtree();

unsigned int sizeof_dm(int rows, int cols, size_t size_elements);
void create_index(void **matrix, int rows, int cols, size_t size_elements);
void create_shm(int *shm_id, void **mtx, int rows, int cols, size_t size_elements);

void show_matrix(int **mtx, int rows, int cols);

void handler(int s){}

int main(int argc, char **argv){

    signal(SIGUSR1, handler);
    int k = atoi(argv[1]);
    int **A;
    int rows, cols;
    int shm_A_id;

    rows = cols = k;
    
    pid_t root = getpid();
    pid_t h1;

    create_shm(&shm_A_id, (void **)&A, rows, cols, sizeof(int));

    h1=fork();
     if (h1==0) { 

    }

    if(root == getpid()){ //padre
        sleep(1);
        int dato;
        for(int i=0; i < rows; i++){
            for(int j=0; j < cols; j++){
                scanf("%d", &dato);
                A[i][j] = dato;
                
            }
        } 

        //for(int i=0; i < n_children; i++) wait(NULL);
        kill(h1, SIGUSR1);
        shmdt(A);
        shmctl(shm_A_id, IPC_RMID, NULL);
        sleep(1);
        pause();


    }else{ //hijo

        pause();
        
        show_matrix(A, rows, cols);

        shmdt(A);
    }

    return EXIT_SUCCESS;
}


void show_matrix(int **mtx, int rows, int cols){
    for(int i=0; i < rows; i++){
        for(int j=0; j < cols; j++){
            printf("[%d]\t ", mtx[i][j]);
        }
        printf("\n");
    }

    printf("\n");
}

void create_shm(int *shm_id, void **mtx, int rows, int cols, size_t size_elements){
    size_t size_mtx;

    size_mtx = sizeof_dm(rows, cols, sizeof(size_elements));
    *shm_id = shmget(IPC_PRIVATE, size_mtx, 0600 | IPC_CREAT);
    if(*shm_id == -1){
        perror("Fail shmget");
        exit(1);
    }

    *mtx = shmat(*shm_id, NULL, 0);
    if(*mtx == NULL){
        perror("Fail Shmat");
        exit(1);
    }

    create_index((void**)*mtx, rows, cols, size_elements);
}

unsigned int sizeof_dm(int rows, int cols, size_t size_elements){
    size_t size = rows * sizeof(void *);
    size += (rows * cols * size_elements); 
    return size;
}

void create_index(void **matrix, int rows, int cols, size_t size_elements){
    int i;
    size_t size_rows = cols * size_elements;
    matrix[0] = matrix + rows;
    for(i = 1; i < rows; i++){
        matrix[i] = (matrix[i-1] + size_rows);
    }
}

void showtree(){
    char cmd[20] = {""};
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);	
}