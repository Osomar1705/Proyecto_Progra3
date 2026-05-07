#include "Processor.h"

#include <string>
#include <vector>

std::vector<Movie> Processor::loadMovies(const std::string& filepath) {
    // TODO: abrir el archivo CSV en `filepath`, parsear linea por linea y
    // construir un vector<Movie> con id, title, synopsis, tags y ranking.
    (void)filepath;
    return {};
}

std::vector<std::string> Processor::cleanString(const std::string& input) {
    // TODO: normalizar el string (minusculas, eliminar puntuacion, separar
    // por espacios) y devolver el vector de tokens listos para indexar.
    (void)input;
    return {};
}
