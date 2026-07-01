# Programación III: Proyecto Final - Plataforma de Streaming

## Integrantes
- Osmar Vilchez Aguirre
- Royer Sebastian Ramos Vargas
- Luciana Mylene Melgarejo Quispe

## Descripción del Proyecto
Este proyecto es una plataforma de búsqueda y administración de películas basada en un conjunto de datos real (`wiki_movie_plots_deduped.csv`).

El sistema carga los datos de películas, procesa texto para eliminar ruido, indexa la información en un Trie de sufijos y ofrece una interfaz de consola para buscar, ver detalles, marcar como "Like" y agregar películas a "Ver más tarde".

## Estado actual
- ✅ Carga de datos desde CSV
- ✅ Pre-procesamiento de texto
- ✅ Indexación en Trie de sufijos
- ✅ Búsqueda por palabra, frase o sub-palabra
- ✅ Visualización de resultados con paginación
- ✅ Visualización de detalles de película
- ✅ Funciones "Like" y "Ver más tarde"
- ✅ Recomendaciones básicas basadas en gustos

---

## 1. Pre-procesamiento de datos

El pre-procesamiento se realiza en `src/DataProcessor.cpp` mediante la clase `DataProcessor`.

### Pasos realizados
1. Convertir todo el texto a minúsculas.
2. Eliminar caracteres no alfanuméricos.
3. Separar el texto en palabras (tokenización).
4. Eliminar palabras muy cortas y stop-words en inglés y español.
5. Generar tres listas por película:
   - `clean_words`: palabras relevantes extraídas de `title + director + genre + plot`
   - `clean_title`: palabras del título
   - `clean_genre`: palabras del género

Estas listas quedan listas para ser ingresadas a la estructura de datos.

---

## 2. Pseudo-código de ingreso a la estructura

```text
Para cada película P en la lista de películas:
    movieIndexMap[P.id] = índice de P
    Para cada palabra W en P.clean_words:
        Para i desde 0 hasta longitud(W) - 1:
            sufijo = W.subcadena(i)
            Trie.insertar(sufijo, P.id)
```

Dentro de `Trie.insertar(sufijo, movieId)`:

```text
actual = root
Para cada carácter c en sufijo:
    si c no está en actual.children:
        crear nuevo nodo hijo
    actual = actual.children[c]
    si movieId no está en actual.movieIds:
        agregar movieId
marcar actual como fin de palabra
```

---

## 3. Funcionamiento de la estructura de datos

### Estructura elegida
Se eligió un **Trie de sufijos**.

### Por qué un Trie de sufijos
- Permite búsquedas rápidas en tiempo O(L), donde L es el largo de la consulta.
- Al insertar sufijos, permite encontrar coincidencias de sub-palabras.
- Es más eficiente que buscar en cada película uno por uno.

### Inserción
- Para cada palabra limpia de una película se insertan todos sus sufijos.
- Esto permite que la consulta "arc" encuentre palabras como "barco".
- Cada nodo almacena una lista de IDs de películas que contienen ese prefijo.

### Búsqueda
- La búsqueda solicita un término o frase.
- La consulta se limpia y se divide en tokens.
- Cada token se busca en el Trie.
- El resultado es la lista de películas asociadas a ese token.
- Se cuentan coincidencias y se ordenan por puntaje.

---

## 4. Interfaz del programa

La interfaz actual es de consola y ofrece:
- Menú principal con opciones numeradas.
- Carga de datos mediante ruta de archivo.
- Búsqueda de películas por palabra, frase o sub-palabra.
- Paginación de resultados en bloques de 5.
- Selección de película para ver detalles completos.
- Opciones de `Like` y `Ver más tarde`.
- Visualización de la lista completa de "Ver más tarde".
- Recomendaciones basadas en las películas marcadas con "Like".

### Ejemplo de uso
1. Ejecutar el programa.
2. Seleccionar `1` para cargar el CSV.
3. Seleccionar `2` para buscar películas.
4. Ingresar un término de búsqueda.
5. Seleccionar una película de la lista.
6. Elegir `1` para dar "Like" o `2` para agregar a "Ver más tarde".

---

## 5. Cómo compilar y ejecutar

Desde la raíz del proyecto:

```bash
g++ -std=c++17 -Iinclude src/DataProcessor.cpp src/Trie.cpp src/main.cpp -o streaming_platform.exe
./streaming_platform.exe
```

---

## 6. Observaciones finales

- El proyecto está listo como una interfaz de consola funcional.
- Cumple con la evaluación de pre-procesamiento, estructura de datos, pseudocódigo, algoritmo e interfaz.
- Para una versión futura se puede mejorar con una interfaz gráfica o búsqueda más avanzada.

---

## 7. Rendimiento y Programación Paralela

Se ha utilizado programación paralela (con la librería de C++) para optimizar la carga y pre-procesamiento de los datos del archivo CSV. A continuación, se presenta una tabla comparativa de los tiempos de carga:

| Tipo de Carga | Tiempo promedio (ms) |
|---|---|
| Carga Secuencial | ~7172 ms |
| Carga Paralela | ~1274 ms |

Como se observa, la carga paralela reduce significativamente el tiempo necesario para leer y procesar los registros de películas antes de insertarlos en el Trie de Sufijos.
