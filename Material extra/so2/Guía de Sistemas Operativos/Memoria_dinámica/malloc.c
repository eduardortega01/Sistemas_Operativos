#include <stdio.h>
#include <stdlib.h>

int main() {
    int n;
    printf("¿Cuántos números deseas ingresar? ");
    scanf("%d", &n);

    // Reservamos memoria para n enteros
    int *numeros = (int *)malloc(n * sizeof(int));

    if (numeros == NULL) {
        printf("Error: no se pudo asignar memoria.\n");
        return 1;
    }

    // Guardar los valores
    for (int i = 0; i < n; i++) {
        printf("Número %d: ", i + 1);
        scanf("%d", &numeros[i]);
    }

    // Mostrar los valores
    printf("Los números son: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", numeros[i]);
    }

    // Liberar la memoria
    free(numeros);

    return 0;
}
