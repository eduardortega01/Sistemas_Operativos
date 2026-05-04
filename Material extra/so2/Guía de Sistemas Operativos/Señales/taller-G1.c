/*
-- Router de comandos Par/Impar por Señales (IPC en canal de control limitado)

Descripción: Un proceso nodo debe enviar información hacia un proceso adicional.
El canal es muy limitado por lo que solo puede viajar una señal al tiempo.
Sin embargo el enrutador debe comunicsar una matriz de datos númericos cada uno
de ellos menores de 10. Por lo tanto dado un archivo con una matriz

N, M 
d11 d12 d13 d14 ..d1m 
... 
dn1 dn2 dn3 dn4 ..dnm 

el proceso nodo debe comunicar la matriz al otro proceso
el proceso nodo, lee el archivo y realíza la comunicación 
el proceso receptor deberá imprimir la matriz por pantalla

Evaluación 
Diseño (10%): protocolo claro (señales usadas)
implementación (20%): prototipo funcional
sustentación (70%): sustentación oral

Se utiliza un protocolo: 
- SIGUSR1 -> Representa 1 pulso (el receptor suma 1 por cada señal recibida)
- SIGUSR2 -> Fin de número. El receptor imprime y reinicia el contador
- SIGTERM -> Fin total de la transmisión
*/

#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h>
#include<wait.h> 
#include<signal.h> 
#include<sys/types.h> 

int contador = 0; // para contar los pulsos SIGUSR1
int **matriz; // Matriz dinámica
int n, m; // Dimensiones de matriz
int i = 0, j = 0; // Iteradores para recorrer matriz
pid_t nodo, receptor; // PIDs de procesos
int iniciado = 0; // Evita imprimir 0 al inicio

// handler del receptor incrementa el contador por cada pulso recibido
void handler_contar(int sig) { 
    contador++;
    iniciado = 1;  // marca que ya empezaron a llegar datos
}

// handler del receptor imprime el número cuando el proceso nodo avisa "fin de número"
void handler_fin(int sig) {

    if (!iniciado) { // si aún no han llegado pulsos reales -> no imprimir 0
        kill(nodo, SIGUSR2); 
        return;
    }

    printf("%d ", contador); 
    fflush(stdout); // muestra en tiempo real 
    contador = 0; // reinicia para el siguiente número

    static int col = 0; 
    col++; 
    if (col == m) { // cuando termina una fila da el salto de línea
        printf("\n");
        col = 0;
    }

    kill(nodo, SIGUSR2); // pide el siguiente dato al proceso nodo 
}

// handler del nodo para enviar número al receptor
void handler_enviar(int sig) {

    if (i == n) {  // si ya no hay más datos -> terminar transmisión
        kill(receptor, SIGTERM);
        exit(0);
    }

    int numero = matriz[i][j];

    // enviar número como pulsos SIGUSR1
    for (int k = 0; k < numero; k++) {
        kill(receptor, SIGUSR1);
        usleep(10000); // evita pérdida de señales (canal limitado)
    }

    // avisar fin de número
    kill(receptor, SIGUSR2);
    // pasar al siguiente elemento
    j++; 
    if (j == m) {
        j = 0; 
        i++;
    }
}

// receptor termina ordenadamente al recibir SIGTERM
void handler_terminar(int sig) {
    printf("\nTransmisión finalizada\n");
    exit(0);
}

int main(){
    nodo = getpid(); // PID proceso padre

    // lectura de archivo y creación dinámica de la matriz
    FILE *archivo = fopen("matriz.txt", "r");  
    if (!archivo) { perror("Error abriendo el archivo"); exit(-1);}

    fscanf(archivo, "%d,%d", &n, &m);

    matriz = (int**) malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        matriz[i] = (int*) malloc(m * sizeof(int)); 
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            fscanf(archivo, "%d", &matriz[i][j]);
        }
    }

    fclose(archivo);

    // crear proceso receptor
    receptor = fork();

    if (!receptor) { // receptor
        signal(SIGUSR1, handler_contar);
        signal(SIGUSR2, handler_fin);
        signal(SIGTERM, handler_terminar);
        while (1) pause();
    } else { // nodo
        signal(SIGUSR2, handler_enviar); 

        sleep(1); // espera a que el receptor instale handlers
        kill(receptor, SIGUSR2); // primer avisao para iniciar envío
        while (1) pause();
    }

    // liberación de memoria dinámica
    for (int i = 0; i < n; i++) free(matriz[i]); 
    free(matriz);

    return 0; 
}




