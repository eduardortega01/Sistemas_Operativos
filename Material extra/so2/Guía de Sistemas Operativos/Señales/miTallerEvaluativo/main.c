#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// Handler vacío para señales SIGUSR1
// Solo necesita existir para que pause() se despierte cuando llegue la señal
void handler(int sig) {}

// Variables globales para almacenar PIDs de todos los procesos
pid_t h1, h2, h11, h12, h21, h22, h112;
pid_t padre_pid; // Guardamos el PID del padre original
int n; // Número de repeticiones de la secuencia

int main() {
    printf("Ingrese el número máximo de repeticiones de la secuencia: ");
    scanf("%d", &n);

    // Registrar el handler para SIGUSR1 en el padre
    signal(SIGUSR1, handler);
    
    // IMPORTANTE: Guardar el PID del padre ANTES de hacer fork
    // Así todos los hijos tendrán este valor cuando se copien las variables
    padre_pid = getpid();

    // ==================== CREAR HIJO 1 ====================
    h1 = fork();
    if (h1 == -1) {
        perror("Error al crear h1");
        exit(-1);
    }

    if (h1 == 0) { // ----- PROCESO HIJO 1 -----
        // Cada hijo debe registrar su propio handler
        signal(SIGUSR1, handler);

        // ==================== CREAR HIJO 11 ====================
        h11 = fork();
        if (h11 == -1) {
            perror("Error al crear h11");
            exit(-1);
        }

        if (h11 == 0) { // ----- PROCESO HIJO 11 -----
            signal(SIGUSR1, handler);

            // ==================== CREAR HIJO 112 ====================
            h112 = fork();
            if (h112 == -1) {
                perror("Error al crear h112");
                exit(-1);
            }

            if (h112 == 0) { // ----- PROCESO HIJO 112 -----
                signal(SIGUSR1, handler);
                
                // CICLO DE HIJO 112:
                // 1. Espera señal (pause)
                // 2. Imprime su mensaje
                // 3. Envía señal de vuelta a su padre (H11)
                while (1) {
                    pause(); // Se bloquea hasta recibir señal
                    printf("Hijo 112 [%d]\n", getpid());
                    kill(getppid(), SIGUSR1); // getppid() = PID del padre (H11)
                }
                
            } else { // ---- PROCESO H11 (después de crear H112) ----
                
                // CICLO DE HIJO 11:
                // 1. Espera señal desde H1
                // 2. Imprime
                // 3. Envía señal a H112
                // 4. Espera respuesta de H112
                // 5. Imprime de nuevo
                // 6. Devuelve control a H1
                while (1) {
                    pause(); // Espera señal de H1
                    printf("Hijo 11 [%d]\n", getpid());
                    kill(h112, SIGUSR1); // Envía a H112
                    
                    pause(); // Espera respuesta de H112
                    printf("Hijo 11 [%d]\n", getpid());
                    kill(getppid(), SIGUSR1); // Devuelve a H1
                }
            }

        } else { // ---- PROCESO H1 (después de crear H11) ----
            
            // ==================== CREAR HIJO 12 ====================
            h12 = fork();
            if (h12 == -1) {
                perror("Error al crear h12");
                exit(-1);
            }

            if (h12 == 0) { // ---- PROCESO HIJO 12 ----
                signal(SIGUSR1, handler);
                
                // CICLO DE HIJO 12:
                // 1. Espera señal de H1
                // 2. Imprime
                // 3. Devuelve control a H1
                while (1) {
                    pause();
                    printf("Hijo 12 [%d]\n", getpid());
                    kill(getppid(), SIGUSR1); // Vuelve a H1
                }
                
            } else { // ---- PROCESO H1 (después de crear H12) ----
                
                // CICLO DE HIJO 1:
                // 1. Espera señal desde H2
                // 2. Imprime
                // 3. Envía señal a H12
                // 4. Espera respuesta de H12
                // 5. Imprime
                // 6. Envía señal a H11
                // 7. Espera respuesta de H11
                // 8. Imprime
                // 9. Devuelve control al PADRE usando padre_pid
                while (1) {
                    pause(); // Espera señal de H2
                    printf("Hijo 1 [%d]\n", getpid());
                    kill(h12, SIGUSR1); // Envía a H12
                    
                    pause(); // Espera respuesta de H12
                    printf("Hijo 1 [%d]\n", getpid());
                    kill(h11, SIGUSR1); // Envía a H11
                    
                    pause(); // Espera respuesta de H11
                    printf("Hijo 1 [%d]\n", getpid());
                    
                    // CLAVE: Usa padre_pid (variable global) para volver al padre
                    // No puede usar getppid() si fuera necesario comunicarse con otro proceso
                    kill(padre_pid, SIGUSR1); // Devuelve al PADRE
                }
            }
        }

    } else { // ---- PROCESO PADRE (después de crear H1) ----
        
        // ==================== CREAR HIJO 2 ====================
        h2 = fork();
        if (h2 == -1) {
            perror("Error al crear h2");
            exit(-1);
        }

        if (h2 == 0) { // ---- PROCESO HIJO 2 ----
            signal(SIGUSR1, handler);
            
            // IMPORTANTE: H2 hereda la variable global h1 con el PID correcto
            // porque h1 ya fue asignado por el padre antes de crear H2
            
            // ==================== CREAR HIJO 21 ====================
            h21 = fork();
            if (h21 == -1) {
                perror("Error al crear h21");
                exit(-1);
            }

            if (h21 == 0) { // ---- PROCESO HIJO 21 ----
                signal(SIGUSR1, handler);
                
                // CICLO DE HIJO 21:
                // 1. Espera señal de H2
                // 2. Imprime
                // 3. Devuelve control a H2
                while (1) {
                    pause();
                    printf("Hijo 21 [%d]\n", getpid());
                    kill(getppid(), SIGUSR1); // Vuelve a H2
                }
                
            } else { // ---- PROCESO H2 (después de crear H21) ----
                
                // ==================== CREAR HIJO 22 ====================
                h22 = fork();
                if (h22 == -1) {
                    perror("Error al crear h22");
                    exit(-1);
                }

                if (h22 == 0) { // ---- PROCESO HIJO 22 ----
                    signal(SIGUSR1, handler);
                    
                    // CICLO DE HIJO 22:
                    // 1. Espera señal de H2
                    // 2. Imprime
                    // 3. Devuelve control a H2
                    while (1) {
                        pause();
                        printf("Hijo 22 [%d]\n", getpid());
                        kill(getppid(), SIGUSR1); // Vuelve a H2
                    }
                    
                } else { // ---- PROCESO H2 (después de crear H22) ----
                    
                    // CICLO DE HIJO 2:
                    // 1. Espera señal del Padre
                    // 2. Imprime
                    // 3. Envía señal a H22
                    // 4. Espera respuesta de H22
                    // 5. Imprime
                    // 6. Envía señal a H21
                    // 7. Espera respuesta de H21
                    // 8. Imprime
                    // 9. Envía señal a H1 (su hermano, usando variable global h1)
                    while (1) {
                        pause(); // Espera señal del Padre
                        printf("Hijo 2 [%d]\n", getpid());
                        kill(h22, SIGUSR1); // Envía a H22
                        
                        pause(); // Espera respuesta de H22
                        printf("Hijo 2 [%d]\n", getpid());
                        kill(h21, SIGUSR1); // Envía a H21
                        
                        pause(); // Espera respuesta de H21
                        printf("Hijo 2 [%d]\n", getpid());
                        
                        // CLAVE: H2 puede enviar señal a H1 porque ambos son hermanos
                        // y H2 heredó la variable global h1 con el PID correcto
                        kill(h1, SIGUSR1); // Pasa control a H1
                    }
                }
            }

        } else { // ---- PROCESO PADRE (después de crear H2) ----
            
            // Esperar 1 segundo para asegurar que todos los procesos se crearon
            sleep(1);

            // ==================== EJECUTAR LA SECUENCIA N VECES ====================
            for (int i = 0; i < n; i++) {
                printf("Padre [%d]\n", getpid());
                kill(h2, SIGUSR1); // Inicia la secuencia enviando señal a H2
                pause(); // Se bloquea esperando que H1 le devuelva el control
            }

            // Mensaje final
            printf("Padre [%d]\n", getpid());
            printf("Secuencia completada %d veces.\n", n);

            // ==================== FINALIZAR PROCESOS ====================
            // Enviar SIGTERM a todos los hijos para terminarlos
            kill(h1, SIGTERM);
            kill(h2, SIGTERM);
            
            // Esperar a que los hijos terminen (evitar zombies)
            wait(NULL);
            wait(NULL);
        }
    }
    return 0;
}

/* 
==================== CONCEPTOS CLAVE PARA EL EXAMEN ====================

1. FORK Y VARIABLES GLOBALES:
   - fork() crea una COPIA del proceso
   - Las variables globales se COPIAN al hijo con sus valores actuales
   - Por eso H2 puede usar h1 (fue copiado del padre con el PID correcto)

2. SEÑALES SIGUSR1:
   - signal(SIGUSR1, handler): Registra el manejador
   - kill(pid, SIGUSR1): Envía la señal al proceso con ese PID
   - pause(): Bloquea el proceso hasta recibir una señal

3. PIDs IMPORTANTES:
   - getpid(): Retorna el PID del proceso actual
   - getppid(): Retorna el PID del padre del proceso actual
   - Variables globales (h1, h2, etc.): PIDs de hermanos/otros procesos

4. FLUJO DE LA SECUENCIA:
   Padre → H2 → H22 → H2 → H21 → H2 → H1 → H12 → H1 → H11 → H112 → H11 → H1 → Padre

5. COMUNICACIÓN ENTRE HERMANOS:
   - H2 y H1 son hermanos (ambos hijos del Padre)
   - H2 puede enviar señal a H1 usando la variable global h1
   - H1 puede enviar señal al Padre usando padre_pid (guardado antes de fork)

6. EVITAR ZOMBIES:
   - wait(NULL): El padre espera a que terminen los hijos
   - Previene procesos zombie en el sistema
*/