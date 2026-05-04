#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void handler(int s) {}

/**
 * Función auxiliar para mostrar el árbol de procesos.
 */
void print_debug_tree() {
    char cmd[80];
    sprintf(cmd, "pstree -lp %d", getpid());
    system(cmd);
}

int main() {
    signal(SIGUSR1, handler);

    pid_t root = getpid();
    int n_hijos = 2;
    pid_t vec_hijos[n_hijos];
    int profundidad = 1;

    int n_repeticiones;
    printf("Ingrese el número de repeticiones: ");
    scanf("%d", &n_repeticiones);

    // Crear hijos del padre
    int i = 0;
    for (; i < n_hijos; ++i) {
        vec_hijos[i] = fork();
        if (vec_hijos[i] == 0) break; // el hijo sale del for
    }

    if (getpid() == root) {
        // === PADRE ===
        usleep(500000);
        print_debug_tree();

        for (int rep = 0; rep < n_repeticiones; rep++) {
            printf("\n=== CICLO %d ===\n", rep + 1);
            printf("PADRE [%d] Enviando señal a Hijo2 [%d]\n", getpid(), vec_hijos[1]);
            kill(vec_hijos[1], SIGUSR1);
            pause(); // espera a que el último hijo le responda
        }

        // Terminar a los hijos después de los ciclos
        for (int k = 0; k < n_hijos; k++) {
            kill(vec_hijos[k], SIGTERM);
        }

        printf("PADRE [%d] finalizando ejecución.\n", getpid());
    } else {
        // === HIJOS ===
        pid_t _hijo;
        int flag = 0;

        for (int j = 0; j < profundidad; j++) {
            _hijo = fork();
            if (_hijo > 0) break;
            flag = 1;
        }

        // Hijo del hijo (nivel más bajo)
        if (flag == 1) {
            for (int rep = 0; rep < n_repeticiones; rep++) {
                pause();
                printf("Hijo del hijo [%d] Enviando señal al padre [%d]\n", getpid(), getppid());
                kill(getppid(), SIGUSR1);
            }
            exit(0);
        }

        // Hijo directo del padre
        if (i == 0) {
            for (int rep = 0; rep < n_repeticiones; rep++) {
                pause();
                printf("Hijo1 [%d] Enviando señal a su hijo [%d]\n", getpid(), _hijo);
                kill(_hijo, SIGUSR1);
                pause();
                printf("Hijo1 [%d] Enviando señal al padre [%d]\n", getpid(), root);
                kill(root, SIGUSR1);
            }
            exit(0);
        } else if (i == 1) {
            for (int rep = 0; rep < n_repeticiones; rep++) {
                pause();
                printf("Hijo2 [%d] Enviando señal a su hijo [%d]\n", getpid(), _hijo);
                kill(_hijo, SIGUSR1);
                pause();
                printf("Hijo2 [%d] Enviando señal a Hijo1 [%d]\n", getpid(), getppid() - 1);
                kill(getppid() - 1, SIGUSR1); // o mejor: kill(vec_hijos[0], SIGUSR1)
            }
            exit(0);
        }
    }

    return 0;
}
