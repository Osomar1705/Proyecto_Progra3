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
                std::cout << "Ruta del archivo CSV (ej. wiki_movie_plots_deduped.csv): ";
                std::string path;
                std::getline(std::cin, path);
                
                std::cout << "Cargando y procesando datos... esto puede tomar un momento.\n";
                movies = processor.loadMovies(path);
                
                if (movies.empty()) {
                    std::cout << "Error: No se pudieron cargar las peliculas. Verifica la ruta.\n";
                } else {
                    std::cout << "Exito! Se cargaron " << movies.size() << " peliculas.\n";
                    
                }
                break;
            }
            case 2: {
                if (movies.empty()) {
                    std::cout << "Por favor, carga los datos primero (Opcion 1).\n";
                    break;
                }
                
                std::cout << "Termino de busqueda: ";
                std::string query;
                std::getline(std::cin, query);
                
                std::vector<std::string> query_tokens = processor.cleanAndSplitText(query);
                
                std::cout << "Tokens procesados a buscar: ";
                for (const auto& token : query_tokens) {
                    std::cout << "[" << token << "] ";
                }
                std::cout << "\n";

                
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
