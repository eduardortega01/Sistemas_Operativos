#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){
    int a = argc + 2;

    FILE* archivo = fopen("entrada.txt", "r");
    
    char opcion[30];
    char cadena[30];

    fgets(opcion, 30, archivo);
    opcion[strcspn(opcion, "\n")] =  '\0';
    

    printf("%s\n", opcion);
    if(strcmp(opcion, "encode") == 0 || strcmp(opcion, "ENCODE") == 0){
        fgets(cadena, 30, archivo);
        cadena[strcspn(cadena, "\n")]='\0';
        //printf("Entra por ENCODE\n");
        char actual = cadena[0];
        int contador = 1;
        for(size_t i = 1; i<strlen(cadena); i++){
            if(actual == cadena[i]){
                contador++;
            }else{
                printf("%c%d", actual, contador);
                actual = cadena[i];
                contador = 1;
            }
        }

    }else if(strcmp(opcion, "decode") == 0 || strcmp(opcion, "DECODE") == 0){
        char actual;
        int contador;
        while (1) {
            fscanf(archivo, "%c", &actual);
            if(actual == '\n'){
                break;
            }
            fscanf(archivo, "%d", &contador);
        
            for(int i =0; i<contador; i++){
                printf("%c", actual);
            }
        }
        
        
    }else{
        printf("Opcion inválida");
    }
    printf("\n");
} 