#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

pid_t router, receptor1, receptor2; // PIDs de procesos globales

// Variables globales para los receptores
char letra_actual = 0;  // reconstrucción de una letra bit a bit
int bit_pos = 7;        // posición del bit que se está llenando (7 a 0)

// ---------------- FUNCIONES DE LOS RECEPTORES ----------------

// Handler cuando llega una señal SIGUSR1 -> bit = 1
void bit1_handler(int sig) {
    letra_actual |= (1 << bit_pos); // encender bit correspondiente
    bit_pos--;
}

// Handler cuando llega una señal SIGUSR2 -> bit = 0
void bit0_handler(int sig) {
    bit_pos--;
}

// Handler cuando se recibe señal SIGINT -> fin de una letra
void fin_letra_handler(int sig) {
    if (bit_pos < 0) { // si ya se recibieron los 8 bits
        printf("%c", letra_actual);
        fflush(stdout);
        letra_actual = 0;
        bit_pos = 7; // reiniciar para siguiente letra
    }
}

// Handler para terminar la recepción
void fin_transmision_handler(int sig) {
    printf("\nTransmisión finalizada.\n");
    exit(0);
}

// ---------------- FUNCIONES DEL ROUTER ----------------

// Función para enviar una letra bit por bit a un receptor
void enviar_letra(pid_t receptor, char c) {
    for (int i = 7; i >= 0; i--) {
        if (c & (1 << i))
            kill(receptor, SIGUSR1); // bit 1
        else
            kill(receptor, SIGUSR2); // bit 0
        usleep(5000); // pequeña pausa para no saturar canal
    }
    kill(receptor, SIGINT); // fin de letra
    usleep(10000); // tiempo entre letras
}

// Función para leer líneas pares
char **read_lines_pairs(const char *filename, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    char **pairs = NULL;
    char buffer[1024];
    *count = 0;
    int line_number = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        line_number++;
        buffer[strcspn(buffer, "\n")] = '\0';
        if (line_number % 2 == 0) { // líneas pares
            pairs = realloc(pairs, (*count + 1) * sizeof(char *));
            pairs[*count] = strdup(buffer);
            (*count)++;
        }
    }
    fclose(file);
    return pairs;
}

// Función para leer líneas impares
char **read_lines_odds(const char *filename, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    char **odds = NULL;
    char buffer[1024];
    *count = 0;
    int line_number = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        line_number++;
        buffer[strcspn(buffer, "\n")] = '\0';
        if (line_number % 2 != 0) { // líneas impares
            odds = realloc(odds, (*count + 1) * sizeof(char *));
            odds[*count] = strdup(buffer);
            (*count)++;
        }
    }
    fclose(file);
    return odds;
}

// ---------------- PROGRAMA PRINCIPAL ----------------
int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo>\n", argv[0]);
        exit(1);
    }

    router = getpid(); // guardar PID del proceso principal

    // Leer archivo
    int num_pares, num_impares;
    char **pares = read_lines_pairs(argv[1], &num_pares);
    char **impares = read_lines_odds(argv[1], &num_impares);

    // Crear primer receptor
    receptor1 = fork();
    if (receptor1 == -1) {
        perror("Error al crear receptor 1");
        exit(1);
    }

    if (receptor1 == 0) {
        // Código del receptor 1 (líneas pares)
        signal(SIGUSR1, bit1_handler);
        signal(SIGUSR2, bit0_handler);
        signal(SIGINT, fin_letra_handler);
        signal(SIGTERM, fin_transmision_handler);
        printf("Receptor 1 listo (pares):\n");
        while (1) pause();
    }

    // Crear segundo receptor
    receptor2 = fork();
    if (receptor2 == -1) {
        perror("Error al crear receptor 2");
        exit(1);
    }

    if (receptor2 == 0) {
        // Código del receptor 2 (líneas impares)
        signal(SIGUSR1, bit1_handler);
        signal(SIGUSR2, bit0_handler);
        signal(SIGINT, fin_letra_handler);
        signal(SIGTERM, fin_transmision_handler);
        printf("Receptor 2 listo (impares):\n");
        while (1) pause();
    }

    // Código del router (padre)
    sleep(1); // espera para que los hijos instalen sus handlers

    printf("\n=== Iniciando transmisión ===\n");

    // Enviar las líneas pares al receptor1
    for (int i = 0; i < num_pares; i++) {
        for (int j = 0; j < strlen(pares[i]); j++) {
            enviar_letra(receptor1, pares[i][j]);
        }
        enviar_letra(receptor1, '\n'); // espacio entre palabras
    }
    kill(receptor1, SIGTERM); // fin transmisión receptor1

    // Enviar las líneas impares al receptor2
    for (int i = 0; i < num_impares; i++) {
        for (int j = 0; j < strlen(impares[i]); j++) {
            enviar_letra(receptor2, impares[i][j]);
        }
        enviar_letra(receptor2, '\n');
    }
    kill(receptor2, SIGTERM); // fin transmisión receptor2

    // Esperar que terminen los receptores
    wait(NULL);
    wait(NULL);

    printf("\n=== Transmisión completa ===\n");

    return 0;
}


/*
 * (1 << bit_pos) es una operación de desplazamiento de bits.
 * El número 1 en binario es 00000001.
 * Cuando hacemos << bit_pos, estamos "moviendo" ese 1 hacia la izquierda.
 * Por ejemplo:
 *   bit_pos = 0 → (1 << 0) = 00000001  → representa el bit menos significativo.
 *   bit_pos = 1 → (1 << 1) = 00000010
 *   bit_pos = 2 → (1 << 2) = 00000100
 * Esto sirve para “encender” (poner en 1) un bit específico dentro de un byte.
 */

/*
 * El operador OR “|=” coloca el bit en 1 en la posición indicada sin alterar los demás.
 * Ejemplo:
 * letra_actual = 00000000
 * (1 << 2) = 00000100
 * Resultado: letra_actual = 00000100
 */