#include<sys/types.h> // tipo de dato pid_t
#include<unistd.h>    // para usar el fork()
#include<stdio.h>     // para usar el printf()
#include<stdlib.h>    // para usar el exit()
#include<sys/wait.h>  // para usar el wait()

// getpid() devuelve el PID del proceso que lo llama
// getppid() devuelve el PID del proceso padre del proceso que lo llama
/* la terminal es el proceso padre de todos los procesos */
// El return 0 le dice que el proceso ha terminado correctamente al quien lo creo
// proceso zombie: proceso hijo que ha terminado pero su padre no ha recogido su estado de terminacion
// proceso huérfano: proceso hijo cuyo padre ha terminado antes que el hijo, el padre de estos procesos es init (PID 1)
// wifexited(status): macro que devuelve true si el proceso hijo terminó de forma normal
// wexitstatus(status): macro que devuelve el valor devuelto por el proceso hijo si terminó
int main(void){
    pid_t pid = fork(); // tipo de dato para almacenar el valor devuelto por fork
    switch(pid){
        case -1: // error
            perror("fork");
            exit(EXIT_FAILURE);
        case 0: // proceso hijo
            usleep(10000); // dormir 10 ms para asegurar que el padre se ejecute primero 
            printf("Soy el proceso hijo, mi PID es %d y el PID de mi padre es %d\n", getpid(), getppid());
            return 10; 
            break;
        default: // proceso padre
            int status; 
            printf("Soy el proceso padre, mi PID es %d y el PID de mi padre es %d\n", getpid(), getppid());
            wait(&status); // espera a que termine el proceso hijo para evitar procesos huérfanos
            if (WIFEXITED(status))
            printf("status: %d\n", status); 
            break;
    }
    return 0;
}