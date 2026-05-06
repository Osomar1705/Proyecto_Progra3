# Programación III: Proyecto Final (2026-1)

## Integrantes (3 ó 4)

* Nombre y Apellidos
* Nombre y Apellidos
* Nombre y Apellidos

El siguiente texto debe ser eliminar en su repositorio.

## Plataforma de Streaming

El objetivo del proyecto final es implementar una plataforma de **streaming**. Un programa que administre la **búsqueda y visualización** de la sinopsis de películas. Para ello se debe implementar las siguientes operaciones:

* El programa debe leer la base de datos en forma **.csv**. La base de datos puede ser descargada desde el siguiente \[link] (https://drive.google.com/file/d/1UJkRuCF8UD92W\_DT7S8dXCYzaR\_9wqB\_/view?usp=sharing). El grupo es responsable del **pre-procesamiento de los datos**.
* El programa debe cargar el contenido corregido del archivo en un **Árbol** que permita la búsqueda rápida de una película. Los caracteres (letras y números) deben ser los valores que se almacenen en los nodos del Árbol. Puede utilizar como referencia estructuras como los **Tries**, **Suffix Trees**, etc." La elección del tipo de Árbol queda a criterio del grupo y debe ser justificada y documentada en el repositorio.
* Para buscar una película se debe utilizar una **palabra, frase o sub-palabra**. Ejemplo:

  * Si se busca la palabra "barco", el programa debería encontrar todas las películas en las cuales la palabra "barco" este en el título o sinópsis.
  * Si se busca la frase "barco fantasma", el programa debería encontrar todas las películas en las cuales las palabras "barco" y/o "fantasma" este en el título o sinópsis.
  * Si se busca el string "bar", el programa debería encontrar todas las películas en las cuales el string "bar" este en el título o sinópsis (El string "bar" podría ser parte de una palabra).
* También se debe poder buscar películas por un **Tag**: director, casting, generero, etc.
* Al buscar películas deben de aparecer la cinco más **importantes** y una opción para visualizar las siguientes cinco coincidencias. El grupo **debe implementar un algoritmo** para determinar que pélicula tiene más importancia en una búsqueda.
* Al seleccionar una película, se debe visualizar la sinopsis y las opciones **Like** y **Ver más tarde** .
* Al iniciar el programa la plataforma debería mostrar las películas que fueron añadidas en **Ver más tarde**. Además, se debe visualizar las películas similares a las que el usuario les dio **Like** (implemente su propio algoritmo).

## Requisitos

* Grupos de cuatro personas como máximo y de tres como mínimo. No se aceptarán grupos de dos o una persona.
* Subir el programa a un repositorio en Github. **En el repositorio debe de estar toda la documentación sobre el proyecto**.
* La exposición del proyecto es **Presencial**. En la presentación de la semana 8, las exposiciones son con respecto a los avances que hayan conseguido.
* Todo el programa, desde la lectura hasta la búsqueda de palabras, debe estar en C++.
* Cumplir con la rúbrica del proyecto.
* Fecha de presentación: La semana 8 (avance) y la semana 16 (final).

------------------------

adicionales:

El pre-procesamiento de los datos. Palabras listas para ser ingresadas a la estructura de datos.

Un pseudo-código de la forma en la cual se ingresarán los datos a la estructura

La explicación del funcionamiento de la estructura de datos escogida por el grupo. Esto debe incluir el algoritmo de inserción y búsqueda.

Un avance de la interfaz del programa.

