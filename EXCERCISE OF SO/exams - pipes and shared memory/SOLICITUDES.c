#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

#define MAX_LEN 125

// Estructura para enviar mensajes con prioridad
typedef struct {
    int type; // 0 = urgente, 1 = crítico
    char text[MAX_LEN]; // contenido del mensaje
} Message;

void showtree(); // Muestra el árbol de procesos
void read_file(const char *file, char ***vec, int *size); // Lee archivo a un vector de strings

int main(int argc, char **argv){
    // Validación de argumentos
    if(argc < 2){
        perror("Send file\n");
        return EXIT_FAILURE;
    }

    pid_t root = getpid(); // Guarda el PID del proceso padre

    int n_children = 2, child_id; // Número de hijos
    int n_pipes = 4;

    // Arreglo de pipes → cada uno tiene [lectura, escritura]
    int pipes[n_pipes][2];

    // Creación de pipes ANTES del fork (importante para herencia)
    for(int i=0; i < n_pipes; i++){
        pipe(pipes[i]);
    }

    // Creación de procesos hijos
    for(child_id=0; child_id < n_children; child_id++){
        if(!fork()) break; // El hijo sale del ciclo, el padre sigue creando
    }

    // ===================== PROCESO PADRE =====================
    if(root == getpid()){
        showtree(); // Visualiza el árbol de procesos

        char buff[MAX_LEN], results[MAX_LEN];
        char **vec;
        int size;
        Message msg;
        
        // Cierre de extremos de pipes que NO usa el padre
        for(int i=0; i < n_pipes; i++){
            if(i != 0) close(pipes[i][1]); // Solo escribe en pipe 0
            if(i != 1 && i != 3) close(pipes[i][0]); // Solo lee de pipes 1 y 3
        }

        // Lectura del archivo
        read_file(argv[1], &vec, &size);
        
        // Envío de mensajes al hijo 0
        for(int i=0; i < size; i++){
            memset(buff, 0, MAX_LEN);
            strncpy(buff, vec[i], MAX_LEN);
            printf("Parent sending: %s\n", buff);

            // Escritura en pipe[0] → comunicación padre → hijo 0
            write(pipes[0][1], buff, MAX_LEN);
        }

        // Cierre importante → indica fin de datos (EOF)
        close(pipes[0][1]);

        // Espera a que los hijos terminen
        for(int i=0; i < n_children; i++) wait(NULL);

        // Lectura de mensajes de baja prioridad
        printf("\n[LOW PRIORITY]:\n");
        while(read(pipes[1][0], results, MAX_LEN) > 0){
            printf("%s\n", results);
        }
        close(pipes[1][0]);

        // Lectura de mensajes procesados (urgente/crítico)
        while(read(pipes[3][0], &msg, sizeof(Message)) > 0){
            if(msg.type){
                printf("[CRITICAL]\n%s\n\n", msg.text);
            }else{
                printf("[URGENT]\n%s\n\n", msg.text);
            }
        }
        close(pipes[3][0]);

        // Liberación de memoria
        for(int i=0; i < size; i++) free(vec[i]);
        free(vec);

    // ===================== PROCESOS HIJOS =====================
    }else{
        sleep(1); // Pequeña sincronización

        char buff[MAX_LEN];

        // -------- HIJO 0: CLASIFICADOR --------
        if(child_id == 0){

            // Configuración de pipes:
            // Lee de pipe 0 (padre)
            // Escribe en pipe 1 (baja prioridad) y pipe 2 (alta prioridad)
            for(int i=0; i < n_pipes; i++){
                if(i != 0) close(pipes[i][0]);
                if(i != 1 && i != 2) close(pipes[i][1]);
            }

            // Lectura de mensajes enviados por el padre
            while(read(pipes[0][0], buff, MAX_LEN) > 0){
                printf("Child %d, recived: %s\n", child_id, buff);

                // Clasificación del mensaje
                if((strncmp(buff, "REQ:", 4) == 0 && strchr(buff, ';') != NULL)
                    && strstr(buff, "URGENTE") != NULL){

                    // Mensaje urgente → enviar al hijo 1
                    write(pipes[2][1], buff, MAX_LEN);

                }else{
                    // Mensaje normal → enviar al padre
                    write(pipes[1][1], buff, MAX_LEN);
                }
            }         

            // Cierre de pipes
            close(pipes[0][0]);
            close(pipes[2][1]);
            close(pipes[1][1]);

        // -------- HIJO 1: ANALIZADOR --------
        }else{

            // Lee de pipe 2 (mensajes urgentes)
            // Escribe en pipe 3 (resultado final al padre)
            for(int i=0; i < n_pipes; i++){
                if(i != 2) close(pipes[i][0]);
                if(i != 3) close(pipes[i][1]);
            }

            Message msg;

            // Procesamiento de mensajes urgentes
            while(read(pipes[2][0], buff, MAX_LEN) > 0){
                printf("Child %d, recived: %s\n", child_id, buff);
                
                // Determina si es crítico
                if(strstr(buff, "servidor") || strstr(buff, "bloqueo") || strstr(buff, "caída")){
                    msg.type = 1; // crítico
                }else{
                    msg.type = 0; // urgente
                }

                strncpy(msg.text, buff, MAX_LEN);

                // Envío al padre
                write(pipes[3][1], &msg, sizeof(msg));
            }

            close(pipes[2][0]);
            close(pipes[3][1]);
        }
    }

    return EXIT_SUCCESS;
}

// ===================== LECTURA DE ARCHIVO =====================
void read_file(const char *file, char ***vec, int *size){
    FILE *fl = fopen(file, "r");
    if(!fl){
        perror("Fail Fopen\n");
        exit(-1);
    }

    char buffer[MAX_LEN];

    // Lee cantidad de líneas
    fscanf(fl, "%d", size);
    fgetc(fl); // consume salto de línea

    printf("Size: %d\n", *size);

    // Reserva memoria para el vector de strings
    *vec = (char**) malloc(*size * sizeof(char *));
    if(!(*vec)){
        perror("Fail malloc\n");
        exit(-1);
    }

    // Lectura línea por línea
    for(int i=0; i < *size; i++){
        fgets(buffer, sizeof(buffer), fl);

        // Elimina salto de línea
        buffer[strcspn(buffer, "\n")] = '\0';

        // Reserva memoria para cada string
        (*vec)[i] = (char*) malloc((strlen(buffer) + 1) * sizeof(char));
        strcpy((*vec)[i], buffer);
    }

    fclose(fl);
}

// ===================== MOSTRAR ÁRBOL DE PROCESOS =====================
void showtree(){
    char cmd[20] = {""};

    // Construye comando para ver jerarquía de procesos
    sprintf(cmd, "pstree -cAlp %d", getpid());

    // Ejecuta comando del sistema
    system(cmd);	
}