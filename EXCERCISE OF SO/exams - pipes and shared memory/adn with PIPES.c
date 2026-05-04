#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <sys/types.h>

#define MAX_LEN 256

// ===================== ESTRUCTURAS =====================

// Estadísticas de bases de ADN
typedef struct ADN_statistics{
    int A, T, C, G;
} ADN_statistics;

// Resultado de búsqueda de patrón
typedef struct Found{
    int child_id, pos;
} Found;

void showtree();

// Lee archivo y guarda cadena ADN
void read_file(const char *file, char **vec, int *size);

// ===================== MAIN =====================
int main(int argc, char **argv){

    // Se espera: archivo + número de procesos
    if(argc < 3){
        perror("Send a file and n_children\n");
        return EXIT_FAILURE;
    }

    pid_t root = getpid();

    int n_children = atoi(argv[2]), child_id;

    char *vec; // cadena ADN
    int size;

    int pipes[n_children][2]; // un pipe por hijo
    int chunk;

    // Leer archivo
    read_file(argv[1], &vec, &size);

    // Crear pipes
    for(int i=0; i < n_children; i++){
        pipe(pipes[i]);
    }

    // División del trabajo (ojo con esto 👇)
    chunk = size / (n_children-1);

    // Crear procesos hijos
    for(child_id=0; child_id < n_children; child_id++){
        if(!fork()) break;
    }

    // ===================== PADRE =====================
    if(root == getpid()){

        // Cierra extremos de escritura
        for(int i=0; i < n_children; i++) close(pipes[i][1]);

        // Espera hijos
        for(int i=0; i < n_children; i++) wait(NULL);

        printf("\nPARENT PROCESS:\n");

        // Leer resultados de cada hijo
        for(int i=0; i < n_children; i++){

            if(i == 0){
                // Hijo 0 envía estadísticas
                ADN_statistics data;

                read(pipes[i][0], &data, sizeof(ADN_statistics));

                printf("Statistics:\nA: %d\nT: %d\nC: %d\nG: %d\n\n",
                    data.A, data.T, data.C, data.G);

            }else{
                // Otros hijos envían posiciones encontradas
                Found data;

                read(pipes[i][0], &data, sizeof(Found));

                printf("Child %d found pattern in %d\n",
                    data.child_id, data.pos);
            }
        }

        // Cerrar pipes
        for(int i=0; i < n_children; i++) close(pipes[i][0]);

    // ===================== HIJOS =====================
    }else{

        // Cerrar extremos de lectura
        for(int i=0; i < n_children; i++) close(pipes[i][0]);

        // ---------------- HIJO 0 ----------------
        if(child_id == 0){

            ADN_statistics stats = {0};

            // Cuenta bases en TODO el ADN
            for(int i=0; i < size; i++){
                switch(vec[i]){
                    case 'A': stats.A++; break;
                    case 'T': stats.T++; break;
                    case 'C': stats.C++; break;
                    case 'G': stats.G++; break;
                }
            }

            // Envía resultado al padre
            write(pipes[child_id][1], &stats, sizeof(ADN_statistics));

        // ---------------- OTROS HIJOS ----------------
        }else{

            int idx_start, idx_end;

            const char *pattern = "TTGTAC";
            int pattern_len = strlen(pattern);

            Found found_p = {0};

            // División del trabajo (por segmentos)
            idx_start = chunk * (child_id-1);
            idx_end = (child_id == n_children-1) ? size : chunk + idx_start;

            // Buscar patrón en su segmento
            for(int i=idx_start; i < idx_end; i++){

                if(vec[i] == 'T'){ // optimización básica

                    if(strncmp(&vec[i], pattern, pattern_len) == 0){

                        found_p.child_id = child_id;
                        found_p.pos = i;

                        // Envía resultado al padre
                        write(pipes[child_id][1], &found_p, sizeof(Found));
                    }
                }
            }
        }

        // Cerrar pipes
        for(int i=0; i < n_children; i++) close(pipes[i][1]);
    }

    // Liberar memoria
    free(vec);
    vec = NULL;

    return EXIT_SUCCESS;
}

// ===================== LECTURA =====================

void read_file(const char *file, char **vec, int *size){

    FILE *fl = fopen(file, "r");

    if(!fl){
        perror("Fail Fopen\n");
        exit(-1);
    }

    fscanf(fl, "%d", size);
    printf("n bases: %d\n", *size);

    fgetc(fl); // consumir salto de línea

    *vec = (char*) malloc(*size * sizeof(char));

    if(!*vec){
        perror("Fail Malloc");
        exit(1);
    }

    // Leer cadena ADN
    for(int i=0; i < *size; i++){
        fscanf(fl, "%c", &(*vec)[i]);
    }

    fclose(fl);
}