#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int main() {
    int n = 2; 
    int pipes[n*4][2]; 
    pid_t h1, h2, h11, h21;
    char buffer[BUFFER_SIZE];

    // Crear tuberías
    for (int i = 0; i < (n*2); i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Error al crear pipe");
            exit(1);
        }
    }

    
    // ---- PADRE: envía cadenas ----
    printf("Ingrese cadenas de texto (escriba FIN para terminar):\n");

    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (strcmp(buffer, "FIN\n") == 0) {
            // Enviar "FIN" a todos los hijos
            for (int i = 0; i < NUM_HIJOS; i++)
                write(pipes[i][1], buffer, strlen(buffer));
            break;
        }

        // Enviar cada línea a todos los hijos
        for (int i = 0; i < NUM_HIJOS; i++)
            write(pipes[i][1], buffer, strlen(buffer));
    }

    // Cerrar pipes del padre
    for (int i = 0; i < NUM_HIJOS; i++)
        close(pipes[i][1]);

    // Esperar a los hijos
    for (int i = 0; i < NUM_HIJOS; i++)
        wait(NULL);

    printf("\nTodos los hijos han terminado.\n");
    return 0;
}
