#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// P1 -> P2 -> P3
int main() {
    int pipe1[2], pipe2[2];
    
    pipe(pipe1);  // P1 -> P2
    pipe(pipe2);  // P2 -> P3
    
    // Crear P2
    if (fork() == 0) {
        close(pipe1[1]);  // P2 no escribe en pipe1
        close(pipe2[0]);  // P2 no lee de pipe2
        
        int n;
        read(pipe1[0], &n, sizeof(int));
        n = n + 10;  // P2 suma 10
        write(pipe2[1], &n, sizeof(int));
        
        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);
    }
    
    // Crear P3
    if (fork() == 0) {
        close(pipe1[0]);  // P3 no usa pipe1
        close(pipe1[1]);
        close(pipe2[1]);  // P3 no escribe en pipe2
        
        int n;
        read(pipe2[0], &n, sizeof(int));
        printf("P3 recibió: %d\n", n);
        
        close(pipe2[0]);
        exit(0);
    }
    
    // P1 (padre)
    close(pipe1[0]);  // P1 no lee de pipe1
    close(pipe2[0]);  // P1 no usa pipe2
    close(pipe2[1]);
    
    int inicio = 5;
    write(pipe1[1], &inicio, sizeof(int));
    close(pipe1[1]);
    
    wait(NULL);
    wait(NULL);
    
    return 0;
}