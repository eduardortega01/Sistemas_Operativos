#include<stdio.h> // estadar para trabajar entrada y salida
#include<stdlib.h> // para manejar memoria dinámica
#include<string.h> // para trabajar con cadenas de caracteres

/* 
Lectura de una matriz desde un archivo txt y creación dinámica 
*/

int main(){
    // FILE *:  es un puntero a una estructura que representa un archivo en C
    FILE *archivo = fopen("txt.txt", "r"); // Abrir el archivo
    if (!archivo) { perror("Error abriendo el archivon"); exit(-1);} // verificar sí abrió bien el archivo
    int n, m; // n = fila, m = columnas de la matriz
    fscanf(archivo, "%d,%d", &n, &m); // leer primera línea y avanza a la siguiente.
    int **matriz; // Puntero a puntero
    matriz = (int**) malloc(n * sizeof(int*)); // reservar espacio para las filas
    for (int i = 0; i < n; i++) { // reservar memoria para fila
        matriz[i] = (int*) malloc(m * sizeof(int)); // cada puntero se convierte en una fila de enteros
    } // se crean n fila, cada una con m colunas de enteros

    // llenar la matriz
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            fscanf(archivo, "%d", &matriz[i][j]);
        }
    }

    // Cerrar el archivo
    fclose(archivo); 

    // Imprimir
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%d ", matriz[i][j]);
        }
        printf("\n");
    }

    for (int i = 0; i < n; i++){
        free(matriz[i]); // liberar espacio de cada fila
    }
    free(matriz); // liberar arreglo de punteros

    return 0; 
}