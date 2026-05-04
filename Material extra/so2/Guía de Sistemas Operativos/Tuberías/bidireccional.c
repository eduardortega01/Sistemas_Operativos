#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int pipe1[2];  // Padre -> Hijo
    int pipe2[2];  // Hijo -> Padre
    
    pipe(pipe1);
    pipe(pipe2);
    
    if (fork() == 0) {
        // HIJO
        close(pipe1[1]);  // No escribe en pipe1
        close(pipe2[0]);  // No lee de pipe2
        
        // Recibir del padre
        int n;
        read(pipe1[0], &n, sizeof(int));
        printf("Hijo recibió: %d\n", n);
        
        // Enviar al padre
        n = n * 2;
        write(pipe2[1], &n, sizeof(int));
        
        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);
        
    } else {
        // PADRE
        close(pipe1[0]);  // No lee de pipe1
        close(pipe2[1]);  // No escribe en pipe2
        
        // Enviar al hijo
        int numero = 5;
        write(pipe1[1], &numero, sizeof(int));
        close(pipe1[1]);
        
        // Recibir del hijo
        int resultado;
        read(pipe2[0], &resultado, sizeof(int));
        printf("Padre recibió: %d\n", resultado);
        
        close(pipe2[0]);
        wait(NULL);
    }
    
    return 0;
}

/* 
           pipe1 (Padre->Hijo)
    PADRE ===================> HIJO
    PADRE <=================== HIJO
           pipe2 (Hijo->Padre)
*/