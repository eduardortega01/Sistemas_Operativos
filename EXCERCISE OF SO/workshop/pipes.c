#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_LEN 256 //para tener un tamaño fijo de lectura/escritura

void showtree();

int main(int argc, char **argv){
    if(argc < 2){
        perror("Send argument\n");
        return EXIT_FAILURE;
    }
    int n_children = 2, child_id;
    int n_pipes = n_children, pipes[n_pipes][2];  //[2][2] 1: escribir,  0: leer
    pid_t root = getpid();

    for(int i=0; i < n_pipes; i++){
        pipe(pipes[i]);
    }

    for(child_id=0; child_id < n_children; child_id++){
        if(!(fork())){
            break;
        }
    }

    if(root == getpid()){
        showtree();
        char buff[MAX_LEN];
        int line=0;

        for(int i=0; i < n_pipes; i++){ //cierro lectura
            close(pipes[i][0]);
            close(pipes[i][0]);
        }
        FILE *fl = fopen(argv[1], "r");
        if(!fl) perror("fail file");

        while(fgets(buff, MAX_LEN, fl)){
            buff[strcspn(buff, "\n")] = '\0';
            if(line % 2 == 0){
                printf("par: %s: \n", buff);
                write(pipes[0][1], buff, MAX_LEN); //64
            }else{
                printf("impar: %s\n", buff);
                write(pipes[1][1], buff, MAX_LEN); //64
            }
            line++;
        }

        fclose(fl);

        for(int i=0; i < n_pipes; i++){//cierro escritura
            close(pipes[i][1]);
            close(pipes[i][1]);
        }
        for(int i=0; i < n_children; i++) wait(NULL);

    }else{
        for(int i=0; i < n_pipes; i++){ //cierro escritura
            close(pipes[i][1]);
            close(pipes[i][1]);
        }
        char buff[MAX_LEN];

        while(read(pipes[child_id][0], buff, MAX_LEN) > 0){
            printf("Child (%d), Word: %s\n", getpid(), buff);
        }

        for(int i=0; i < n_pipes; i++){ //cierro lectura
            close(pipes[i][0]);
            close(pipes[i][0]);
        }
    }

    return EXIT_SUCCESS;
}

void showtree(){
    char cmd[20] = {""};
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);	
}