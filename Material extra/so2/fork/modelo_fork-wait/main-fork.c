#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

long long leerNumeros(char *filename, long long **vec);
void escribirResultado(char *filename, long long resultado);
void editarResultado(char *filename, long long resultado);
long long sumar(long long *vec, long long inicio, long long fin);
void sumar_Imprimir(char *filename);

int main(){
    long long *vector, mitad, cantidadNumeros;
    cantidadNumeros = leerNumeros("input_5M.txt", &vector);
    printf("\nCantidad de numeros: %lld\n", cantidadNumeros);
    mitad = cantidadNumeros / 2;
    pid_t pid1, pid2;
    
    pid1 = fork();
    if(pid1 < 0) { perror("Error en el fork\n"); exit(-1); }
    if(pid1 == 0){ //Hijo 1
        long long resultado1;
        printf("Hijo 1, PID: %d, PPID: %d\n", getpid(), getppid());
        resultado1 = sumar(vector, 0, mitad);
        escribirResultado("resultado3.txt", resultado1);
        printf("hijo 1 temino su proceso\n");
        exit(0);
    } else { //Padre
        pid2 = fork();
        if(pid2 < 0) { perror("Error en el fork\n"); exit(-1); }
        if(pid2 == 0){ //Hijo 2
            long long resultado2;
            printf("Hijo 2, PID: %d, PPID: %d\n", getpid(), getppid());
            resultado2 = sumar(vector, mitad, cantidadNumeros);
            editarResultado("resultado3.txt", resultado2);
            printf("hijo 2 temino su proceso\n");
            exit(0);
        }
    wait(NULL);
    wait(NULL);
    sumar_Imprimir("resultado3.txt");
    }   
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

void escribirResultado(char *filename, long long resultado){
    FILE *outfile;
    outfile = fopen(filename, "w");
    if(! outfile ) { perror("Error al abrir el fichero\n"); exit(-1);}
    fprintf(outfile, "%lld\n", resultado);
    fclose(outfile);
}

void editarResultado(char *filename, long long resultado){
    FILE *outfile;
    outfile = fopen(filename, "a");
    if(! outfile ) { perror("Error al abrir el fichero\n"); exit(-1);}
    fprintf(outfile, "%lld\n", resultado);
    fclose(outfile);
}

long long sumar(long long *vec, long long inicio, long long fin){
    long long suma = 0;
    for(long long i = inicio; i < fin; i++){
        suma += vec[i];
    }
    return suma;
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