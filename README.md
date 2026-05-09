# Programación III: Proyecto Final - Plataforma de Streaming

## Integrantes
* Osmar Vilchez Aguirre
* Royer Sebastian Ramos Vargas
* Luciana Mylene Melgarejo Quispe

## Descripción del Proyecto
Este proyecto implementa una plataforma de administración y búsqueda de películas utilizando estructuras de datos avanzadas (Tries) para garantizar una búsqueda rápida y eficiente por palabras, frases o sub-palabras.

## Estructura de Datos: Trie de Sufijos
Se ha seleccionado un **Trie (Árbol de Prefijos)** para almacenar el léxico de las películas. Para cumplir con el requisito de búsqueda por **sub-palabras** (substrings), se insertan todos los **sufijos** de cada palabra relevante (título, género, director, etc.) en el Trie.

### Justificación
* **Eficiencia de Búsqueda:** El Trie permite buscar cualquier prefijo en tiempo O(L), donde L es la longitud de la consulta.
* **Soporte de Substrings:** Al insertar todos los sufijos, una búsqueda de "arc" encontrará "barco" (ya que "arco" es un sufijo de "barco" y "arc" es un prefijo de "arco").
* **Espacio:** Aunque consume más memoria que un Trie simple, es manejable para el volumen de datos de Wikipedia Movie Plots y evita la necesidad de escaneos lineales (O(N)).

## Algoritmos Implementados

### Pre-procesamiento
1. **Tokenización:** El texto se limpia de caracteres no alfanuméricos en la clase `DataProcessor`.
2. **Normalización:** Todo el texto se convierte a minúsculas.
3. **Stop-words:** Se filtran palabras comunes en inglés y español (the, and, el, la, etc.).
4. **Optimización:** Las palabras se limpian una sola vez durante la carga del CSV y se almacenan en el objeto `Movie`, quedando listas para la estructura (Trie).

### Inserción (Pseudo-código)
```text
Para cada Película P en la Base de Datos (cargada por DataProcessor):
    Para cada Palabra W en P.clean_words:
        Para i desde 0 hasta longitud(W):
            Sufijo = W.subcadena(i)
            Trie.Insertar(Sufijo, P.ID)
```

### Búsqueda e Importancia
* Se dividen los términos de búsqueda en tokens.
* Se consulta el Trie por cada token.
* **Algoritmo de Importancia:** Cada película recibe un puntaje basado en la cantidad de tokens de búsqueda que contiene. Las películas con mayor puntaje (más coincidencias) aparecen primero.

### Recomendación de Similitud
* Basado en las películas que el usuario marcó con **Like**.
* El algoritmo analiza los géneros más frecuentes y palabras clave en los títulos de las películas "Likeadas".
* Se puntúan las demás películas del catálogo según su coincidencia con estos géneros y palabras clave.

## Guía de Uso
1. **Cargar Datos:** Ingrese la ruta del archivo `wiki_movie_plots_deduped.csv`.
2. **Buscar:** Ingrese palabras, frases o partes de palabras.
3. **Detalles:** Seleccione una película para ver su sinopsis completa.
4. **Interacción:** Use las opciones de 'Like' para recibir recomendaciones y 'Ver más tarde' para organizar su lista.
