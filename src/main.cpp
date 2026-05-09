#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <algorithm>

#include "Movie.h"
#include "DataProcessor.h"
#include "Trie.h"

using namespace std;

// Función para insertar todos los sufijos de una palabra en el Trie
void insertWordSuffixes(Trie& trie, const string& word, int movieId) {
    for (size_t i = 0; i < word.length(); ++i) {
        trie.insert(word.substr(i), movieId);
    }
}

void indexMovie(Trie& trie, const Movie& m) {
    // Usamos las palabras ya limpias que se procesaron al cargar el CSV
    for (const string& word : m.clean_words) {
        insertWordSuffixes(trie, word, m.id);
    }
}

vector<int> getSimilarMovies(const set<int>& likedMovies, const vector<Movie>& allMovies, unordered_map<int, int>& movieIndexMap) {
    if (likedMovies.empty()) return {};

    unordered_map<string, int> genreCount;
    unordered_map<string, int> keywordCount;

    for (int mid : likedMovies) {
        if (movieIndexMap.count(mid)) {
            const Movie& m = allMovies[movieIndexMap[mid]];
            for (const string& g : m.clean_genre) genreCount[g]++;
            for (const string& t : m.clean_title) keywordCount[t]++;
        }
    }

    unordered_map<int, int> scores;
    for (const auto& m : allMovies) {
        if (likedMovies.count(m.id)) continue;

        int score = 0;
        for (const string& g : m.clean_genre) {
            if (genreCount.count(g)) score += genreCount[g] * 2;
        }
        for (const string& t : m.clean_title) {
            if (keywordCount.count(t)) score += keywordCount[t];
        }
        if (score > 0) scores[m.id] = score;
    }

    vector<pair<int, int>> ranked;
    for (auto const& [id, score] : scores) ranked.push_back({score, id});
    sort(ranked.rbegin(), ranked.rend());

    vector<int> result;
    for (int i = 0; i < min((int)ranked.size(), 5); ++i) result.push_back(ranked[i].second);
    return result;
}

int main() {
    DataProcessor processor;
    Trie trie;
    vector<Movie> movies;
    unordered_map<int, int> movieIndexMap; 
    set<int> watchLater;
    set<int> likedMovies;
    vector<int> recommendedCache;
    size_t lastLikedSize = 0;

    while (true) {
        cout << "\n=== Sistema de Recomendacion de Peliculas ===\n";

        if (!movies.empty()) {
            if (!watchLater.empty()) {
                cout << "\n--- Ver mas tarde ---\n";
                int count = 0;
                for (int id : watchLater) {
                    if (count++ < 5) cout << "  > " << movies[movieIndexMap[id]].title << "\n";
                }
                if (watchLater.size() > 5) cout << "  ... y " << watchLater.size() - 5 << " mas.\n";
            }

            // Recalcular recomendaciones solo si han cambiado los likes
            if (likedMovies.size() != lastLikedSize) {
                recommendedCache = getSimilarMovies(likedMovies, movies, movieIndexMap);
                lastLikedSize = likedMovies.size();
            }

            if (!recommendedCache.empty()) {
                cout << "\n--- Recomendadas para ti ---\n";
                for (int id : recommendedCache) {
                    cout << "  * " << movies[movieIndexMap[id]].title << " (" << movies[movieIndexMap[id]].genre << ")\n";
                }
            }
        }

        cout << "\n1. Cargar datos (CSV)\n";
        cout << "2. Buscar peliculas\n";
        cout << "3. Ver lista completa 'Ver mas tarde'\n";
        cout << "4. Salir\n";
        cout << "Seleccione una opcion: ";

        int option = 0;
        if (!(cin >> option)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Entrada invalida.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (option) {
            case 1: {
                cout << "Ruta del archivo CSV (ej. wiki_movie_plots_deduped.csv): ";
                string path;
                getline(cin, path);
                
                cout << "Cargando y procesando datos... esto puede tomar un momento.\n";
                vector<Movie> loadedMovies = processor.loadMovies(path);
                
                if (loadedMovies.empty()) {
                    cout << "Error: No se pudieron cargar las peliculas. Verifica la ruta.\n";
                } else {
                    // Limpiar datos anteriores
                    movies = move(loadedMovies);
                    movieIndexMap.clear();
                    trie.clear();
                    watchLater.clear();
                    likedMovies.clear();
                    recommendedCache.clear();
                    lastLikedSize = 0;

                    cout << "Exito! Se cargaron " << movies.size() << " peliculas.\n";
                    cout << "Indexando peliculas en el Árbol (Trie de sufijos)... ";
                    for (size_t i = 0; i < movies.size(); ++i) {
                        movieIndexMap[movies[i].id] = i;
                        indexMovie(trie, movies[i]);
                        if (i > 0 && i % 5000 == 0) cout << i << "... ";
                    }
                    cout << "Completado!\n";
                }
                break;
            }
            case 2: {
                if (movies.empty()) {
                    cout << "Por favor, carga los datos primero (Opcion 1).\n";
                    break;
                }

                cout << "Termino de busqueda (palabra, frase o sub-palabra): ";
                string query;
                getline(cin, query);

                vector<string> query_tokens = processor.cleanAndSplitText(query);
                if (query_tokens.empty() && !query.empty()) {
                    string raw_query = query;
                    for (char &c : raw_query) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
                    query_tokens.push_back(raw_query);
                }

                if (query_tokens.empty()) {
                    cout << "Busqueda vacia.\n";
                    break;
                }

                unordered_map<int, int> scores;
                for (const auto& token : query_tokens) {
                    vector<int> results = trie.search(token);
                    for (int id : results) {
                        scores[id]++;
                    }
                }

                vector<pair<int, int>> rankedResults;
                for (auto const& [id, score] : scores) {
                    rankedResults.emplace_back(score, id);
                }
                sort(rankedResults.rbegin(), rankedResults.rend());

                if (rankedResults.empty()) {
                    cout << "No se encontraron coincidencias.\n";
                    break;
                }

                int current_start = 0;
                const int pageSize = 5;
                while (current_start < (int)rankedResults.size()) {
                    int end = min(current_start + pageSize, (int)rankedResults.size());
                    cout << "\n--- Resultados " << current_start + 1 << " - " << end << " de " << rankedResults.size() << " ---\n";
                    for (int i = current_start; i < end; ++i) {
                        int mid = rankedResults[i].second;
                        cout << (i - current_start + 1) << ". " << movies[movieIndexMap[mid]].title << " [" << movies[movieIndexMap[mid]].genre << "]\n";
                    }

                    cout << "\nSeleccione un numero para ver detalles, 'n' para mas resultados, o 'q' para volver: ";
                    string choice;
                    getline(cin, choice);

                    if (choice == "q") break;
                    if (choice == "n") {
                        current_start += pageSize;
                        continue;
                    }

                    try {
                        int localIndex = stoi(choice);
                        if (localIndex >= 1 && localIndex <= end - current_start) {
                            int resultIndex = current_start + localIndex - 1;
                            int mid = rankedResults[resultIndex].second;
                            Movie& m = movies[movieIndexMap[mid]];
                            cout << "\n///////////////////////////////////\n";
                            cout << "TITULO:   " << m.title << "\n";
                            cout << "DIRECTOR: " << m.director << "\n";
                            cout << "GENERO:   " << m.genre << "\n";
                            cout << "REPARTO:  " << m.cast << "\n";
                            cout << "SINOPSIS: " << m.plot.substr(0, 800) << (m.plot.length() > 800 ? "..." : "") << "\n";
                            cout << "\n///////////////////////////////////\n";

                            cout << "\n1. Like  2. Ver mas tarde  3. Volver\nOpcion: ";
                            string sub_choice;
                            getline(cin, sub_choice);
                            if (sub_choice == "1") {
                                likedMovies.insert(mid);
                                cout << "Agregado a tus Likes!\n";
                            } else if (sub_choice == "2") {
                                watchLater.insert(mid);
                                cout << "Agregado a Ver mas tarde!\n";
                            }
                        } else {
                            cout << "Indice fuera de rango.\n";
                        }
                    } catch (...) {
                        cout << "Opcion invalida.\n";
                    }
                }
                break;
            }
            case 3:
                cout << "\n--- Lista completa: Ver Mas Tarde ---\n";
                if (watchLater.empty()) cout << "(Vacia)\n";
                for (int id : watchLater) {
                    if (movieIndexMap.count(id))
                        cout << "- " << movies[movieIndexMap[id]].title << "\n";
                }
                break;
            case 4:
                cout << "Saliendo...\n";
                return 0;
            default:
                cout << "Opcion no valida.\n";
                break;
        }
    }

    return 0;
}
