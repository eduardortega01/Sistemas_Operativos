#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 1024 // tamaño maxímo por línea
#define MAX_SOLICITUDES 100 // tamaño maxímo de solicitudes

// Estructura para las solicitudes 
typedef struct {
    char contenido[MAX_LINE]; 
    char categoria[50]; // [CRÍTICAS], [URGENTES], [BAJA PRIORIDAD]
} Solicitud;

// Función para el proceso hijo 1: Validación de formato
void proceso_hijo1(int pipe1[2], int pipe2[2], int pipe3[2]) {
    close(pipe1[1]); // padre_hijo1 - cerrar escritura
    close(pipe2[0]); // hijo1_hijo2 - cerrar lectura
    close(pipe3[0]); // hijo1_padre - cerrar lectura

    Solicitud sol; 

    while(read(pipe1[0], &sol, sizeof(Solicitud)) > 0) {
        // verificar si contiene "REQ:", delimitador ";" y contiene URGENTE
        if (strncmp(sol.contenido, "REQ:", 4) == 0 && strchr(sol.contenido, ';') != NULL && strstr(sol.contenido, "URGENTE") != NULL) {
            // Tiene formato válido y URGENTE -> enviar a hijo2 para análisis técnico
            write(pipe2[1], &sol, sizeof(Solicitud));
        } else if (strncmp(sol.contenido, "REQ:", 4) == 0 && strchr(sol.contenido, ';') != NULL) { 
            // Tiene formato válido pero NO urgente -> baja prioridad
            strcpy(sol.categoria, "[BAJA PRIORIDAD]"); 
            write(pipe3[1], &sol, sizeof(Solicitud));
        } else {
            // Formato inválido
            strcpy(sol.categoria, "[BAJA PRIORIDAD]");
            write(pipe3[1], &sol, sizeof(Solicitud));
        }
    }

    close(pipe1[0]);
    close(pipe2[1]);
    close(pipe3[1]); 
}

void proceso_hijo2(int pipe1[2], int pipe2[2]){
    close(pipe1[1]); // hijo1_hijo2 - cerrar escritura 
    close(pipe2[0]); // hijo2_padre - cerrar lectura 

    Solicitud soli; 

    while (read(pipe1[0], &soli, sizeof(Solicitud)) > 0){
        // verificar si contiene palabras claves críticas como: servidor, bloqueo, caída
        if (strstr(soli.contenido, "servidor") != NULL || strstr(soli.contenido, "bloqueo") != NULL || strstr(soli.contenido, "caída") != NULL) {
            strcpy(soli.categoria, "[CRÍTICAS]"); 
            write(pipe2[1], &soli, sizeof(Solicitud));
        } else {
            strcpy(soli.categoria, "[URGENTES]");
            write(pipe2[1], &soli, sizeof(Solicitud));
        }
    }

    close(pipe1[0]);
    close(pipe2[1]); 
}

int main(int argc, char *argv[]){

    // Verificar argumento
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo_solicitudes>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t hijo1, hijo2;

    // pipes 
    int padre_hijo1[2], hijo1_hijo2[2], hijo2_padre[2], hijo1_padre[2];
    if (pipe(padre_hijo1) == -1 || pipe(hijo1_hijo2) == -1 || pipe(hijo2_padre) == -1 || pipe(hijo1_padre) == -1) {
        perror("Error creando pipes\n");
        exit(EXIT_FAILURE);
    } 

    hijo1 = fork();
    if (hijo1 == -1) {
        perror("Error creando hijo1\n");
        exit(EXIT_FAILURE);
    }

    if (!hijo1) { // hijo1

        close(hijo2_padre[0]);
        close(hijo2_padre[1]);

        proceso_hijo1(padre_hijo1, hijo1_hijo2, hijo1_padre); 
        exit(EXIT_SUCCESS);
    }

    hijo2 = fork();
    if (hijo2 == -1) {
        perror("Error creando hijo2\n");
        exit(EXIT_FAILURE); 
    }

    if (!hijo2) { // hijo 2
        close(padre_hijo1[0]); 
        close(padre_hijo1[1]);
        close(hijo1_padre[0]);
        close(hijo1_padre[1]);

        proceso_hijo2(hijo1_hijo2, hijo2_padre); 
        exit(EXIT_SUCCESS);
    }

    // Padre
    close(padre_hijo1[0]); // no lee la entrada de hijo 1
    close(hijo1_hijo2[0]); 
    close(hijo1_hijo2[1]);
    close(hijo2_padre[1]); // no escribe a hijo 2
    close(hijo1_padre[1]);  // no escribe directamente desde hijo 1

    FILE *archivo = fopen(argv[1], "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo\n");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_LINE]; 
    if (fgets(buffer, MAX_LINE, archivo) == NULL) {
        fprintf(stderr, "Error: archivo vacío\n"); 
        fclose(archivo);
        exit(EXIT_FAILURE); 
    }

    int num_solicitudes = atoi(buffer);

    if (num_solicitudes <= 0 || num_solicitudes > MAX_SOLICITUDES) {
        fprintf(stderr, "Error: número de solicitudes inválido (%d)\n", num_solicitudes);
    }

    Solicitud solicitudes[num_solicitudes]; 
    int contador = 0;

    while (fgets(solicitudes[contador].contenido, MAX_LINE, archivo) != NULL && contador < num_solicitudes) {
        // Eliminar salto de línea
        solicitudes[contador].contenido[strcspn(solicitudes[contador].contenido, "\n")] = 0;

        if(strlen(solicitudes[contador].contenido) > 0) {
            strcpy(solicitudes[contador].categoria, "");
            contador++;
        }
    }

    fclose(archivo);

    // enviar solicitudes al hijo 1
    for (int i = 0; i < contador; i++) {
        printf("Enviando: %s\n", solicitudes[i].contenido);
        write(padre_hijo1[1], &solicitudes[i], sizeof(Solicitud));
    }

    close(padre_hijo1[1]); // cerrar escritura para indicar que terminó

    // Recibir resultados clasificados 
    Solicitud solis[num_solicitudes];
    int conta = 0;

    // leer de hijo1_padre (baja prioridad e invaálidas)
    Solicitud temp;
    while(read(hijo1_padre[0], &temp, sizeof(Solicitud)) > 0){
        solis[conta] = temp;
        conta++; 
    }

    // leer de hijo2_padre (críticas y urgentes)
    while(read(hijo2_padre[0], &temp, sizeof(Solicitud)) > 0){
        solis[conta] = temp;
        conta++;
    }

    
    close(hijo1_padre[0]);
    close(hijo2_padre[0]);
    
    // Esperar a que terminen los hijos
    wait(NULL);
    wait(NULL);

    // Imprimir por categoría - Reporte final
    printf("[CRÍTICAS]\n");
    for (int i = 0; i < conta; i++) {
        if (strstr(solis[i].categoria, "[CRÍTICAS]") != NULL) {
            printf("%s\n", solis[i].contenido);
        }
    }
    
    printf("[URGENTES]\n");
    for (int i = 0; i < conta; i++) {
        if (strstr(solis[i].categoria, "[URGENTES]") != NULL) {
            printf("%s\n", solis[i].contenido);
        }
    }

    printf("[BAJA PRIORIDAD]\n");
    for (int i = 0; i < conta; i++) {
        if (strstr(solis[i].categoria, "[BAJA PRIORIDAD]") != NULL) {
            printf("%s\n", solis[i].contenido);
        }
    }

    return 0;
}