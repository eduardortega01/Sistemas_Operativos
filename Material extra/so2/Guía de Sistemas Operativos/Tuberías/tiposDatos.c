// Enviar cadenas (strings) 
// ESCRITOR
char mensaje[] = "Hola mundo";
write(fd[1], mensaje, strlen(mensaje) + 1);  // +1 para '\0'

// LECTOR
char buffer[100];
read(fd[0], buffer, sizeof(buffer));
printf("Recibido: %s\n", buffer);

// Enviar Enteros
// ESCRITOR
int numero = 42;
write(fd[1], &numero, sizeof(int));

// LECTOR
int num_recibido;
read(fd[0], &num_recibido, sizeof(int));
printf("Número: %d\n", num_recibido);

// Enviar estructuras
struct Persona {
    char nombre[50];
    int edad;
};

// ESCRITOR
struct Persona p = {"Juan", 25};
write(fd[1], &p, sizeof(struct Persona));

// LECTOR
struct Persona p_recibida;
read(fd[0], &p_recibida, sizeof(struct Persona));
printf("Nombre: %s, Edad: %d\n", p_recibida.nombre, p_recibida.edad);

//Enviar arrays
// ESCRITOR
int numeros[] = {1, 2, 3, 4, 5};
write(fd[1], numeros, sizeof(numeros));

// LECTOR
int nums[5];
read(fd[0], nums, sizeof(nums));
for(int i = 0; i < 5; i++) {
    printf("%d ", nums[i]);
}