#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

// Función para leer archivo
char **read_lines(const char *filename, int *count) {
    FILE *archivo = fopen(filename, "r");
    if (!archivo) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    char **palabras = NULL;
    char buffer[BUFFER_SIZE];
    *count = 0;

    while (fgets(buffer, sizeof(buffer), archivo)) {
        buffer[strcspn(buffer, "\n")] = '\0'; // eliminar salto de línea
        palabras = realloc(palabras, (*count + 1) * sizeof(char *));
        palabras[*count] = malloc(strlen(buffer) + 1);
        strcpy(palabras[*count], buffer);
        (*count)++;
    }
    fclose(archivo);
    return palabras;
}

// Mostrar árbol de procesos
void print_debug_tree() {
    char cmd[64];
    sprintf(cmd, "pstree -lp %d", getpid());
    system(cmd);
}

// PROGRAMA PRINCIPAL
int main() {

    int hijo1_padre[2];
    int padre_hijo2[2];
    int hijo2_padre[2];

    pipe(hijo1_padre);
    pipe(padre_hijo2);
    pipe(hijo2_padre);

    pid_t hijo1 = fork();
    if (hijo1 == 0) {
        // ========= HIJO 1 =========
        close(hijo1_padre[0]); // solo escribe
        close(padre_hijo2[0]); close(padre_hijo2[1]);
        close(hijo2_padre[0]); close(hijo2_padre[1]);

        int n;
        char **palabras = read_lines("palabras.txt", &n);
        for (int i = 0; i < n; i++) {
            write(hijo1_padre[1], palabras[i], strlen(palabras[i]) + 1);
        }

        close(hijo1_padre[1]);
        exit(0);
    }

    pid_t hijo2 = fork();
    if (hijo2 == 0) {
        // ========= HIJO 2 =========
        close(padre_hijo2[1]);
        close(hijo2_padre[0]);
        close(hijo1_padre[0]); close(hijo1_padre[1]);

        char buffer[BUFFER_SIZE];
        ssize_t bytes;

        bytes = read(padre_hijo2[0], buffer, sizeof(buffer));
        printf("Hijo2 [%d] recibe de padre[%d]: %s\n", getpid(), getppid(),  buffer);

        // ======== Crear NIETO 2 ========
        int hijo2_nieto2[2], nieto2_hijo2[2];
        pipe(hijo2_nieto2);
        pipe(nieto2_hijo2);

        pid_t nieto2 = fork();
        if (nieto2 == 0) {
            // ------- NIETO 2 -------
            close(hijo2_nieto2[1]); // solo lee
            close(nieto2_hijo2[0]); // solo escribe

            char buffer_n2[BUFFER_SIZE];
            read(hijo2_nieto2[0], buffer_n2, sizeof(buffer_n2));
            printf("Nieto2 [%d] recibe de hijo 2[%d]: %s\n", getpid(), getppid(), buffer_n2);

            write(nieto2_hijo2[1], buffer_n2, strlen(buffer_n2) + 1);

            close(hijo2_nieto2[0]);
            close(nieto2_hijo2[1]);
            exit(0);
        }

        // --- Hijo2 envía mensaje a nieto2 ---
        write(hijo2_nieto2[1], buffer, bytes);
        close(hijo2_nieto2[1]);

        // --- Hijo2 recibe respuesta de nieto2 ---
        bytes = read(nieto2_hijo2[0], buffer, sizeof(buffer));
        close(nieto2_hijo2[0]);
        wait(NULL);

        printf("Hijo2[%d] recibe del nieto2[%d]: %s\n", getpid(), nieto2, buffer);

        // ======== Crear NIETO 1 ========
        int hijo2_nieto1[2], nieto1_hijo2[2];
        pipe(hijo2_nieto1);
        pipe(nieto1_hijo2);

        pid_t nieto1 = fork();
        if (nieto1 == 0) {
            // ------- NIETO 1 -------
            close(hijo2_nieto1[1]);
            close(nieto1_hijo2[0]);

            char buffer_n1[BUFFER_SIZE];
            read(hijo2_nieto1[0], buffer_n1, sizeof(buffer_n1));
            printf("Nieto1 [%d] recibe de hijo 2[%d]: %s\n", getpid(), getppid(), buffer_n1);

            write(nieto1_hijo2[1], buffer_n1, strlen(buffer_n1) + 1);

            close(hijo2_nieto1[0]);
            close(nieto1_hijo2[1]);
            exit(0);
        }

        // --- Hijo2 envía a nieto1 ---
        write(hijo2_nieto1[1], buffer, bytes);
        close(hijo2_nieto1[1]);

        // --- Hijo2 recibe respuesta de nieto1 ---
        bytes = read(nieto1_hijo2[0], buffer, sizeof(buffer));
        close(nieto1_hijo2[0]);
        wait(NULL);

        printf("Hijo2[%d] recibe del nieto1[%d]: %s\n",getpid(), nieto1,  buffer);

        // --- Enviar de vuelta al padre ---
        write(hijo2_padre[1], buffer, strlen(buffer) + 1);
        close(hijo2_padre[1]);
        exit(0);
    }

    // ========= PADRE =========
    close(hijo1_padre[1]);
    close(padre_hijo2[0]);
    close(hijo2_padre[1]);

    char buffer[BUFFER_SIZE];
    ssize_t bytes;

    bytes = read(hijo1_padre[0], buffer, sizeof(buffer));
    close(hijo1_padre[0]);

    printf("Padre[%d] recibe de hijo1[%d]: %s\n", getpid(), hijo1, buffer);

    // Enviar al hijo2
    write(padre_hijo2[1], buffer, bytes);
    close(padre_hijo2[1]);

    // Esperar respuesta final
    bytes = read(hijo2_padre[0], buffer, sizeof(buffer));
    close(hijo2_padre[0]);

    printf("Padre[%d] recibe respuesta final hijo2[%d]: %s\n", getpid(), hijo2, buffer);

    wait(NULL);
    wait(NULL);

    printf("\nSecuencia completada correctamente \n");
    return 0;
}
