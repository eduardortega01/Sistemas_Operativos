//Enviar múltiples mensajes
// ESCRITOR
for(int i = 0; i < 5; i++) {
    write(fd[1], &i, sizeof(int));
}
close(fd[1]);  // Importante: cerrar para enviar EOF

// LECTOR
int num;
while(read(fd[0], &num, sizeof(int)) > 0) {
    printf("Recibido: %d\n", num);
}
close(fd[0]);

// Protocolo con centinela
// ESCRITOR
int numeros[] = {1, 2, 3, 4, 5};
for(int i = 0; i < 5; i++) {
    write(fd[1], &numeros[i], sizeof(int));
}
int fin = -1;  // Centinela
write(fd[1], &fin, sizeof(int));
close(fd[1]);

// LECTOR
int num;
while(1) {
    read(fd[0], &num, sizeof(int));
    if(num == -1) break;  // Detectar centinela
    printf("%d ", num);
}
close(fd[0]);