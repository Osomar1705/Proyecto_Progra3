#include <iostream>
#include <limits>
#include <string>
#include <unordered_set>
#include <vector>

#include "Movie.h"
#include "Processor.h"
#include "Trie.h"

namespace {

void indexMovies(Trie& trie, const std::vector<Movie>& movies) {
    for (const auto& movie : movies) {
        for (const auto& word : movie.clean_words) {
            trie.insert(word, movie.id);
        }
    }
}

// Devuelve la interseccion de los IDs encontrados para cada token de la query.
// Una pelicula coincide solo si contiene TODOS los tokens (sirve para frases).
std::vector<int> searchAll(const Trie& trie, const std::vector<std::string>& tokens) {
    std::vector<int> result;
    if (tokens.empty()) return result;

    std::unordered_set<int> acc;
    bool first = true;
    for (const auto& token : tokens) {
        std::vector<int> ids = trie.search(token);
        if (first) {
            acc.insert(ids.begin(), ids.end());
            first = false;
        } else {
            std::unordered_set<int> next;
            for (int id : ids) {
                if (acc.count(id)) next.insert(id);
            }
            acc = std::move(next);
        }
        if (acc.empty()) break;
    }

    result.assign(acc.begin(), acc.end());
    return result;
}

void printResults(const std::vector<int>& ids, const std::vector<Movie>& movies, size_t max_show = 20) {
    std::cout << "Se encontraron " << ids.size() << " peliculas.\n";
    size_t shown = 0;
    for (int id : ids) {
        if (shown >= max_show) {
            std::cout << "  ... (" << (ids.size() - shown) << " mas)\n";
            break;
        }
        if (id >= 1 && static_cast<size_t>(id) <= movies.size()) {
            const Movie& m = movies[id - 1];
            std::cout << "  - [" << id << "] " << m.title;
            if (!m.genre.empty()) std::cout << " (" << m.genre << ")";
            std::cout << "\n";
        }
        ++shown;
    }
}

} // namespace

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
                    std::cout << "Indexando palabras en el Trie de Sufijos...\n";
                    indexMovies(trie, movies);
                    std::cout << "Indexacion completa.\n";
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
                if (query_tokens.empty()) {
                    std::cout << "Consulta vacia tras la limpieza.\n";
                    break;
                }

                std::cout << "Tokens a buscar:";
                for (const auto& token : query_tokens) std::cout << " [" << token << "]";
                std::cout << "\n";

                std::vector<int> ids = searchAll(trie, query_tokens);
                printResults(ids, movies);
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
