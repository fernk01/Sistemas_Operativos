# malloc

Lugar para respuestas en prosa y documentación del TP.

## Estructura del código.
El código está organizado en varios archivos:

- `printfmt.h` y `printfmt.c`: Definen la función `printfmt` que se utiliza para imprimir datos formateados en la consola sin usar la función `printf`.
- `testlib.h` y `testlib.c`: Proporcionan funciones y macros auxiliares para ejecutar las pruebas y mostrar los resultados de manera legible en la consola.
- `malloc.h` y `malloc.c`: Contienen la implementación de las funciones de asignación y liberación de memoria (malloc, free, calloc, realloc).
- `malloc.test.c`: Contiene las pruebas específicas para cada función y casos límite.

# Diseño de la estructura.
Se utiliza un conjunto de arrays de punteros a la estructura `region` (`small_blocks`, `medium_blocks`, `large_blocks`) para organizar los bloques de memoria en diferentes tamaños predefinidos, con un máximo de `MAX_BLOCKS` bloques de memoria por categoría. Esto permite un acceso rápido a los bloques de memoria del tamaño adecuado durante la asignación y la búsqueda de regiones libres. Cada índice del array actúa como un bloque de memoria.

La estructura `region` se utiliza para crear una lista enlazada de regiones de memoria. Cada región está conectada mediante punteros `next` y `prev`, lo que facilita la búsqueda y la fusión de regiones contiguas cuando se liberan. Estos punteros permiten un acceso eficiente a los bloques de memoria adyacentes y simplifican las operaciones de gestión de memoria.

Además, se ha definido un tamaño mínimo (`MIN_SIZE`) que siempre se devuelve, incluso cuando el usuario solicita una cantidad menor de memoria. Esto garantiza que se asigne un espacio mínimo para cada solicitud y ayuda a evitar asignaciones excesivamente pequeñas que puedan afectar el rendimiento.

# Diseño de Malloc:
La función `malloc` para asignar bloques de memoria dinámicamente. En primer lugar, verifica si el tamaño solicitado es válido y si hay suficiente memoria disponible en el sistema. Luego, alinea el tamaño solicitado a múltiplos de 4 bytes para garantizar una correcta alineación de memoria.

A continuación, busca una región libre adecuada llamando a la función `find_free_region`, que utiliza una estrategia de ajuste ("First Fit" o "Best Fit") para encontrar un bloque de memoria disponible que pueda satisfacer la solicitud. Si encuentra una región adecuada, la divide en dos partes: una parte asignada y otra libre, si hay suficiente espacio.

Si no se encuentra una región libre adecuada, se crea un nuevo bloque de memoria utilizando la función `create_block`, que utiliza la función `mmap` para asignar un nuevo bloque de memoria y lo agrega a la lista correspondiente según su tamaño.

Finalmente, se actualizan las estadísticas de seguimiento, se incrementa el contador de asignaciones (`amount_of_mallocs`) y se registra la cantidad de memoria solicitada (`requested_memory`).

# Diseño de Free:
Primero, verifica la validez del puntero proporcionado para asegurarse de que apunte a una región válida del bloque de memoria. Luego, comprueba si existen regiones adyacentes a la derecha e izquierda del bloque a liberar que también estén marcadas como libres. Si es así, combina estas regiones en una sola región más grande mediante el proceso de coalescencia. 

Verifica cuando el bloque a liberar es el único bloque en su región o cuando es el último. En estos casos, se realiza la liberación de memoria correspondiente utilizando la función `munmap` para devolver la memoria al sistema operativo.

# Diseño encontrar una región libre:  First fit y Best fit
En First Fit, se busca la `primera` región libre que tenga un tamaño igual o mayor al tamaño solicitado. El algoritmo recorre la lista de bloques de memoria disponibles y, dentro de cada bloque, busca una región libre que cumpla con los requisitos de tamaño. Si se encuentra una región libre que pueda acomodar el tamaño solicitado, se utiliza esa región para asignar la memoria. Si la región libre es más grande que el tamaño solicitado, se divide en dos regiones, una para la asignación solicitada y otra para el resto del espacio libre.

En Best Fit, se busca la región libre que tenga el tamaño `más cercano` al tamaño solicitado. El algoritmo recorre la lista de bloques de memoria disponibles y, dentro de cada bloque, busca la región libre cuyo tamaño sea el más cercano al tamaño solicitado. Si se encuentra una región libre que tenga el tamaño exacto, se utiliza esa región para asignar la memoria. De lo contrario, se selecciona la región libre más cercana en tamaño y se divide en dos regiones, una para la asignación solicitada y otra para el resto del espacio libre.

El First Fit es más rápido en términos de tiempo de búsqueda, ya que simplemente busca la primera región libre que cumpla con los requisitos. Sin embargo, puede resultar en fragmentación de la memoria, ya que asigna la primera región libre disponible, incluso si hay una región más pequeña pero adecuada para la asignación.

Por otro lado, el Best Fit ofrece una asignación más eficiente en términos de aprovechamiento del espacio libre, ya que busca la región libre que se ajuste de manera más cercana al tamaño solicitado. Sin embargo, requiere una búsqueda más exhaustiva, lo que puede aumentar el tiempo de asignación.

# Diseño Calloc:
La función `calloc` combina la funcionalidad de asignar memoria utilizando `malloc` y luego inicializarla a cero utilizando `memset`.

# Diseño Realloc:
La función `realloc` se utiliza para cambiar el tamaño de un bloque de memoria previamente asignado. En primer lugar, se realizan algunas verificaciones especiales. Si el puntero proporcionado es `NULL`, la función se comporta como `malloc` y asigna un nuevo bloque de memoria del tamaño especificado. Si el tamaño especificado es cero, la función se comporta como `free` y libera el bloque de memoria apuntado por el puntero.

Después de las verificaciones iniciales, el tamaño especificado se alinea a múltiplos de 4 bytes y se verifica si excede el tamaño máximo permitido para un bloque grande. En caso afirmativo, se asigna `errno = ENOMEM` para indicar que no se puede asignar la cantidad de memoria solicitada.

Luego, se obtiene la estructura `region` correspondiente al bloque de memoria apuntado por el puntero. Si el tamaño especificado es menor o igual al tamaño actual de la región, se crea una nueva región más pequeña utilizando la función `create_region`, manteniendo la ubicación original del bloque de memoria.

Si el tamaño especificado es mayor que el tamaño actual de la región, se verifican dos escenarios. Primero, se verifica si la memoria contigua a la derecha está libre y si la memoria resultante (sumando los tamaños de ambas regiones más el tamaño del encabezado) es suficiente para acomodar el tamaño solicitado. En ese caso, se fusiona la región actual con la región contigua de la derecha utilizando la función `coalesce_right`, y luego se crea una nueva región con el tamaño especificado utilizando `create_region`.

Si ninguno de los escenarios anteriores se cumple, se asigna un nuevo bloque de memoria utilizando `malloc` y se copia el contenido del bloque original al nuevo bloque utilizando `memcpy`. Luego, el bloque original se libera utilizando `free`.

# Test:
Las variables `amount_of_mallocs`, `amount_of_frees` y `requested_memory` son utilizadas para rastrear y obtener información sobre el uso de la memoria en el programa.
A las pruebas unitarias prevista por la cátedra se agregaron las pruebas de casos borde y pruebas de funcionalidad.