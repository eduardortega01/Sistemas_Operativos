#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

long long leerNumeros(char *filename, long long **vec);
long long sumar(long long *vec, long long inicio, long long fin);
void editarResultado(char *filename, long long resultado);
void sumar_Imprimir(char *filename);

int main(int argc, char *argv[]){
    if(argc < 3){
        printf("Uso: %s archivo.txt P\n", argv[0]);
        return 1;
    }

    long long *vector;
    long long totalNumeros = leerNumeros(argv[1], &vector);
    printf("Total de numeros leidos: %lld\n", totalNumeros);
    int P = atoi(argv[2]);
    long long chunk = totalNumeros / P;

    for (int i = 0; i < P; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error en el fork\n");
            exit(-1);
        }
        if (pid == 0) { // Hijo
            long long inicio = i * chunk;
            long long fin = (i == P - 1) ? totalNumeros : inicio + chunk; // El último proceso toma el resto
            printf("Proceso hijo %d, PID: %d, PPID: %d, sumando de %lld a %lld\n", i, getpid(), getppid(), inicio, fin);
            long long resultado = sumar(vector, inicio, fin);
            editarResultado("resultado.txt", resultado);
            printf("Hijo %d termino su proceso con suma: %lld\n", i, resultado);
            printf("Hijo %d termino su proceso\n", i);
            exit(0);
        }
    }

    for (int i = 0; i < P; i++)
    wait(NULL); //Padre espera a que todos los hijos terminen
    sumar_Imprimir("resultado.txt");
    free(vector);
    return 0; 
}

long long leerNumeros(char *filename, long long **vec){
    long long c, numero, totalNumeros;
    FILE *infile;
    infile = fopen(filename, "r");
    if(! infile ) { perror("Error al abrir el fichero\n"); exit(-1);}
    fscanf(infile, "%lld", &totalNumeros);
    *vec = (long long *) calloc(totalNumeros, sizeof(long long));
    if(! *vec ) { perror("Error en la reserva de memoria\n"); exit(-1);}
    for(c = 0; c < totalNumeros; c++){
        fscanf(infile, "%lld", &numero);
        (*vec)[c] = numero;
        printf("%lld ", numero);
    }
    fclose(infile);
    return c;
}

long long sumar(long long *vec, long long inicio, long long fin){
    long long suma = 0;
    for(long long i = inicio; i < fin; i++){
        suma += vec[i];
    }
    return suma;
}

void editarResultado(char *filename, long long resultado){
    FILE *outfile;
    outfile = fopen(filename, "a");
    if(! outfile ) { perror("Error al abrir el fichero\n"); exit(-1);}
    fprintf(outfile, "%lld\n", resultado);
    fclose(outfile);
}

void sumar_Imprimir(char *filename){
    long long *vector, sumaTotal = 0, i;
    FILE *infile;
    infile = fopen(filename, "r");
    if(! infile ) { perror("Error al abrir el fichero\n"); exit(-1);}
    while(fscanf(infile, "%lld", &i) != EOF){
        sumaTotal += i;
    }
    fclose(infile);
    printf("\nSuma total: %lld\n", sumaTotal);
}