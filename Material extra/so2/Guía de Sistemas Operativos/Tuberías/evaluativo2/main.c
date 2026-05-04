#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int n;
    printf("Ingrese el número de hijos: ");
    scanf("%d", &n);
    getchar(); // limpiar buffer del salto de línea

    char mensaje[100];
    printf("Ingrese el mensaje a enviar: ");
    fgets(mensaje, sizeof(mensaje), stdin);
    mensaje[strcspn(mensaje, "\n")] = '\0'; // eliminar salto de línea

    int pipes[n][2]; // tuberías entre procesos
    pid_t pids[n];

    // Crear todas las tuberías
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Error al crear pipe");
            exit(1);
        }
    }

    // Crear procesos hijo
    for (int i = 0; i < n; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("Error al crear hijo");
            exit(1);
        }

        if (pids[i] == 0) { // Proceso hijo i
            char buffer[100];

            // Cerrar todos los pipes que no voy a usar
            for (int j = 0; j < n; j++) {
                if (j == i) {
                    // Este hijo lee del pipe i
                    close(pipes[j][1]); // cerrar escritura
                } else if (i < n - 1 && j == i + 1) {
                    // Si no soy el último, escribo al pipe i+1
                    close(pipes[j][0]); // cerrar lectura
                } else {
                    // Cerrar pipes que no uso
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }

            // Leer mensaje desde mi pipe
            read(pipes[i][0], buffer, sizeof(buffer));
            close(pipes[i][0]);

            printf("Hijo %d (PID %d) recibió: %s\n", i + 1, getpid(), buffer);

            // Si no es el último, reenviar al siguiente
            if (i < n - 1) {
                write(pipes[i + 1][1], buffer, strlen(buffer) + 1);
                close(pipes[i + 1][1]);
            }

            exit(0); // salir del hijo
        }
    }

    // Padre: cerrar todos los pipes excepto el primero para escribir
    for (int i = 0; i < n; i++) {
        if (i == 0) {
            close(pipes[i][0]); // solo mantener escritura del primero
        } else {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
    }

    // Escribir el mensaje al primer hijo
    write(pipes[0][1], mensaje, strlen(mensaje) + 1);
    close(pipes[0][1]);

    // Esperar a que terminen los hijos
    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    printf("Padre (PID %d) ha terminado.\n", getpid());
    return 0;
}