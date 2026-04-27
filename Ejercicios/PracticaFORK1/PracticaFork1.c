#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


int main(){
    pid_t pidHijo;
    pidHijo=fork();
    if(pidHijo<0){
        perror("Error en la creacion\n"); _exit(-1);
    }
    printf("Hola mundo Fork() [%d] [%d]!\n", getpid(), getppid());
    wait(NULL);
    return 0;
}

int LeerNumeros(char *filename, int **vec){
    int c, numero, totalNumeros;
    FILE *infile;
    infile=fopen(filename, "r");
    if(!infile){
        error ("Error fopen\n");
    }
    fscanf(infile, "%d", &totalNumeros);
    *vec= (int *)calloc(totalNumeros, sizeof(int));
    if(!*vec){
        error("error calloc");
        for (c=0; c<totalNumeros; c++)
        {
            fscanf(infile, "%d", &numero);
            (*vec) [c]= numero;
            printf("%d\n", numero);
        }
        fclose(infile);
        return c;
        
    }

}


void error(char *msg){
    perror(msg);
    exit(-1);
}
    
int leerTotal(){
    FILE *infile;
    int sumap1=0, sumap2=0, total=0;
    infile= fopen("out.txt", "r");
    if(!infile) error("error padre archivo resultados");
    fscanf(infile, "%d", &sumap1);
    fscanf(infile, "%d", &sumap2);
    total = sumap1+sumap2;
    return total;
}
