# malloc

Lugar para respuestas en prosa y documentación del TP.

# Diseño de la estructura
Se utilizan array de punteros a struct region (`small_blocks`, `medium_blocks`, `large_blocks`) para organizar los bloques de memoria en diferentes tamaños predefinidos. Estos array actúan como índices y permiten un acceso rápido a los bloques de memoria del tamaño adecuado durante la asignación y la búsqueda de regiones libres.

La estructura `region` se utiliza para crear una lista enlazada de regiones de memoria, donde cada region está conectado mediante punteros `next` y `prev`. Esto permite un acceso rápido a los bloques de memoria contiguos y facilita las operaciones de fusión de regiones adyacentes cuando se liberan.

# Diseño de Malloc:
La función `malloc` para asignar bloques de memoria dinámicamente. En primer lugar, verifica si el tamaño solicitado es válido y si hay suficiente memoria disponible en el sistema. Luego, alinea el tamaño solicitado a múltiplos de 4 bytes para garantizar una correcta alineación de memoria.

A continuación, busca una región libre adecuada llamando a la función `find_free_region`, que utiliza una estrategia de ajuste ("First Fit" o "Best Fit") para encontrar un bloque de memoria disponible que pueda satisfacer la solicitud. Si encuentra una región adecuada, la divide en dos partes: una parte asignada y otra libre, si hay suficiente espacio.

Si no se encuentra una región libre adecuada, se crea un nuevo bloque de memoria utilizando la función `create_block`, que utiliza la función `mmap` para asignar un nuevo bloque de memoria y lo agrega a la lista correspondiente según su tamaño.

Finalmente, se actualizan las estadísticas de seguimiento, se incrementa el contador de asignaciones (`amount_of_mallocs`) y se registra la cantidad de memoria solicitada (`requested_memory`).
