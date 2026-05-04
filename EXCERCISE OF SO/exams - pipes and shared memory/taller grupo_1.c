#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

#define MAX 10000

// Estructura que representa un registro de video
typedef struct {
    int video_id;
    int user_id;
    int face_score, audio_score, sync_score;
    int origin;

    int partial_risk;
    int is_suspisous; // 0 = no, 1 = sospechoso
} Data;

// Resultado por usuario
typedef struct {
    int user_id;
    int count; // cantidad de videos sospechosos
} Result;

void showtree();
int is_suspisous(Data data);

int main(int argc, char **argv){

    // Validación de entrada
    if(argc < 2){
        perror("Send the file\n");
        return EXIT_FAILURE;
    }

    pid_t root = getpid();

    int n_children = 3, child_id;
    int n_pipes = n_children;

    // Pipes de entrada (padre → hijo)
    int in_pipes[n_children][2];

    // Pipes de salida (hijo → padre)
    int out_pipes[n_children][2];

    // Creación de pipes
    for(int i=0; i < n_pipes; i++){
        pipe(in_pipes[i]);
        pipe(out_pipes[i]);
    }

    // Creación de hijos
    for(child_id=0; child_id < n_children; child_id++){
        if(!fork()) break;
    }

    // ===================== PROCESO PADRE =====================
    if(root == getpid()){
        showtree();

        // El padre:
        // - escribe en in_pipes
        // - lee de out_pipes
        for(int i=0; i < n_pipes; i++){
            close(in_pipes[i][0]);   // no lee de entrada
            close(out_pipes[i][1]); // no escribe en salida
        }

        int N, B, turn = 0;
        int total_susp, total_bann;

        Data data;
        Result results = {0}, total[MAX];

        FILE *fl;

        total_susp = total_bann = 0;

        fl = fopen(argv[1], "r");
        if(!fl) exit(1);

        // B = tamaño de bloque, N = total registros
        fscanf(fl, "%d %d", &B, &N);
        printf("B: %d, N: %d\n", B, N);

        // Enviar B a todos los hijos (parámetro de procesamiento)
        for(int i=0; i < n_pipes; i++){
            write(in_pipes[i][1], &B, sizeof(int));
        }

        // Distribución de datos en round-robin
        for(int i=0; i < N/B; i++){
            for(int j=0; j < B; j++){

                // Leer registro del archivo
                fscanf(fl, "%d %d %d %d %d %d",
                    &data.video_id,
                    &data.user_id,
                    &data.face_score,
                    &data.audio_score,
                    &data.sync_score,
                    &data.origin
                );

                // Enviar registro al hijo correspondiente
                write(in_pipes[turn][1], &data, sizeof(Data));

                turn++;
                if(turn == n_children) turn = 0; // round-robin
            }
        }

        // Cierre de pipes de escritura → indica fin de datos
        for(int i=0; i < n_pipes; i++){
            close(in_pipes[i][1]);
        }

        // Inicializa acumulador total
        memset(total, 0, sizeof(total));
        
        // Lectura de resultados desde cada hijo
        for(int i=0; i < n_pipes; i++){
            while(read(out_pipes[i][0], &results, sizeof(results)) > 0){

                // Señal de fin enviada por el hijo
                if(results.user_id == -1) break;

                // Acumula resultados por usuario
                total[results.user_id].user_id = results.user_id;
                total[results.user_id].count += results.count;
            }
        }

        // Cierre de pipes de lectura
        for(int i=0; i < n_pipes; i++){
            close(out_pipes[i][0]);
        }

        fclose(fl);

        // Procesamiento final: usuarios sospechosos y baneados
        for(int i=0; i < MAX; i++){
            if(total[i].count > 0){

                total_susp += total[i].count;

                // Si tiene más de 3 → baneado
                if(total[i].count > 3){
                    printf("User: %d, Count: %d\n", i, total[i].count);
                    total_bann++;
                }
            }
        }

        // Espera a hijos
        for(int i=0; i < n_children; i++) wait(NULL);

        printf("All banned: %d\n", total_bann);
        printf("All suspisous: %d\n", total_susp);

    // ===================== PROCESOS HIJOS =====================
    }else{

        // Configuración de pipes:
        // Cada hijo usa SOLO su canal
        for(int i=0; i < n_pipes; i++){
            if(i == child_id){
                close(in_pipes[i][1]);   // solo lee
                close(out_pipes[i][0]); // solo escribe
            }else{
                // Cierra TODOS los pipes que no usa
                close(in_pipes[i][0]); close(in_pipes[i][1]);
                close(out_pipes[i][0]); close(out_pipes[i][1]);
            }
        }

        int B, lines = 0;
        Data recived;
        Result resp[MAX];

        // Inicializa acumulador local
        memset(resp, 0, sizeof(resp));

        // Lee tamaño de bloque
        read(in_pipes[child_id][0], &B, sizeof(int));
        
        // Procesamiento de datos
        while(read(in_pipes[child_id][0], &recived, sizeof(Data)) > 0){

            // Si es sospechoso → acumula por usuario
            if(is_suspisous(recived)){
                resp[recived.user_id].user_id = recived.user_id;
                resp[recived.user_id].count++;
            }

            lines++;

            // Cuando completa un bloque B → envía resultados parciales
            if(lines == B){
                for(int i=0; i < MAX; i++){
                    if(resp[i].count > 0){
                        write(out_pipes[child_id][1], &resp[i], sizeof(Result));
                    }
                }
                lines = 0;
                memset(resp, 0, sizeof(resp));
            }
        }

        close(in_pipes[child_id][0]);
        
        // Envío de lo que quede pendiente
        for(int i=0; i < MAX; i++){
            if(resp[i].count > 0){
                write(out_pipes[child_id][1], &resp[i], sizeof(Result));
            }
        }

        // Señal de fin al padre
        Result end = {-1, -1};
        write(out_pipes[child_id][1], &end, sizeof(Result));

        close(out_pipes[child_id][1]);
    }

    return EXIT_SUCCESS;
}

// ===================== FUNCIÓN DE CLASIFICACIÓN =====================
int is_suspisous(Data data){

    int is = 0;

    if(data.face_score >= 80) is++;
    if(data.audio_score >= 75) is++;
    if(data.sync_score <= 35) is++;

    // Es sospechoso si cumple al menos 2 condiciones
    return is >= 2;
}

// ===================== ÁRBOL DE PROCESOS =====================
void showtree(){
    char cmd[20] = {""};
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);	
}