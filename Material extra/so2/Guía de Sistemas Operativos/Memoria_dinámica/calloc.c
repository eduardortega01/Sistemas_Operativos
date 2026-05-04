#include <stdio.h>
#include <stdlib.h>

int main() {
    int n;
    printf("¿Cuántos números deseas ingresar? ");
    scanf("%d", &n);

    // calloc reserva espacio para n enteros, todos inicializados en 0
    int *numeros = (int *)calloc(n, sizeof(int));

    if (numeros == NULL) {
        printf("Error: no se pudo asignar memoria.\n");
        return 1;
    }

    printf("Contenido inicial (debe ser 0): ");
    for (int i = 0; i < n; i++) {
        printf("%d ", numeros[i]);
    }

    free(numeros);
    return 0;
}


/* 
¿Cuántos números deseas ingresar? 5
Contenido inicial (debe ser 0): 0 0 0 0 0
*/