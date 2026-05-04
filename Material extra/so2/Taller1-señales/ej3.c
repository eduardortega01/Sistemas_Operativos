#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>

void handler(int s){}

void print_debug_tree(){
    char cmd[50];
    sprintf(cmd, "pstree -lp %d", getpid());
    system(cmd);
}

int main(int argc, char** argv){

    signal(SIGUSR1, handler);

    pid_t root = getpid();

    int n_hijos = 3;
    int profundidad = 2;

    //pid_t* vec_hijos = (pid_t*)malloc(sizeof(vec_hijos) * )
    pid_t vec_hijos[n_hijos];

    int i = 0; 
    for(; i < n_hijos; ++i){
        vec_hijos[i] = fork();
        if(!vec_hijos[i])
            break;
    }

    if(root == getpid()){ //Padre

        usleep(500);

        print_debug_tree();
        printf("PID: [%d] Enviando señal a %d\n", getpid(), vec_hijos[n_hijos - 1] );
        kill(vec_hijos[n_hijos - 1], SIGUSR1);
        pause();

    } else { //Hijos

        int flag = 0;
        int j = 0; //CHECKPOINT
        pid_t _hijo;
        for(; j < profundidad; j++){
            if((_hijo = fork()))
                break;

            flag = 1;
        }

        if(_hijo == 0){  //Hijo del hijo

            if(j == profundidad - 1){
                pause();
                printf("PID: [%d] Enviando señal al padre: %d\n", getpid(), getppid());
                kill(getppid(), SIGUSR1);
                return 0;
            }

            pause();
            pid_t padre = getppid();
            printf("PID: [%d] Enviando señal al padre: %d\n",getpid(), padre);
            kill(padre, SIGUSR1);

        }else if (flag == 0) { //Hijo

            if(i == 0){ //Codigo del primer hijo del primer nivel

                pause();
                printf("PID: [%d] Enviando señal al hijo: %d\n",getpid(), _hijo);
                kill(_hijo, SIGUSR1);
                pause();
                printf("PID: [%d] Enviando señal al padre: %d\n",getpid(), root);
                kill(root, SIGUSR1);

                return 0;
            } //Codigo del resto de hijos del primer nivel

            pause();
            printf("PID: [%d] Enviando señal al hijo: %d\n",getpid(), _hijo);
            kill(_hijo, SIGUSR1);
            pause();
            printf("PID: [%d] Enviando señal a %d\n", getpid(), vec_hijos[i - 1]);
            kill(vec_hijos[i-1], SIGUSR1);

        } else {

            pause();
            printf("PID: [%d] Enviando señal al hijo: %d\n",getpid(), _hijo);
            kill(_hijo, SIGUSR1);
            pause();
            printf("PID: [%d] Enviando señal al padre: %d\n",getpid(), getppid());
            kill(getppid(), SIGUSR1);
        }
    }

    return 0;

}
