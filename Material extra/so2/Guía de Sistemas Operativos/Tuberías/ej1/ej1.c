#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>

int main(){
    int pipe_a_d[2], pipe_d_g[2], pipe_g_i[2], pipe_i_g[2], pipe_g_d[2];
    int pipe_d_c[2], pipe_c_f[2], pipe_f_c[2];
    int pipe_c_b[2], pipe_b_e[2], pipe_e_h[2], pipe_h_e[2], pipe_e_b[2];
    int pipe_b_a[2]; 
    
    // Crear tuberías - SIN [2]
    if (pipe(pipe_a_d) == -1 || pipe(pipe_d_g) == -1 || pipe(pipe_g_i) == -1 ||
        pipe(pipe_i_g) == -1 || pipe(pipe_g_d) == -1 || pipe(pipe_d_c) == -1 || 
        pipe(pipe_c_f) == -1 || pipe(pipe_f_c) == -1 || pipe(pipe_c_b) == -1 || 
        pipe(pipe_b_e) == -1 || pipe(pipe_e_h) == -1 || pipe(pipe_h_e) == -1 || 
        pipe(pipe_e_b) == -1 || pipe(pipe_b_a) == -1) {
        perror("Error al crear las tuberías"); 
        exit(1);
    }

    printf("Proceso 0 [%d]\n", getpid());

    pid_t d = fork(); // Crear proceso d
    if (d == 0) { // Proceso d (hijo)
        char mensaje[100];
        sprintf(mensaje, "Proceso 1 [%d]\n", getpid());

        close(pipe_a_d[1]); // cerrar escritura a -> d

        // Esperar la señal de a
        char buffer_a[100];
        read(pipe_a_d[0], buffer_a, sizeof(buffer_a)); 
        printf("%s", buffer_a);  // Sin \n extra, ya está en buffer
        close(pipe_a_d[0]);

        pid_t g = fork(); 
        if (g == 0) {  // Proceso g
            char mensaje_g[100];
            sprintf(mensaje_g, "Proceso 2 [%d]\n", getpid());

            close(pipe_d_g[1]); // cerrar escritura d -> g

            // esperar señal de d
            char buffer_d[100];
            read(pipe_d_g[0], buffer_d, sizeof(buffer_d));
            printf("%s", buffer_d);
            close(pipe_d_g[0]);

            pid_t i = fork();
            if (i == 0) {  // Proceso i
                char mensaje_i[100];
                sprintf(mensaje_i, "Proceso 7 [%d]\n", getpid());

                close(pipe_g_i[1]); // cerrar escritura g -> i

                char buffer_g[100]; 
                read(pipe_g_i[0], buffer_g, sizeof(buffer_g));
                printf("%s", buffer_g);
                close(pipe_g_i[0]);

                close(pipe_i_g[0]); // cerrar lectura i -> g
                write(pipe_i_g[1], mensaje_i, strlen(mensaje_i) + 1);
                close(pipe_i_g[1]);
                
                exit(0);  // ¡Importante!
            }

            // Proceso g continúa
            close(pipe_g_i[0]); // cerrar lectura g -> i
            write(pipe_g_i[1], mensaje_g, strlen(mensaje_g) + 1);
            close(pipe_g_i[1]);

            close(pipe_i_g[1]); // cerrar escritura i -> g
            char buffer_i[100];
            read(pipe_i_g[0], buffer_i, sizeof(buffer_i));
            printf("%s", buffer_i);
            close(pipe_i_g[0]);

            close(pipe_g_d[0]); // cerrar lectura g -> d
            write(pipe_g_d[1], mensaje_g, strlen(mensaje_g) + 1);
            close(pipe_g_d[1]);

            wait(NULL);  // Esperar al hijo i
            exit(0);     // ¡Importante!
        }

        // Proceso d continúa
        close(pipe_d_g[0]); // cerrar lectura d -> g
        write(pipe_d_g[1], mensaje, strlen(mensaje) + 1);
        close(pipe_d_g[1]);

        close(pipe_g_d[1]); // cerrar escritura g -> d
        char buffer_g[100]; 
        read(pipe_g_d[0], buffer_g, sizeof(buffer_g));
        printf("%s", buffer_g);
        close(pipe_g_d[0]);

        close(pipe_d_c[0]); // cerrar lectura d -> c
        write(pipe_d_c[1], mensaje, strlen(mensaje) + 1);
        close(pipe_d_c[1]);  // ¡Corregido!

        wait(NULL);  // Esperar al hijo g
        exit(0);     // ¡Importante!
    }

    // Proceso principal (a) continúa
    close(pipe_a_d[0]); // cerrar lectura a -> d
    char mensaje_a[100];
    sprintf(mensaje_a, "Proceso 0 [%d]\n", getpid());
    write(pipe_a_d[1], mensaje_a, strlen(mensaje_a) + 1);
    close(pipe_a_d[1]);

    wait(NULL);  // Esperar al hijo d
    
    return 0; 
}