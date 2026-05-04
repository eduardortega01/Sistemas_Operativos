#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MSG_SIZE 256

int main() {
    // 4 tuberías para conectar en el orden:
    // Padre -> Hijo1 -> Hijo2 -> Nieto1 -> Nieto2
    int pipes[4][2];
    for (int i = 0; i < 4; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Error creando pipe");
            exit(1);
        }
    }

    pid_t hijo1, hijo2;

    // Crear Hijo1
    hijo1 = fork();
    if (hijo1 == 0) {
        // === Código del HIJO 1 ===
        
        // Crear NIETO1 PRIMERO (hijo del hijo1)
        pid_t nieto1 = fork();
        if (nieto1 == 0) {
            // === Código del NIETO 1 ===
            // Cierra TODAS las pipes excepto las que usa
            close(pipes[0][0]); 
            close(pipes[0][1]);
            close(pipes[1][0]); 
            close(pipes[1][1]);
            
            close(pipes[2][1]); // leerá del hijo2
            close(pipes[3][0]); // escribirá al nieto2

            char msg[MSG_SIZE];
            while (read(pipes[2][0], msg, sizeof(msg)) > 0) {
                printf("Nieto1 (PID %d) recibió: %s\n", getpid(), msg);
                fflush(stdout);
                write(pipes[3][1], msg, strlen(msg) + 1);
            }

            close(pipes[2][0]);
            close(pipes[3][1]);
            exit(0);
        }

        // === Código del hijo1 continúa ===
        // Ahora cierra lo que no usa
        close(pipes[0][1]); // Lee del padre
        close(pipes[1][0]); // Escribe al hijo2

        // Cierra las demás pipes
        close(pipes[2][0]); 
        close(pipes[2][1]);
        close(pipes[3][0]); 
        close(pipes[3][1]);

        char msg[MSG_SIZE];
        while (read(pipes[0][0], msg, sizeof(msg)) > 0) {
            printf("Hijo1 (PID %d) recibió: %s\n", getpid(), msg);
            fflush(stdout);
            write(pipes[1][1], msg, strlen(msg) + 1);
        }

        close(pipes[0][0]);
        close(pipes[1][1]);
        wait(NULL); // espera a su hijo (nieto1)
        exit(0);
    }

    // Crear Hijo2
    hijo2 = fork();
    if (hijo2 == 0) {
        // === Código del HIJO 2 ===
        
        // Crear NIETO2 PRIMERO (hijo del hijo2)
        pid_t nieto2 = fork();
        if (nieto2 == 0) {
            // === Código del NIETO 2 ===
            // Cierra TODAS las pipes excepto las que usa
            close(pipes[0][0]); 
            close(pipes[0][1]);
            close(pipes[1][0]); 
            close(pipes[1][1]);
            close(pipes[2][0]); 
            close(pipes[2][1]);
            
            close(pipes[3][1]); // leerá del nieto1

            char msg[MSG_SIZE];
            while (read(pipes[3][0], msg, sizeof(msg)) > 0) {
                printf("Nieto2 (PID %d) recibió: %s\n", getpid(), msg);
                fflush(stdout);
            }

            close(pipes[3][0]);
            exit(0);
        }

        // === Código del hijo2 continúa ===
        // Ahora cierra lo que no usa
        close(pipes[1][1]); // leerá de hijo1
        close(pipes[2][0]); // escribirá al nieto1

        // Cierra las demás pipes
        close(pipes[0][0]); 
        close(pipes[0][1]);
        close(pipes[3][0]); 
        close(pipes[3][1]);

        char msg[MSG_SIZE];
        while (read(pipes[1][0], msg, sizeof(msg)) > 0) {
            printf("Hijo2 (PID %d) recibió: %s\n", getpid(), msg);
            fflush(stdout);
            write(pipes[2][1], msg, strlen(msg) + 1);
        }

        close(pipes[1][0]);
        close(pipes[2][1]);
        wait(NULL); // espera al nieto2
        exit(0);
    }

    // === Código del PADRE ===
    close(pipes[0][0]); // escribe al hijo1
    
    // Cierra todas las demás pipes
    close(pipes[1][0]); 
    close(pipes[1][1]);
    close(pipes[2][0]); 
    close(pipes[2][1]);
    close(pipes[3][0]); 
    close(pipes[3][1]);

    char msg[MSG_SIZE];
    while (1) {
        printf("Ingrese un mensaje (Fin para terminar): ");
        fflush(stdout);
        
        if (fgets(msg, sizeof(msg), stdin) == NULL) break;
        
        // Eliminar el salto de línea al final
        msg[strcspn(msg, "\n")] = '\0';
        
        if (strcmp(msg, "Fin") == 0) break;
        if (strlen(msg) == 0) continue; // Ignora líneas vacías

        printf("Padre (PID %d) envió: %s\n", getpid(), msg);
        fflush(stdout);
        write(pipes[0][1], msg, strlen(msg) + 1);
        
        // Pequeña pausa para que los hijos procesen antes del siguiente prompt
        usleep(100000); // 100ms
    }

    close(pipes[0][1]);
    
    // Espera a sus dos hijos directos
    wait(NULL);
    wait(NULL);
    
    printf("Padre finaliza.\n");
    return 0;
}