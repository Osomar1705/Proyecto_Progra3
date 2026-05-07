#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "Movie.h"
#include "Processor.h"
#include "Trie.h"

int main() {
    Processor processor;
    Trie trie;
    std::vector<Movie> movies;

    while (true) {
        std::cout << "\n=== Plataforma de Streaming ===\n";
        std::cout << "1. Cargar datos\n";
        std::cout << "2. Buscar\n";
        std::cout << "3. Salir\n";
        std::cout << "Seleccione una opcion: ";

        int option = 0;
        if (!(std::cin >> option)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Entrada invalida.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (option) {
            case 1: {
                std::cout << "Ruta del archivo CSV: ";
                std::string path;
                std::getline(std::cin, path);
                // TODO: cargar peliculas con processor.loadMovies(path) y
                // poblar el Trie iterando tokens de cleanString().
                (void)path;
                break;
            }
            case 2: {
                std::cout << "Termino de busqueda: ";
                std::string query;
                std::getline(std::cin, query);
                // TODO: tokenizar la query con processor.cleanString(),
                // consultar trie.search() y mostrar las peliculas resultantes.
                (void)query;
                break;
            }
            case 3:
                std::cout << "Saliendo...\n";
                return 0;
            default:
                std::cout << "Opcion no valida.\n";
                break;
        }
    }

    return 0;
}
