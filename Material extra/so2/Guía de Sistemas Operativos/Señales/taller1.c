#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

void handler(int s){}

pid_t H1, H11, H2, H21;
int n; 

int main(){
    signal(SIGUSR1, handler); 

    printf("Ingrese el número máximo de repeticiones de la secuencia: \n");
    scanf("%d", &n);

    // Cada proceso que deba recibir una señal debe declarar su propio signal() con su manejador
    H1 = fork();
    if (H1 == 0){ // Hijo 1
        signal(SIGUSR1, handler);
        H11 = fork();
        if (H11 == 0) { //Hijo 11
            signal(SIGUSR1, handler);
            /*
            Se utiliza while(1) porque queremos que el proceso siga participando en la secuencia de señales varias veces.
            La secuencia se debe repetir n veces, entonces cada vez que llegue una señal, el proceso: 
            1. Se "despierta" (gracias a pause()),
            2. Hace su trabajo (printf()),
            3. Y envía la señal al siguiente proceso.
            Después de eso vuelve al pause() y espera otra señal.
            Así se mantiene sincronizado con los demás procesos sin consumir CPU (no usa sleep,, no hace bucles vacíos, solo espera).
            */
            while(1) { 
                pause(); // Detiene el proceso hasta que llegue una señal. Mientras no llegue ninguna, el proceso queda bloqueado.
                printf("Hijo11 [%d]\n", getpid());
                kill(getppid(), SIGUSR1);
            }
        } else { // Hijo 1
            while(1) { 
                pause();
                printf("Hijo1 [%d]\n", getpid());
                kill(H11, SIGUSR1);
                pause();
                printf("Hijo1 [%d]\n", getpid());
                kill(getppid(), SIGUSR1);
            }
        }
    } else {
        H2 = fork();
        if (H2 == 0) { // Hijo 2
            signal(SIGUSR1, handler);
            H21 = fork();
            if (H21 == 0) { // Hijo 21
                signal(SIGUSR1, handler);
                while (1) {
                    pause();
                    printf("Hijo21 [%d]\n", getpid());
                    kill(getppid(), SIGUSR1);
                }
            } else { // Hijo 2
                while (1) {
                    pause();
                    printf("Hijo2 [%d]\n", getpid());
                    kill(H21, SIGUSR1);
                    pause();
                    printf("Hijo2 [%d]\n", getpid());
                    kill(H1, SIGUSR1);
                }
            }
        } else { // Padre
            signal(SIGUSR1, handler);
            sleep(1); // Espera que todos los hijos estén listos
            for (int i = 0; i < n; i++){
                printf("Padre [%d]\n", getpid());
                kill(H2, SIGUSR1); //Inicia la secuencia
                pause();
            }
            printf("Padre [%d]\n", getpid());
            printf("\nSecuencia completada %d veces.\n", n);
            // Finaliza todos los procesos
            kill(H1, SIGTERM); //SIGTERM mata todos los procesos. s
            kill(H11, SIGTERM);
            kill(H2, SIGTERM);
            kill(H21, SIGTERM);
        }
    }
    return 0;
}