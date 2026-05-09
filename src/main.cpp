#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <algorithm>

#include "Movie.h"
#include "Processor.h"
#include "Trie.h"

using namespace std;

// Función para insertar todos los sufijos de una palabra en el Trie
void insertWordSuffixes(Trie& trie, const string& word, int movieId) {
    for (size_t i = 0; i < word.length(); ++i) {
        trie.insert(word.substr(i), movieId);
    }
}

void indexMovie(Trie& trie, Processor& processor, const Movie& m) {
    vector<string> fields = {m.title, m.director, m.cast, m.genre, m.plot};
    for (const string& field : fields) {
        vector<string> words = processor.cleanAndSplitText(field);
        for (const string& word : words) {
            insertWordSuffixes(trie, word, m.id);
        }
    }
}

vector<int> getSimilarMovies(const set<int>& likedMovies, const vector<Movie>& allMovies, unordered_map<int, int>& movieIndexMap, Processor& processor) {
    if (likedMovies.empty()) return {};

    unordered_map<string, int> genreCount;
    unordered_map<string, int> keywordCount;

    for (int mid : likedMovies) {
        const Movie& m = allMovies[movieIndexMap[mid]];
        vector<string> genres = processor.cleanAndSplitText(m.genre);
        for (const string& g : genres) genreCount[g]++;
        
        vector<string> titles = processor.cleanAndSplitText(m.title);
        for (const string& t : titles) keywordCount[t]++;
    }

    unordered_map<int, int> scores;
    for (const auto& m : allMovies) {
        if (likedMovies.find(m.id) != likedMovies.end()) continue;

        // Score based on genre match
        vector<string> genres = processor.cleanAndSplitText(m.genre);
        for (const string& g : genres) {
            if (genreCount.count(g)) scores[m.id] += genreCount[g] * 2;
        }

        // Score based on title keyword match
        vector<string> titles = processor.cleanAndSplitText(m.title);
        for (const string& t : titles) {
            if (keywordCount.count(t)) scores[m.id] += keywordCount[t];
        }
    }

    vector<pair<int, int>> ranked;
    for (auto const& [id, score] : scores) ranked.push_back({score, id});
    sort(ranked.rbegin(), ranked.rend());

    vector<int> result;
    for (int i = 0; i < min((int)ranked.size(), 5); ++i) result.push_back(ranked[i].second);
    return result;
}

int main() {
    Processor processor;
    Trie trie;
    vector<Movie> movies;
    unordered_map<int, int> movieIndexMap; // Maps movie ID to index in 'movies' vector
    set<int> watchLater;
    set<int> likedMovies;

    while (true) {
        cout << "\n========================================\n";
        cout << "   PLATAFORMA DE STREAMING PROGRA III   \n";
        cout << "========================================\n";

        if (!movies.empty()) {
            if (!watchLater.empty()) {
                cout << "\n--- Ver mas tarde ---\n";
                int count = 0;
                for (int id : watchLater) {
                    if (count++ < 5) cout << "  > " << movies[movieIndexMap[id]].title << "\n";
                }
                if (watchLater.size() > 5) cout << "  ... y " << watchLater.size() - 5 << " mas.\n";
            }

            vector<int> similar = getSimilarMovies(likedMovies, movies, movieIndexMap, processor);
            if (!similar.empty()) {
                cout << "\n--- Recomendadas para ti ---\n";
                for (int id : similar) {
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
                movies = processor.loadMovies(path);
                
                if (movies.empty()) {
                    cout << "Error: No se pudieron cargar las peliculas. Verifica la ruta.\n";
                } else {
                    cout << "Exito! Se cargaron " << movies.size() << " peliculas.\n";
                    cout << "Indexando peliculas en el Árbol (Trie de sufijos)... ";
                    for (size_t i = 0; i < movies.size(); ++i) {
                        movieIndexMap[movies[i].id] = i;
                        indexMovie(trie, processor, movies[i]);
                        if (i % 5000 == 0 && i > 0) cout << i << "... ";
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
                    rankedResults.push_back({score, id});
                }
                sort(rankedResults.rbegin(), rankedResults.rend());

                int current_start = 0;
                while (current_start < static_cast<int>(rankedResults.size())) {
                    cout << "\n--- Resultados " << current_start + 1 << " - " << min((int)rankedResults.size(), current_start + 5) << " de " << rankedResults.size() << " ---\n";
                    for (int i = current_start; i < min((int)rankedResults.size(), current_start + 5); ++i) {
                        int mid = rankedResults[i].second;
                        cout << i + 1 << ". " << movies[movieIndexMap[mid]].title << " [" << movies[movieIndexMap[mid]].genre << "]\n";
                    }

                    cout << "\nSeleccione un numero para ver detalles, 'n' para mas resultados, o 'q' para volver: ";
                    string choice;
                    getline(cin, choice);

                    if (choice == "q") break;
                    if (choice == "n") {
                        current_start += 5;
                        continue;
                    }

                    try {
                        int idx = stoi(choice) - 1;
                        if (idx >= 0 && idx < static_cast<int>(rankedResults.size())) {
                            int mid = rankedResults[idx].second;
                            Movie& m = movies[movieIndexMap[mid]];
                            cout << "\n----------------------------------------\n";
                            cout << "TITULO:   " << m.title << "\n";
                            cout << "DIRECTOR: " << m.director << "\n";
                            cout << "GENERO:   " << m.genre << "\n";
                            cout << "REPARTO:  " << m.cast << "\n";
                            cout << "SINOPSIS: " << m.plot.substr(0, 800) << (m.plot.length() > 800 ? "..." : "") << "\n";
                            cout << "----------------------------------------\n";
                            
                            cout << "\n1. Like  2. Ver mas tarde  3. Volver\nOpcion: ";
                            int sub_opt;
                            cin >> sub_opt;
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            if (sub_opt == 1) {
                                likedMovies.insert(mid);
                                cout << "Agregado a tus Likes!\n";
                            } else if (sub_opt == 2) {
                                watchLater.insert(mid);
                                cout << "Agregado a Ver mas tarde!\n";
                            }
                        }
                    } catch (...) {
                        cout << "Opcion invalida.\n";
                    }
                    break;
                }
                if (rankedResults.empty()) cout << "No se encontraron coincidencias.\n";
                break;
            }
            case 3:
                cout << "\n--- Lista completa: Ver Mas Tarde ---\n";
                if (watchLater.empty()) cout << "(Vacia)\n";
                for (int id : watchLater) {
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
