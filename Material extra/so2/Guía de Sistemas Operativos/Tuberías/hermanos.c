#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pipe(fd);
    
    // Crear hijo 1 (escritor)
    if (fork() == 0) {
        close(fd[0]);
        char msg[] = "Mensaje del hermano 1";
        write(fd[1], msg, strlen(msg) + 1);
        close(fd[1]);
        exit(0);
    }
    
    // Crear hijo 2 (lector)
    if (fork() == 0) {
        close(fd[1]);
        char buffer[100];
        read(fd[0], buffer, sizeof(buffer));
        printf("Hermano 2 recibió: %s\n", buffer);
        close(fd[0]);
        exit(0);
    }
    
    // Padre cierra ambos extremos
    close(fd[0]);
    close(fd[1]);
    
    wait(NULL);
    wait(NULL);
    
    return 0;
}