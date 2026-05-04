#include <stdio.h>

int main(){
    FILE *fp;
    char method[10];
    fp = fopen("cadena.txt", "r");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return 1;
    }
    fscanf(fp, "%s", method);
    if(strcmp(method, "rle_encode") == 0){
        rle_encode();
    } else if(strcmp(method, "rle_decode") == 0){
        rle_decode();
    }
    fclose(fp);
    return 0; 
}

void rle_encode(){
    printf("Hello, I am method rle_encode\n");
}

void rle_decode(){
    printf("Hello, I am method rle_decode\n");
}