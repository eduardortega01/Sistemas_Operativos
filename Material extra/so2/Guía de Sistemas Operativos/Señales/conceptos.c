#include<stdio.h> // entrada/salida estándar (printf)
#include<stdlib.h> // funciones del sistema (exit)
#include<unistd.h> // funciones del sistema UNIX (fork, sleep, getpid)
#include<wait.h> // para que el padre espere al hijo (wait)
#include<signal.h> // para manejar señales (signal, kill, SIGUSR1)

/*
Definición del manejador
Es una función especial que se ejecuta automáticamente cuando un proceso recibe una señal
*/
void handler(int sig) {
    printf("Soy el hijo y recibí una señal del padre! (%d)\n", sig);
}

/*
kill(pid, sig) enviar una señal a otro proceso
pause() Esperar a que llegue una señal
SIGUSR1 / SIGUSR2 Señales libres para comunicación de procesos
*/

int main(){
    pid_t pid = fork();
    /*
    Configuración del manejador
    Esto le indica al sistema que cuando el proceso reciba una señal, use la función handler.
    SIG_ERR indica si hubo un error al configurarla.
    */

    /* 
    Función signal()
    signal(int signal_number, void (*handler)(int));
    - signal_number: la señal
    - handler: función a ejecutar cuando llegue una señal.
    */
    signal(SIGUSR1, handler); 
    if (signal(SIGUSR1, handler) == SIG_ERR) {
        perror("signal:");
        exit(EXIT_FAILURE);
    }

    if (pid == 0){ // proceso hijo 
        signal(SIGUSR1, handler);
        pause(); 
        printf("Hijo finaliza\n");
    } else { // proceso padre
        sleep(1); // damos tiempo al hijo para configurar al handler
        kill(pid, SIGUSR1); 
        printf("padre envió señal\n");
    }
    /* 
    Restaurar el manejador original
    Esto restaura el manejador previo de la señal
    (buena prática para no dejarlo modificado). 
    */
    if (signal(SIGUSR1, signal(SIGUSR1, handler)) == SIG_ERR){
        perror("signal:");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS; 
}