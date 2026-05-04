#include <stdio.h>
#include <stdlib.h>

int main() {
    int n;
    printf("¿Cuántos números deseas ingresar? ");
    scanf("%d", &n);

    int *numeros = (int *)malloc(n * sizeof(int));
    if (numeros == NULL) {
        printf("Error al asignar memoria.\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        printf("Número %d: ", i + 1);
        scanf("%d", &numeros[i]);
    }

    printf("¿Cuántos números más deseas agregar? ");
    int extra;
    scanf("%d", &extra);

    // Redimensionar memoria
    numeros = (int *)realloc(numeros, (n + extra) * sizeof(int));
    if (numeros == NULL) {
        printf("Error al redimensionar memoria.\n");
        return 1;
    }

    // Ingresar los nuevos valores
    for (int i = n; i < n + extra; i++) {
        printf("Nuevo número %d: ", i + 1);
        scanf("%d", &numeros[i]);
    }

    // Mostrar todos los números
    printf("Todos los números: ");
    for (int i = 0; i < n + extra; i++) {
        printf("%d ", numeros[i]);
    }

    free(numeros);
    return 0;
}
