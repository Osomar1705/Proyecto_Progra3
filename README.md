# Plataforma de Streaming — Búsqueda por sub-palabras y frases

Programación III · Proyecto Final (2026-1)

## Integrantes
- Osmar Vilchez Aguirre
- Royer Sebastian Ramos Vargas
- Luciana Mylene Melgarejo Quispe

## Estructura del proyecto
```
include/
  Movie.h        # Struct Movie (id, title, director, cast, genre, plot, clean_words)
  Processor.h    # Lectura/limpieza del CSV
  Trie.h         # Trie de Sufijos
src/
  Movie.cpp
  Processor.cpp  # Parser CSV (comillas escapadas y campos multilinea) + tokenizado
  Trie.cpp       # Insert/search/destroy del Trie de Sufijos
  main.cpp       # CLI: cargar, indexar y buscar
wiki_movie_plots_deduped.csv
```

## Bloque 1 — Pre-procesamiento
- `Processor::loadMovies` lee el CSV soportando:
  - Comillas escapadas `""` dentro de campos quoted.
  - Registros que abarcan varias líneas físicas (los plots de Wikipedia contienen `\n`).
- `Processor::cleanAndSplitText` baja a minúsculas, descarta puntuación y filtra *stop words* en inglés.
- Cada `Movie.clean_words` agrega los tokens de **título, director, género y sinopsis**, de modo que la búsqueda funciona contra cualquiera de esos campos.

## Bloque 2 — Estructura de datos elegida: **Trie de Sufijos**

### Por qué un Trie de Sufijos (no un Trie clásico)
El proyecto exige dos cosas que un Trie *normal* no resuelve bien:

1. **Búsqueda de sub-palabras en cualquier posición**: la consulta `bar` debe encontrar `barco`, `embarcar`, `cobarde`, etc. Un Trie clásico sólo indexa **prefijos** — `bar` encontraría `barco` pero nunca `embarcar`.
2. **Búsqueda de frases**: dada una consulta de varios términos, todos deben coincidir contra alguna palabra indexada de la película.

La solución es un **Trie de Sufijos**: para cada palabra `W` de longitud *n* se insertan sus *n* sufijos
(`W[0..]`, `W[1..]`, …, `W[n-1..]`). Cualquier substring de `W` es prefijo de algún sufijo de `W`,
así que descender por la consulta carácter a carácter llega exactamente a los nodos correctos.

### Cómo funciona en este proyecto
- **Nodo** (`TrieNode`): un mapa `char → TrieNode*` y un `unordered_set<int>` con los IDs de películas que
  contienen ese substring. El `unordered_set` evita duplicados cuando varios sufijos de la misma película
  pasan por el mismo nodo.
- **Inserción** (`Trie::insert(word, movieId)`): recorre los `n` sufijos. En cada nodo del camino agrega
  `movieId` al set, de modo que **cualquier prefijo de cualquier sufijo** (= cualquier substring) recupere
  la película.
- **Búsqueda** (`Trie::search(query)`): desciende por el Trie siguiendo los caracteres de `query`. Si en
  algún paso no existe el hijo, no hay coincidencias. Si llega al final, devuelve los IDs del nodo.
- **Frases** (`searchAll` en `main.cpp`): tokeniza la consulta con la misma limpieza del Bloque 1 y devuelve
  la **intersección** de los resultados por token — una película coincide solo si contiene **todos** los términos.

### Complejidad
| Operación | Complejidad |
|---|---|
| Insertar una palabra de longitud *n* | O(n²) tiempo, O(n²) nodos en el peor caso |
| Buscar un substring de longitud *m* | **O(m)** — independiente del tamaño del corpus |
| Buscar una frase de *k* tokens (longitud total *L*) | O(L + R), con R = tamaño del resultado parcial |

El intercambio es claro: pagamos **memoria** durante la indexación para que la **búsqueda sea proporcional
solo a la longitud de la consulta**, no al tamaño del dataset (~35 000 películas).

### Alternativas consideradas
- **Trie de prefijos clásico**: descartado, no resuelve sub-palabras en posiciones interiores.
- **Suffix Array / FM-Index**: más compactos en memoria, pero la implementación queda fuera del alcance del curso.
- **Búsqueda lineal con `find`**: O(N · |texto|) por consulta — inviable para un dataset grande.

## Compilar y ejecutar
```bash
cmake -S . -B build
cmake --build build
./build/streaming_platform
```
Menú:
1. Cargar datos → ruta del CSV (`wiki_movie_plots_deduped.csv`).
2. Buscar → escribe la consulta (sub-palabra o frase).
3. Salir.
