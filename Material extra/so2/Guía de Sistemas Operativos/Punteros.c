/*
¿Qué es un puntero?
Es una variable que duarda la posición de memoria de otra varibale
*/
#include<stdio.h>
#include<stdlib.h>

void cambiarValor(int *ptr) {
    *ptr = 100;  // Cambia el valor de la variable original
}

int main(){

int x = 10;   // variable normal
int *p;       // declaración de puntero a entero
p = &x;       // el puntero guarda la dirección de x
*p = 20;  // Cambia el valor de x a través del puntero
cambiarValor(&x);

// Para declarar un puntero en C se sigue la siguiente estructura: 
// Tipo de puntero + * + nombre del puntero.
int* punteroInt;
char* PunteroChar; 
float* punteroFloat;
double* punteroDouble;

// Para asignar una posición a un puntero
int numero = 10; 
// Se utiliza & para asignar la posición 
punteroInt = &numero; 
// Para imprimir el puntero
printf("%u\n", punteroInt);

// *punteroIn imprime lo que contiene la variable numero. 

int vector[] = {1,2,3,4,5};
int* dir_vec = vector; // se asigna la posición del primer elemento del vector
for (int i = 0; i < 5; i++){
    printf("[%d] - direccion en memoria [%u]", vector[i],dir_vec++);
}

int arr[3] = {10, 20, 30};
int *p = arr;  // arr equivale a &arr[0]

printf("%d\n", *p);       // 10
printf("%d\n", *(p + 1)); // 20
printf("%d\n", *(p + 2)); // 30


int x = 5;
int *p = &x;
int **pp = &p;
/*
x = 5
*p = 5
**pp = 5

*/

printf("x = %d\n", x);
printf("*p = %d\n", *p);
printf("**pp = %d\n", **pp);

return 0; 
}