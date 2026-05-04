#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// Función para leer todas las líneas (comandos) de un archivo
char **read_lines(const char *filename, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    char **commands = NULL;   // Arreglo dinámico de punteros a comandos
    char buffer[512];         // Búfer temporal para cada línea
    *count = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Eliminar salto de línea final si existe
        buffer[strcspn(buffer, "\n")] = '\0';

        // Redimensionar arreglo para agregar un nuevo comando
        commands = realloc(commands, (*count + 1) * sizeof(char *));
        if (!commands) {
            perror("Error al asignar memoria");
            exit(1);
        }

        // Reservar espacio exacto para la línea leída
        commands[*count] = malloc(strlen(buffer) + 1);
        if (!commands[*count]) {
            perror("Error al asignar memoria para línea");
            exit(1);
        }

        strcpy(commands[*count], buffer); // Copiar el comando
        (*count)++;
    }

    fclose(file);
    return commands;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo_comandos>\n", argv[0]);
        exit(1);
    }

    int n; // número de comandos
    char **commands = read_lines(argv[1], &n);

    if (n == 0) {
        printf("El archivo está vacío.\n");
        return 0;
    }

    int pipes[n][2]; // un pipe por comando

    // Crear las tuberías antes de hacer los fork
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Error al crear pipe");
            exit(1);
        }
    }

    // Crear procesos hijos
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Error en fork");
            exit(1);
        } 
        else if (pid == 0) {
            // --- PROCESO HIJO ---
            close(pipes[i][0]); // Cierra el extremo de lectura

            // Usamos popen() para ejecutar el comando y leer su salida
            FILE *fp = popen(commands[i], "r");
            if (!fp) {
                perror("Error al ejecutar comando");
                exit(1);
            }

            // Leer la salida del comando y escribirla al pipe
            char buffer[512];
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                write(pipes[i][1], buffer, strlen(buffer));
            }

            pclose(fp);
            close(pipes[i][1]);
            exit(0);
        } 
        else {
            // --- PROCESO PADRE ---
            close(pipes[i][1]); // Cierra el extremo de escritura
        }
    }

    // --- PROCESO PADRE: lee resultados ---
    for (int i = 0; i < n; i++) {
        char buffer[512];
        ssize_t bytesRead;

        printf("\n---- Salida del comando [%s] ----\n", commands[i]);
        while ((bytesRead = read(pipes[i][0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            printf("%s", buffer);
        }
        close(pipes[i][0]);
    }

    // Esperar a que terminen todos los hijos
    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    printf("\nTodos los comandos han sido ejecutados.\n");

    // Liberar memoria
    for (int i = 0; i < n; i++) {
        free(commands[i]);
    }
    free(commands);

    return 0;
}
