#include<sys/types.h> // tipo de dato pid_t
#include<unistd.h>    // para usar el fork()
#include<stdio.h>     // para usar el printf()
#include<stdlib.h>    // para usar el exit()
#include<sys/wait.h>  // para usar el wait()

int main(void){
    pid_t root, child;
    int i, j;
    root = getpid();
    printf("root: %d\n", root);
    for (i = 0; i < 3; i++){
        if (!fork()){
            break;
        }
    }

    if (root == getpid()){
        for (i = 0; i < 3; i++)
            wait(NULL);
        printf("Finalizando padre %d\n", getpid());
    }
        
            if (i == 3){
                child = getpid();
                for (i = 0; i < 3; i++){
                    if (!fork()){
                    break;
                    }
                }
            if (child == getpid()){
                for (j = 0; j < 3; j++)
                wait(NULL);
            printf("Soy el proceso hijo, mi PID es %d y el PID de mi padre es %d\n", getpid(), getppid());
            } else {
                printf("Soy un proceso hijo, mi PID es %d y el PID de mi padre es %d\n", getpid(), getppid());
            }
            }
            return 0;
        }
    
