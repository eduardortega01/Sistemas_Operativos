/*Buscar el entero que mas se repite*/




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

void readfile(const char *file, int **vec, int *size)

int main(int argc, char  **argv[]){
    if(argc < 2){
        perror("error open file");
        return EXIT_FAILURE;
    }
    int size;
    int *vec;

    readfile(argv[1], &vec, &size);

    return EXIT_SUCCESS;
}

void readfile(const char *file, int **vec, int *size){
    FILE *fl = fopen(file, "r");
    if(!fl){
        perror("fail open\n");
        exit(-1);
    }

    fscanf(fl, "%d", size);

    fclose(fl);
}

 