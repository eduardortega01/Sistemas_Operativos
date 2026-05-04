# Guía de Sistemas Operativos - Señales, Tuberías y Memoria Compartida 🚀
<br>

# Punteros

### ¿Qué es un puntero? 

Es una variable que guarda una dirección de memoria de otra variable.
En vez de almacenar directamente un valor, almacena dónde está ese valor.
<br>

# 🧠 Memoria dinámica

La **memoria dinámica** permite reservar memoria mientras el programa se está ejecutando, en lugar de definir de antemano cuánta se necesita. 
Esto es útil cuando la cantidad de datos no se conoce hasta el momento de la ejecución (por ejemplo, cuando el usuario ingresa cuántos elementos quiere guardar).

### Funciones principales

Estas funciones están en la librería <stdlib.h>

- **malloc()**: Asigna memoria, reserva un bloque de memoria de un tamaño específico. 

- **calloc()**: Asigna memoria inicializada en 0, similar a **malloc**, pero llena con ceros. 

- **realloc()**: Cambia el tamaño de una memoria ya reservada, permite agrandar o reducir el bloque.

- **free()**: Libera memoria, devuelve al sistema la memoria que ya no usa. 
<br>

# Matriz dinámica

En C una matriz dinámica se implementa como: 

Un arreglo de punteros, donde cada puntero apunta a un fila de enteros.

¿Qué representa cada *? 

- int *: Puntero a entero (una fila)
- int **: Puntero a puntero -> matriz

# Archivos

La librerías que usamos en C para lectura de archivos son: stdio.h y string.h

### 📌 ¿Qué es FILE * ?

FILE *: es un puntero a una estructura que representa un archivo en C.

allí guarda: 
- La posición donde vas leyendo
- Permisos (lectura, escritura, etc)
- información del archivo

Entonces cuando declaras un FILE *archivo no estas creando un archivo, solo el puntero para manejarlo.

### 📌 ¿Qué hace fopen() ?

fopen() abre un archivo del disco y le asigna ese FILE *. 

recibe dos parámetros el nombre del archivo con su extención y el modo en que se abre el archivo. 

Si fopen() abre bien el archivo, el puntero tendrá una dirección válida. 
Si hay error (no existe, permisos, etc.), devuelve NULL.
Siempre se debe verificar si abrió bien, para evitar que el programa caiga al intentar abrir algo que no existe.

### Modos de aperturas

- "r": Leer, si no existe devuelve NULL. 
- "w": Escribir (borra contenido anterior), si no existe lo crea. 
- "a": Agregar al final, si no existe lo crea. 
- "r+": Leer + escribir, si no existe devuelve NULL.
- "w+": Leer + escribir (borra antes), si no existe lo crea
- "a+": Leer + agregar, si no existe lo crea.

### fclose()

Cuando se termina de leer un archivo se debe cerrar con esta función, si se deja abierto podrías perder datos o bloquear el archivo. 

### fscanf()

La función fscanf() se usa para leer datos formateados directamente desde un archivo y almacenarlos en variables. 

recibe el nombre del archivo, el formato, y la variable

¿Qué hace? 

- Lee datos del archivo siguiendo un formato
- Convierte el texto leído a tipos numéricos o caracteres
- Avanza automáticamente dentro del archivo o a medida que lee (lee una linea y da el salto a la siguiente posición, es decir, a la línea que sigue)
- Guarda el dato en la variable correspondiente

### fgets() 

La función fgets() en C se usa para leer una cadena de texto (string) desde un archivo o desde la entrada estándar (stdin, o sea, el teclado).

Su sintaxis es esta:
char *fgets(char *buffer, int size, FILE *stream);

Parámetros: 
-  1. buffer: Es un arreglo de caracteres donde se guardará el texto leído.

- 2. size: Es el tamaño máximo de caracteres que se van a leer, incluyendo el carácter nulo '\0' que marca el final de la cadena.
Por ejemplo, si pones size = 256, fgets() leerá como máximo 255 caracteres y usará uno para '\0'.

- 3. stream: Es el origen desde donde se leerá. 
     * Si es stdin, leerá desde el teclado.
     * Si es un archivo abierto, leerá desde ese archivo.

¿Qué hace? 

fgets():

- Lee caracteres hasta que:

    * Encuentra un salto de línea ('\n'),

    * o llega al fin del archivo (EOF),

    * o ha leído size - 1 caracteres.

- Luego agrega automáticamente '\0' al final de la cadena.
<br>

# 📘 Señales

### 🧠 ¿Qué es una señal?

Una señal (signal) es un mensaje muy simple que el sistema operativo usa para comunicar eventos a un proceso. 

Piensa en una señal como un golpecito en hombro que el sistema le da a un proceso para decirle: 

"Oye, acaba de pasar algo, reacciona."

### ¿Qué pasa cuando llega una señal?

Cuando un proceso recibe una señal: 
1. El sistema operativo interrumpe la ejecución normal del proceso. 

2. Busca qué debe hacer con esa señal: 
*¿Tiene un manejador definido por el usuario? ✅ -> lo ejecuta. 
*¿Debe usar la acción por defecto? ⚙️ -> la aplica (por ejemplo, terminar el proceso).
*¿Debe ignorarla? 🚫 -> la descarta.

Una vez que se maneja la señal, el proceso continúa donde estaba.

### Librerías necesarias

- signal.h: Sirve para el manejo de señales -> signal(), nombres como SIGUSR1
- Unistd.h: fork(), getpid(), kill(), puase()
- sys/types.h: pid_t 
- wait.h: wait() o waitpid() cuando un padre espera a sus hijos

### Enviar señales

kill(pid_destino, señal); 

No mata el proceso, solo envía una señal.

### Bloquearse esperando una señal

Para que no termine antes de recibir: 
pause();

¿Qué hace pause()? 
pause() pone al proceso en estado bloqueado (sleep) hasta que llegue una señal.

- No consume CPU
- No ejecuta nada hasta que una señal lo despierte
- Es perfecto para protocolos basados en señales

# Funciones auxiliares

- usleep(): Se usa para sincronizar.
- sleep(): Cada proceso debe instalar sus manejadores antes de recibir señales. El sleep(1) garantiza que los proceso hijos han instalado sus manejadores de señales antes de que el npadre comience a enviar la señales. Si el padre enviara señales antes de eso, se perderían porque los procesos hijos aún no estarían preparados. Es una sincronización inicial simple entre procesos.
- while(): Se usa para dejar vivo el proceso, junto con pause() asegura que solo actúa cuando le llega una señal. Sin el while el programa terminaría después de la primera señal. 
- fflush(stdout): para imprimir en tiempo real

### ¿Por qué usamos while(1) pause();? 

Explicación en una frase para sustentación:

Los procesos están coordinados únicamente por señales. Para evitar que se ejecuten instrucciones por fuera del protocolo y para que no consuman CPU, usamos un loop infinito con pause(), que bloquea al proceso hasta que llegue una señal. Es la forma correcta de implementar un canal de comunicación por señales.

¿Qué pasaria si no usas pause()?

El proceso seguiría ejecutando el código normal y podría finalizar antes de recibir señales, rompiedo el protocolo.

¿Qué pasaría si saco el whilw? 

solo recibiría una señal y luego terminaría; no escucharía el resto de la matriz.

¿Qué diferencia hay con un sleep()?

Sleep bloquea por tiempo fijo, pause() solo se despierta con una señal real.

# Tuberias

### ¿Qué es una tubería? 

una tubería (pipe) es un mecanismo de comunicación entre procesos (IPC) que permite que dos procesos relacionados (padre-hijo) intercambien datos. La salida de un proceso se convierte en la entrada de otro. 

### Características principales: 

- Sin nombre: No tienen presencia en sistema de archivos
- Unidireccional: Los datos fluyen en una sola dirección
- FIFO: First In, First Out (cola)
- Comunicación entre procesos relacionados: Solo padre-hijo o hermanos
- buffer en memoria: el kernel administra un buffer limitado
- Temporal: Existe solo mientras los procesos estén vivos
- Sincronización automática: El SO maneja lectura/escritura

### ¿Por qué son "anónimas"? 

No tienen nombre ni ruta en el sistema de archivos. Solo existen en memoria y se acceden mediante descriptores de archivo. 

### Creación de tuberías

la llamada al sistema pipe()
Code tuberias.c

### Uso con fork()

Patrón básico padre-hijo

El patrón estándar es: 

- 1. Crear la tubería con pipe()
- 2. Crear proceso hijo con fork()
- 3. Cerrar extremos no usados
- 4. Leer/escribir según corresponda
- 5. Cerrar extremos usados

Code padre-hijo.c

### ¿Por qué cerrar descriptores? 

REGLA DE ORO: Cada proceso debe cerrar el extremo que no usa. 

Razones: 

- 1. Evitar bloqueos: Si el lector no cierra fd[1], el read() nunca recibirá EOF
- 2. Liberar recursos: Los descriptores son recursos limitados
- 3. Detector EOF: El lector recibe EOF solo cuando TODOS los fd[1] están cerrados
- 4. Buenas práticas: Código limpio y predecible

### Funciones 

write(int fd, const void *buf; size_t count); 

- fd: descriptor del archivo donde escribir
- buf: puntero al buffer de datos
- count: cantidad de bytes a enviar 

### Funciones auxiliares

- strlen(): es una función de la biblioteca estándar de C que calcula la longitud de un cadena de caracteres (strign).

- ssize_t: ssize_t es un tipo de dato entero con signo (es decir, puede ser positivo o negativo). Viene definido en <sys/types.h> y su tamaño depende del sistema (normalmente es equivalente a long o long long). 

Las funciones read() y write() están declaradas así: 

ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

- size_t (sin signo) se usa para indicar cuántos bytes quieres leer o escribir (no puede ser negativo).
- ssize_t (con signo) se usa como valor de retorno, porque:
    - Si todo sale bien, devuelve la cantidad de bytes leídos o escritos (un número positivo).
    - Si ocurre un error, devuelve -1.

# Funciones importantes para el examen

### Para búsqueda de cadenas: 

- strstr(linea, "texto") -Busca una subcadena, retorna puntero o NULL
- strchr(linea, ':') -Busca un carácter, retorna puntero o NULL
- strcmp(str1, str2) -Compara cadenas exactas (retorna 0 si son iguales)
- strncmp(str1, str2, n) -Compara los primeros n caracteres 

### Para dividir strings: 

- strtok(linea, ":") -Divide por delimitador (modifica el string origina)

### Para lectura: 

- fgets(buffer, tamaño, archivo) -Lee línea completa (incluye\n)
- fscanf(archivo, "%s", palabra) -Lee palabra por palabra

## ¿Qué hace strcspn? 

strcspn significa "string complement span" (extensión complementaria de cadena).

Retorna: La longitud (número de caracteres) del segmento inicial de una cadena que NO contiene nigundo de los caracteres especificados. 

## Sintaxis: 

size_t strcspn(const char *str, const char *reject); 

## Uos principal: Eliminar el \n de fgets

Cuando usas fgets(), la línea incluye el salto de línea \n al final. strcspn es la forma más elegante de quitarlo: 

char linea[256];
fgets(linea, sizeof(linea), archivo);

// Reemplazar \n con \0
linea[strcspn(linea, "\n")] = '\0';

### ¿Por qué funciona? 

- strcspn(linea, "\n") busca dónde está el primer \n
- Retorna el índice de ese carácter
- Lo reemplazamos con \0 (terminador de cadena)

## En resumen: 

strcspn es perfecta para: 

- Quitar el \n de fgets (uso más común) 
- Encontrar el primer delimitador de varios posibles
- Validar formatos de entrada
- Extraer substrings hasta un carácater especial 

## ¿Qué hace strcpy?

strcpy significa "string copy" (copiar cadena).

Función: Copia una cadena de caracteres de un lugar a otro, incluyendo el carácter nulo \0 al final.

## Sintaxis:

char *strcpy(char *destino, const char *origen);

- destino: Donde se copiará la cadena (debe tener espacio suficiente)
- origen: La cadena que se va a copiar
- Retorna: Un puntero al destino

##  IMPORTANTE - Peligros de strcpy:

strcpy NO verifica si hay espacio suficiente en el destino. Esto puede causar buffer overflow (desbordamiento de buffer), un error muy común en exámenes.