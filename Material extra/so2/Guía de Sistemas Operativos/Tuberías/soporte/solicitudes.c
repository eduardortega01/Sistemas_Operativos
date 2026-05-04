#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 256
#define MAX_SOLICITUDES 100

typedef struct {
    char contenido[MAX_LINE];
    char categoria[50];
    int prioridad;
} Solicitud;

// Función para el proceso hijo 1: Validación de formato y prioridad
void proceso_hijo1(int pipe_in[2], int pipe_out[2]) {
    close(pipe_in[1]); // Cerrar escritura del pipe de entrada
    close(pipe_out[0]); // Cerrar lectura del pipe de salida
    
    Solicitud sol;
    
    while(read(pipe_in[0], &sol, sizeof(Solicitud)) > 0) {
        // Verificar si contiene "REQ:" y delimitador ";"
        if(strstr(sol.contenido, "REQ:") != NULL && strchr(sol.contenido, ';') != NULL) {
            // Verificar si contiene "URGENTE"
            if(strstr(sol.contenido, "URGENTE") != NULL) {
                strcpy(sol.categoria, "[CRITICAS]");
                sol.prioridad = 3;
            } else {
                strcpy(sol.categoria, "[URGENTES]");
                sol.prioridad = 2;
            }
        } else {
            // Formato inválido
            strcpy(sol.categoria, "INVALID solicitud sin formato adecuado");
            sol.prioridad = 0;
        }
        
        // Enviar al hijo 2
        write(pipe_out[1], &sol, sizeof(Solicitud));
    }
    
    close(pipe_in[0]);
    close(pipe_out[1]);
}

// Función para el proceso hijo 2: Análisis técnico urgente
void proceso_hijo2(int pipe_in[2], int pipe_out[2]) {
    close(pipe_in[1]); // Cerrar escritura del pipe de entrada
    close(pipe_out[0]); // Cerrar lectura del pipe de salida
    
    Solicitud sol;
    
    while(read(pipe_in[0], &sol, sizeof(Solicitud)) > 0) {
        // Solo procesar si viene del hijo 1 con formato válido
        if(strcmp(sol.categoria, "INVALID solicitud sin formato adecuado") != 0) {
            // Buscar palabras críticas: servidor, bloqueo, caída
            if(strstr(sol.contenido, "servidor") != NULL || 
               strstr(sol.contenido, "bloqueo") != NULL || 
               strstr(sol.contenido, "caída") != NULL ||
               strstr(sol.contenido, "caida") != NULL) {
                // Es crítica
                strcpy(sol.categoria, "[CRITICAS]");
                sol.prioridad = 3;
            }
        }
        
        // Enviar al padre
        write(pipe_out[1], &sol, sizeof(Solicitud));
    }
    
    close(pipe_in[0]);
    close(pipe_out[1]);
}

int main(int argc, char *argv[]) {
    // Verificar argumento
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo_solicitudes>\n", argv[0]);
        exit(1);
    }

    // Pipes: [0] lectura, [1] escritura
    int pipe_padre_hijo1[2], pipe_hijo1_hijo2[2], pipe_hijo2_padre[2];
    pid_t pid1, pid2;
    
    // Crear las tuberías
    if(pipe(pipe_padre_hijo1) == -1 || pipe(pipe_hijo1_hijo2) == -1 || pipe(pipe_hijo2_padre) == -1) {
        perror("Error creando pipes");
        exit(1);
    }
    
    // Crear proceso hijo 1
    pid1 = fork();
    if(pid1 == -1) {
        perror("Error en fork hijo 1");
        exit(1);
    }
    
    if(pid1 == 0) {
        // PROCESO HIJO 1
        close(pipe_padre_hijo1[1]); // No escribe en su entrada
        close(pipe_hijo2_padre[0]); // No lee de la salida final
        close(pipe_hijo2_padre[1]); // No escribe en la salida final
        
        proceso_hijo1(pipe_padre_hijo1, pipe_hijo1_hijo2);
        exit(0);
    }
    
    // Crear proceso hijo 2
    pid2 = fork();
    if(pid2 == -1) {
        perror("Error en fork hijo 2");
        exit(1);
    }
    
    if(pid2 == 0) {
        // PROCESO HIJO 2
        close(pipe_padre_hijo1[0]); // No lee de la entrada del hijo 1
        close(pipe_padre_hijo1[1]); // No escribe en la entrada del hijo 1
        close(pipe_hijo1_hijo2[1]); // No escribe en su entrada
        
        proceso_hijo2(pipe_hijo1_hijo2, pipe_hijo2_padre);
        exit(0);
    }
    
    // PROCESO PADRE
    close(pipe_padre_hijo1[0]); // No lee de la entrada del hijo 1
    close(pipe_hijo1_hijo2[0]); // No lee del pipe intermedio
    close(pipe_hijo1_hijo2[1]); // No escribe en el pipe intermedio
    close(pipe_hijo2_padre[1]); // No escribe en la salida final
    
    // Cargar solicitudes desde archivo
    printf("=== SISTEMA DE CLASIFICACIÓN DE SOLICITUDES ===\n\n");
    
    FILE *archivo = fopen(argv[1], "r");
    if(archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(1);
    }
    
    // Leer el número de solicitudes de la primera línea
    char buffer[MAX_LINE];
    if(fgets(buffer, MAX_LINE, archivo) == NULL) {
        fprintf(stderr, "Error: archivo vacío\n");
        fclose(archivo);
        exit(1);
    }
    
    int num_solicitudes = atoi(buffer);
    
    if(num_solicitudes <= 0 || num_solicitudes > MAX_SOLICITUDES) {
        fprintf(stderr, "Error: número de solicitudes inválido (%d)\n", num_solicitudes);
        fclose(archivo);
        exit(1);
    }
    
    printf("Número de solicitudes a procesar: %d\n\n", num_solicitudes);
    
    Solicitud solicitudes[MAX_SOLICITUDES];
    int contador = 0;
    
    printf("Cargando solicitudes desde archivo...\n\n");
    while(fgets(solicitudes[contador].contenido, MAX_LINE, archivo) != NULL && 
        contador < num_solicitudes) {
        // Eliminar el salto de línea
        solicitudes[contador].contenido[strcspn(solicitudes[contador].contenido, "\n")] = 0;
        
        if(strlen(solicitudes[contador].contenido) > 0) {
            strcpy(solicitudes[contador].categoria, "");
            solicitudes[contador].prioridad = 0;
            contador++;
        }
    }
    
    fclose(archivo);
    printf("Se cargaron %d solicitudes del archivo.\n\n", contador);
    
    // Enviar solicitudes al hijo 1
    for(int i = 0; i < contador; i++) {
        printf("Enviando: %s\n", solicitudes[i].contenido);
        write(pipe_padre_hijo1[1], &solicitudes[i], sizeof(Solicitud));
    }
    
    close(pipe_padre_hijo1[1]); // Terminar de enviar
    
    // Recibir solicitudes clasificadas del hijo 2
    printf("\n=== SOLICITUDES CLASIFICADAS ===\n\n");
    
    Solicitud sol_clasificada;
    int criticas = 0, urgentes = 0, baja_prioridad = 0, invalidas = 0;
    
    while(read(pipe_hijo2_padre[0], &sol_clasificada, sizeof(Solicitud)) > 0) {
        printf("Categoría: %s\n", sol_clasificada.categoria);
        printf("Contenido: %s\n", sol_clasificada.contenido);
        printf("Prioridad: %d\n", sol_clasificada.prioridad);
        
        if(strstr(sol_clasificada.categoria, "CRITICAS")) criticas++;
        else if(strstr(sol_clasificada.categoria, "URGENTES")) urgentes++;
        else if(strstr(sol_clasificada.categoria, "INVALID")) invalidas++;
        else baja_prioridad++;
    }
    
    close(pipe_hijo2_padre[0]);
    
    // Esperar a que terminen los hijos
    wait(NULL);
    wait(NULL);
    
    // Imprimir reporte final
    printf("\n=== REPORTE FINAL ===\n");
    printf("Solicitudes CRÍTICAS: %d\n", criticas);
    printf("Solicitudes URGENTES: %d\n", urgentes);
    printf("Solicitudes BAJA PRIORIDAD: %d\n", baja_prioridad);
    printf("Solicitudes INVÁLIDAS: %d\n", invalidas);
    printf("Total procesadas: %d\n", contador);
    
    return 0;
}