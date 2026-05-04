#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pid_t pid;
    char buffer[100];
    
    // Paso 1: Crear tubería
    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }
    
    // Paso 2: Crear proceso hijo
    pid = fork();
    
    if (pid == -1) {
        perror("fork");
        return 1;
    }
    
    if (pid == 0) {
        // PROCESO HIJO (LECTOR)
        close(fd[1]);  // Cerrar escritura
        
        read(fd[0], buffer, sizeof(buffer));
        printf("Hijo recibió: %s\n", buffer);
        
        close(fd[0]);  // Cerrar lectura
        exit(0);
        
    } else {
        // PROCESO PADRE (ESCRITOR)
        close(fd[0]);  // Cerrar lectura
        
        char msg[] = "Hola desde el padre";
        write(fd[1], msg, strlen(msg) + 1);
        
        close(fd[1]);  // Cerrar escritura
        wait(NULL);    // Esperar al hijo
    }
    
    return 0;
}

/* 
Esquema de cierre: 

ESCRITOR:
- close(fd[0])  ← No necesita leer
- write(fd[1])  ← Solo escribe
- close(fd[1])  ← Cerrar cuando termine

LECTOR:
- close(fd[1])  ← No necesita escribir
- read(fd[0])   ← Solo lee
- close(fd[0])  ← Cerrar cuando termine
*/