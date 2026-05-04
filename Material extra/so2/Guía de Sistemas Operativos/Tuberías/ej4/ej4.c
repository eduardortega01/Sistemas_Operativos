#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int N;
    printf("Ingrese el número de hijos: ");
    scanf("%d", &N);

    if (N <= 0) {
        printf("El número de hijos debe ser mayor que 0.\n");
        return 1;
    }

    // Creamos N+1 pipes para formar el anillo
    int pipes[N + 1][2];
    for (int i = 0; i <= N; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Error al crear pipe");
            exit(1);
        }
    }

    pid_t pid;
    for (int i = 0; i < N; i++) {
        pid = fork();

        if (pid < 0) {
            perror("Error en fork");
            exit(1);
        } else if (pid == 0) {
            // --- Código de cada hijo ---
            char msg[100];
            close(pipes[i][1]);      // No escribe en su pipe de entrada
            close(pipes[i + 1][0]);  // No lee del siguiente pipe

            // Lee el mensaje del pipe anterior
            read(pipes[i][0], msg, sizeof(msg));
            printf("Hijo %d (PID=%d) recibió: %s\n", i + 1, getpid(), msg);

            // Envía el mensaje al siguiente pipe
            write(pipes[i + 1][1], msg, strlen(msg) + 1);

            // Cierra descriptores
            close(pipes[i][0]);
            close(pipes[i + 1][1]);
            exit(0);
        }
    }

    // --- Código del padre ---
    char mensaje[100];
    printf("Ingrese el mensaje (sin espacios): ");
    scanf("%s", mensaje);

    // Cierra pipes innecesarios
    close(pipes[0][0]);          // No lee del primero
    close(pipes[N][1]);          // No escribe en el último

    // Envía mensaje al primer hijo
    write(pipes[0][1], mensaje, strlen(mensaje) + 1);

    // Lee mensaje que regresa del último hijo
    char buffer[100];
    read(pipes[N][0], buffer, sizeof(buffer));
    printf("Padre (PID=%d) recibió nuevamente: %s\n", getpid(), buffer);

    // Cierra pipes restantes
    close(pipes[0][1]);
    close(pipes[N][0]);

    // Espera que todos los hijos terminen
    for (int i = 0; i < N; i++) {
        wait(NULL);
    }

    return 0;
}
