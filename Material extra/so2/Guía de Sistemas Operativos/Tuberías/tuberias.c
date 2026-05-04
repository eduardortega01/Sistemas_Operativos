#include<unistd.h> 

int main(){
    int pipe(int pipefd[2]); // Creació de tuberías

    /*
    Parámetros:

    - pipefd: Array de 2 enteros que almacernará los descriptores
        - pipefd[0]: Extremo de lectura (read end)
        - pipefd[1]: Extremo de escritura (write end)

    retorno:

        - 0: Éxito
        - -1: Error (usar perror para ver el error)

        Ejemplo: 
    */

    int fd[2]; 

    if (pipe(fd) == -1) {
        perror("Error al crear pipe");
        exit(1);
    }

    // ahora: 
    // fd[0] es para leer
    // fd[1] es para escribir

    /* 
    Proceso A              TUBERÍA              Proceso B
    +---------------+       +----------+        +---------------+
    |               |       |          |        |               |
    |   write() --> | ----> | BUFFER   | -----> | --> read()    |
    |    fd[1]      |       |  KERNEL  |        |     fd[0]     |
    +---------------+       +----------+        +---------------+
    */
    return 0; 
}