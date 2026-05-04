#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void handler(int s) {}

pid_t hijos[6];
int num_hijos = 6;

int main() {
    signal(SIGUSR1, handler);

    pid_t padre = getpid();

    // Crear los hijos
    for (int i = 0; i < num_hijos; i++) {
        hijos[i] = fork();

        if (hijos[i] < 0) {
            perror("Error al crear el hijo");
            exit(1);
        }

        if (hijos[i] == 0) { // Código de cada hijo
            signal(SIGUSR1, handler);
            pause(); // espera señal
            printf("HIJO %d [%d]\n", i + 1, getpid());
            if (i > 0)
                kill(hijos[i - 1], SIGUSR1); // avisa al anterior
            else
                kill(padre, SIGUSR1); // el último hijo avisa al padre
            exit(0);
        }
    }

    // Código del padre
    sleep(1); // asegura que todos los hijos estén listos
    printf("PADRE [%d]\n", getpid());

    // el padre inicia la secuencia con el último hijo
    kill(hijos[num_hijos - 1], SIGUSR1);
    pause();

    printf("PADRE [%d] (Fin de secuencia)\n", getpid());
    return 0;
}
