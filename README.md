# Programación III: Proyecto Final - Plataforma de Streaming

## Integrantes
- Osmar Vilchez Aguirre
- Royer Sebastian Ramos Vargas

## Descripción del Proyecto
Este proyecto es una plataforma de búsqueda y administración de películas basada en un conjunto de datos real (`wiki_movie_plots_deduped.csv`, 34 886 películas).

El sistema carga los datos de películas, procesa texto para eliminar ruido, los indexa en un índice invertido + un índice de sufijos (Trie), y ofrece una interfaz de consola para buscar por **palabra, frase o sub-palabra**, buscar por **tags** (director / reparto / género), ver detalles, marcar como "Like" y agregar a "Ver más tarde". La sesión (likes + ver más tarde) se **persiste entre ejecuciones**.

## Estado actual
- ✅ Carga de datos desde CSV (con soporte de campos multilínea entrecomillados)
- ✅ Pre-procesamiento de texto (stop-words ES/EN)
- ✅ Indexación: índice invertido (vocabulario único) + Trie de sufijos
- ✅ Búsqueda por palabra, frase o sub-palabra
- ✅ Búsqueda por tag: `director:`, `cast:`, `genre:`
- ✅ Algoritmo de importancia propio para rankear resultados
- ✅ Visualización de resultados con paginación (5 por página)
- ✅ Funciones "Like" y "Ver más tarde"
- ✅ Recomendaciones basadas en gustos (patrón Observer)
- ✅ Persistencia de la sesión entre ejecuciones

---

## 1. Pre-procesamiento de datos

El pre-procesamiento se realiza en `src/DataProcessor.cpp` mediante la clase `DataProcessor` (patrón **Singleton**).

### Pasos realizados
1. Leer el CSV respetando campos entrecomillados que abarcan **varias líneas físicas** (`readRecords`); una sinopsis con saltos de línea es un solo registro.
2. Convertir todo el texto a minúsculas.
3. Eliminar caracteres no alfanuméricos.
4. Separar el texto en palabras (tokenización).
5. Eliminar palabras muy cortas y stop-words en inglés y español.
6. Generar tres listas por película:
   - `clean_words`: palabras relevantes de `title + director + cast + genre + plot`
   - `clean_title`: palabras del título
   - `clean_genre`: palabras del género

> El reparto (`cast`) se incluye en el índice para poder buscar películas por actor, como pide el enunciado (búsqueda por tag de casting).

Estas listas quedan listas para ser ingresadas a la estructura de datos.

---

## 2. Pseudo-código de ingreso a la estructura

```text
Para cada película P en la lista de películas:
    movieIndexMap[P.id] = índice de P
    Para cada palabra W en P.clean_words:
        termId = índiceInvertido.registrar(W, P.id)   # vocabulario único + posting list

Para cada término único T del vocabulario:
    Para i desde 0 hasta longitud(T) - 1:
        Trie.insertar(subcadena(T, i), termId(T))     # sufijos del término, no de cada ocurrencia
```

Dentro de `Trie.insertar(sufijo, termId)`:

```text
actual = root
Para cada carácter c en sufijo:
    si c no está en actual.children:
        crear nuevo nodo hijo (unique_ptr)
    actual = actual.children[c]
al llegar al final del sufijo (nodo terminal):
    agregar termId a actual.termIds
```

---

## 3. Funcionamiento de la estructura de datos

### Estructura elegida (dos niveles)
1. **Índice invertido** (`InvertedIndex`): vocabulario deduplicado (146 631 términos únicos), cada término con una *posting list* ordenada de IDs de película. Evita repetir el término por cada aparición.
2. **Trie de sufijos** (`Trie`): se insertan los sufijos de cada **término único** (no de cada ocurrencia). El payload (termId) vive **solo en el nodo terminal**, manteniendo los nodos intermedios en O(1). Los nodos se gestionan con `std::unique_ptr` (destrucción automática, sin `delete` manual).

### Por qué esta estructura
- Búsqueda en O(L), con L = largo de la consulta.
- Al indexar sufijos, encuentra coincidencias de **sub-palabra**: buscar `bar` encuentra "barco", "Zanzibar", "Barbara".
- Separar vocabulario (índice invertido) de la búsqueda por sufijos ahorra memoria frente a insertar los sufijos de cada aparición.

### Búsqueda
1. La consulta se limpia y se divide en tokens.
2. Cada token desciende por el Trie hasta su nodo (O(L)) y recolecta por DFS los termIds del subárbol.
3. Las posting lists del índice invertido resuelven las películas.
4. Se aplica el algoritmo de importancia (sección 3.1) y se ordena.

### 3.1 Algoritmo de importancia (ranking propio)

Por cada token de la consulta, una película acumula:

| Condición | Puntos |
|---|---|
| El token aparece como sub-palabra en cualquier campo indexado | +1 |
| Además, el token es **palabra exacta** de la película (`std::binary_search` sobre la posting list) | +2 |
| Además, el token aparece dentro de una palabra del **título** | +3 |

Se ordena por puntaje descendente; el empate se resuelve por el orden original del dataset. Ejemplo: buscando `bar`, las primeras posiciones son títulos con "bar" (*Wonder Bar*, *Bar 20*...), no coincidencias sueltas en la sinopsis.

La búsqueda por tag (`director:nolan`, `cast:jolson`, `genre:comedy`) puntúa por cantidad de tokens presentes en el campo elegido.

---

## 4. Patrones de diseño (4)

| Patrón | Dónde | Rol |
|---|---|---|
| **Singleton** | `DataProcessor::getInstance()` | Única instancia del procesador; comparte la tabla de stop-words |
| **Strategy** | `IRecommendationStrategy` / `GenreKeywordRecommendation` | Algoritmo de recomendación intercambiable |
| **Observer** | `LikeSubject` (subject + lista de observadores + `notify`) → `RecommendationEngine` | Al cambiar los likes, recalcula recomendaciones automáticamente |
| **Facade** | `StreamingPlatform` | Punto de entrada único: carga, índices, búsqueda y persistencia |

---

## 5. Interfaz del programa

Interfaz de consola con:
- Menú principal numerado.
- Carga de datos por ruta de archivo.
- Búsqueda por palabra, frase, sub-palabra o tag.
- Paginación de resultados en bloques de 5.
- Detalle de película con `Like` y `Ver más tarde`.
- Lista completa de "Ver más tarde".
- Recomendaciones basadas en los "Like".
- **Al iniciar**, restaura la sesión previa (recarga el CSV y muestra "Ver más tarde" y recomendaciones desde la primera pantalla).

### Ejemplo de uso
1. Ejecutar el programa.
2. Seleccionar `1` para cargar el CSV.
3. Seleccionar `2` para buscar (p. ej. `bar`, o `cast:jolson`).
4. Elegir una película de la lista.
5. `1` para "Like" o `2` para "Ver más tarde".
6. Al reabrir el programa, la sesión se restaura automáticamente.

---

## 6. Cómo compilar y ejecutar

Desde la raíz del proyecto:

```bash
# g++ / MinGW
g++ -std=c++17 -O2 -pthread -Iinclude src/*.cpp -o streaming_platform

# o con CMake
cmake -S . -B build && cmake --build build

./streaming_platform
```

Al cargar datos, indicar la ruta al CSV (p. ej. `wiki_movie_plots_deduped.csv`).
La sesión se guarda en `platform_state.txt` (ignorado por git) y se restaura al iniciar.

---

## 7. Rendimiento y Programación Paralela

`parallel_map` (`include/Utils.h`) es una plantilla genérica (`template <T, U, Func>`) que reparte `f(input[i], i)` entre hilos (`std::thread`) sobre rangos disjuntos de índices, sin data races. Se usa para paralelizar el parseo y la tokenización de los registros del CSV (la lectura de disco sigue siendo secuencial: es I/O).

### Tabla de tiempos (medida real)
Máquina de prueba: CPU de 20 hilos lógicos, Windows 11, g++ 15.2 (MinGW-w64), `-O2`. Dataset: 34 886 películas, 146 631 términos únicos. El programa imprime esta comparación en vivo en cada carga.

| Etapa | Secuencial | Paralelo (20 hilos) | Speedup |
|---|---|---|---|
| Carga + parseo + tokenización | ~1970 ms | ~530 ms | ~3.7× |
| Indexado (índice invertido + Trie de sufijos) | ~2400 ms | — (serial) | — |

El speedup no es lineal porque la lectura del archivo es secuencial (cuello de botella de I/O, ley de Amdahl): solo se paraleliza el procesamiento.

---

## 8. Observaciones finales

- Interfaz de consola funcional, con control de errores (entrada inválida, EOF, consultas degeneradas) y campos multilínea del CSV.
- Cumple pre-procesamiento, estructura de datos, pseudocódigo, algoritmo de importancia, interfaz, 4 patrones y programación paralela/genérica.
- Trabajo futuro: paralelizar el indexado, `std::partial_sort` para el top-K, y una interfaz gráfica.

---

## 9. Referencias (Formato APA)

* Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2022). *Introduction to Algorithms* (4.ª ed.). MIT Press.
* Gamma, E., Helm, R., Johnson, R., & Vlissides, J. (1994). *Design Patterns: Elements of Reusable Object-Oriented Software*. Addison-Wesley.
* Manning, C. D., Raghavan, P., & Schütze, H. (2008). *Introduction to Information Retrieval*. Cambridge University Press.
* Ukkonen, E. (1995). On-line construction of suffix trees. *Algorithmica, 14*(3), 249–260. https://doi.org/10.1007/BF01206331
* Williams, A. (2019). *C++ Concurrency in Action* (2.ª ed.). Manning Publications.
* cplusplus.com. (s.f.). *C++ Reference*. Recuperado de https://cplusplus.com/reference/
* GeeksforGeeks. (2023). *Trie Data Structure*. Recuperado de https://www.geeksforgeeks.org/trie-insert-and-search/
* Kaggle (proporcionado vía Drive). (2021). *Wikipedia Movie Plots*. Dataset utilizado para la base de datos de películas.
