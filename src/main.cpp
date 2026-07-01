#include <iostream>
#include <fstream>
#include <functional>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <memory>

#include "Movie.h"
#include "DataProcessor.h"
#include "InvertedIndex.h"
#include "Trie.h"
#include "Observer.h"
#include "Strategy.h"
#include "Utils.h"

using namespace std;

// Insert every suffix of `word` into the suffix index, tagging each terminal with termId.
void insertTermSuffixes(Trie& suffixIndex, const string& word, int termId) {
    for (size_t i = 0; i < word.length(); ++i) {
        suffixIndex.insert(word.substr(i), termId);
    }
}

// Build the two indexes from the loaded movies:
//   1) inverted index  -> unique vocabulary + posting lists (termId -> movie ids)
//   2) suffix index    -> suffixes of each UNIQUE term, so substrings map to termIds
void buildIndexes(const vector<Movie>& movies, InvertedIndex& invIndex, Trie& suffixIndex) {
    for (const Movie& m : movies) {
        for (const string& word : m.clean_words) {
            invIndex.addOccurrence(word, m.id);
        }
    }
    for (int termId = 0; termId < invIndex.termCount(); ++termId) {
        insertTermSuffixes(suffixIndex, invIndex.term(termId), termId);
    }
}

// Resolve a query token to the set of movie ids that contain it as a substring.
unordered_set<int> matchToken(const string& token, const InvertedIndex& invIndex, const Trie& suffixIndex) {
    unordered_set<int> matched;
    for (int termId : suffixIndex.collect(token)) {
        const vector<int>& plist = invIndex.postings(termId);
        matched.insert(plist.begin(), plist.end());
    }
    return matched;
}

string toLowerCopy(const string& s) {
    string out = s;
    transform(out.begin(), out.end(), out.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return out;
}

// ---------------------------------------------------------
// PATRON DE DISENO: OBSERVER (observador concreto)
// ---------------------------------------------------------
// Observa el conjunto de likes: cada notificación recalcula la caché de
// recomendaciones aplicando la Strategy configurada.
class RecommendationEngine : public ILikeObserver {
public:
    RecommendationEngine(const set<int>& likedMovies,
                         const vector<Movie>& movies,
                         const unordered_map<int, int>& movieIndexMap)
        : likedMovies_(likedMovies), movies_(movies), movieIndexMap_(movieIndexMap),
          strategy_(make_unique<GenreKeywordRecommendation>()) {}

    void onLikedMoviesChanged() override {
        recommendations_ = strategy_->recommend(likedMovies_, movies_, movieIndexMap_);
    }

    const vector<int>& recommendations() const { return recommendations_; }

private:
    const set<int>& likedMovies_;
    const vector<Movie>& movies_;
    const unordered_map<int, int>& movieIndexMap_;
    unique_ptr<IRecommendationStrategy> strategy_;
    vector<int> recommendations_;
};

// ---------------------------------------------------------
// PATRON DE DISENO: FACADE (usa Singleton, Strategy y Observer)
// ---------------------------------------------------------
class StreamingPlatform {
private:
    InvertedIndex invIndex;
    Trie suffixIndex;
    vector<Movie> movies;
    unordered_map<int, int> movieIndexMap;
    set<int> watchLater;
    set<int> likedMovies;
    string csvPath;

    LikeSubject likeSubject;        // Subject: notifica cambios de likes
    RecommendationEngine recEngine; // Observer: recalcula recomendaciones

    static constexpr const char* STATE_FILE = "platform_state.txt";

public:
    StreamingPlatform() : recEngine(likedMovies, movies, movieIndexMap) {
        likeSubject.attach(&recEngine);
    }

    // ----- Persistencia de la sesion (likes + ver mas tarde) -----

    void saveState() const {
        ofstream out(STATE_FILE);
        if (!out.is_open()) return;
        out << "csv=" << csvPath << "\n";
        out << "liked=" << joinIds(likedMovies) << "\n";
        out << "watchlater=" << joinIds(watchLater) << "\n";
    }

    // Al iniciar: si hay una sesion guardada, recarga el CSV y restaura
    // likes / ver-mas-tarde (requisito: mostrar "Ver mas tarde" al iniciar).
    void restoreSession() {
        ifstream in(STATE_FILE);
        if (!in.is_open()) return;

        string savedCsv, line;
        vector<int> likedIds, laterIds;
        while (getline(in, line)) {
            if (line.rfind("csv=", 0) == 0) savedCsv = line.substr(4);
            else if (line.rfind("liked=", 0) == 0) likedIds = parseIdList(line.substr(6));
            else if (line.rfind("watchlater=", 0) == 0) laterIds = parseIdList(line.substr(11));
        }
        if (savedCsv.empty()) return;

        cout << "Restaurando sesion anterior (" << savedCsv << ")...\n";
        loadData(savedCsv);
        if (movies.empty()) return;

        for (int id : likedIds) {
            if (movieIndexMap.count(id)) likedMovies.insert(id);
        }
        for (int id : laterIds) {
            if (movieIndexMap.count(id)) watchLater.insert(id);
        }
        likeSubject.notifyLikedMoviesChanged();
    }

    void loadData(const string& path) {
        DataProcessor& processor = DataProcessor::getInstance(); // Singleton

        // Comparación de tiempos: secuencial vs paralelo (tabla de la rúbrica).
        cout << "Cargando datos secuencialmente para comparar tiempos...\n";
        long long ms_seq = 0;
        size_t seqCount = 0;
        {
            auto t0_seq = chrono::steady_clock::now();
            vector<Movie> seqMovies = processor.loadMoviesSequential(path);
            auto t1_seq = chrono::steady_clock::now();
            ms_seq = chrono::duration_cast<chrono::milliseconds>(t1_seq - t0_seq).count();
            seqCount = seqMovies.size();
        } // seqMovies se libera aqui: la carga paralela no duplica el pico de memoria

        cout << "Cargando y procesando datos en paralelo (Programacion Paralela)...\n";
        auto t0_par = chrono::steady_clock::now();
        vector<Movie> loadedMovies = processor.loadMovies(path);
        auto t1_par = chrono::steady_clock::now();
        auto ms_par = chrono::duration_cast<chrono::milliseconds>(t1_par - t0_par).count();

        if (loadedMovies.empty()) {
            cout << "Error: No se pudieron cargar las peliculas. Verifica la ruta.\n";
            return;
        }

        cout << "Tiempos de carga:\n";
        cout << "- Secuencial: " << ms_seq << " ms (" << seqCount << " peliculas)\n";
        cout << "- Paralelo:   " << ms_par << " ms (" << loadedMovies.size() << " peliculas)\n";

        movies = move(loadedMovies);
        csvPath = path;
        movieIndexMap.clear();
        invIndex.clear();
        suffixIndex.clear();
        watchLater.clear();
        likedMovies.clear();

        for (size_t i = 0; i < movies.size(); ++i) {
            movieIndexMap[movies[i].id] = static_cast<int>(i);
        }
        likeSubject.notifyLikedMoviesChanged(); // Observer: resetea recomendaciones

        cout << "Exito! Se cargaron " << movies.size() << " peliculas.\n";
        cout << "Indexando (indice invertido + indice de sufijos)... ";
        auto t0 = chrono::steady_clock::now();
        buildIndexes(movies, invIndex, suffixIndex);
        auto t1 = chrono::steady_clock::now();
        auto ms = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();
        cout << "Completado en " << ms << " ms (" << invIndex.termCount() << " terminos unicos).\n";
    }

    void searchMovies() {
        if (movies.empty()) {
            cout << "Por favor, carga los datos primero (Opcion 1).\n";
            return;
        }

        cout << "Termino de busqueda (palabra, frase o sub-palabra).\n"
             << "Tags: 'director:<nombre>', 'cast:<actor>', 'genre:<genero>'\n> ";
        string query;
        if (!getline(cin, query)) return;

        // Busqueda por Tag: filtra un campo especifico (director / cast / genero)
        size_t colon = query.find(':');
        if (colon != string::npos) {
            string tag = toLowerCopy(query.substr(0, colon));
            string rest = query.substr(colon + 1);
            if (tag == "director") {
                searchByTag([](const Movie& m) -> const string& { return m.director; }, rest);
                return;
            }
            if (tag == "cast" || tag == "casting") {
                searchByTag([](const Movie& m) -> const string& { return m.cast; }, rest);
                return;
            }
            if (tag == "genre" || tag == "genero") {
                searchByTag([](const Movie& m) -> const string& { return m.genre; }, rest);
                return;
            }
            cout << "Tag desconocido '" << tag << "'. Tags validos: director, cast, genre.\n";
            return;
        }

        vector<string> query_tokens = DataProcessor::getInstance().cleanAndSplitText(query);
        if (query_tokens.empty()) {
            // Query sin palabras utiles (solo stop-words o muy corto): buscarlo crudo
            // como sub-palabra, pero con un minimo de 3 caracteres para no degenerar
            // en un recorrido de casi todo el indice de sufijos.
            string raw = toLowerCopy(query);
            if (raw.size() >= 3) query_tokens.push_back(raw);
        }
        if (query_tokens.empty()) {
            cout << "Busqueda invalida: ingresa al menos una palabra util (3+ caracteres).\n";
            return;
        }

        // ALGORITMO DE IMPORTANCIA: por cada token del query, una pelicula suma
        //   +1 si el token aparece como sub-palabra en cualquier campo indexado
        //   +2 si ademas el token es una palabra exacta de la pelicula
        //   +3 si ademas el token aparece dentro de una palabra del titulo
        // El desempate es el orden original del dataset (id ascendente).
        unordered_map<int, int> scores;
        for (const auto& token : query_tokens) {
            unordered_set<int> matched = matchToken(token, invIndex, suffixIndex);

            const int exactTermId = invIndex.findTerm(token);
            const vector<int>* exactPostings =
                exactTermId >= 0 ? &invIndex.postings(exactTermId) : nullptr;

            for (int id : matched) {
                int s = 1;
                if (exactPostings &&
                    binary_search(exactPostings->begin(), exactPostings->end(), id)) {
                    s += 2;
                }
                const Movie& m = movies[movieIndexMap.at(id)];
                for (const string& w : m.clean_title) {
                    if (w.find(token) != string::npos) { s += 3; break; }
                }
                scores[id] += s;
            }
        }

        vector<pair<int, int>> rankedResults;
        rankedResults.reserve(scores.size());
        for (auto const& [id, score] : scores) {
            rankedResults.emplace_back(score, id);
        }
        sortRanked(rankedResults);

        if (rankedResults.empty()) {
            cout << "No se encontraron coincidencias.\n";
            return;
        }
        browseResults(rankedResults);
    }

    void displayUI() {
        while (true) {
            cout << "\n=== Sistema de Recomendacion de Peliculas ===\n";

            if (!movies.empty()) {
                if (!watchLater.empty()) {
                    cout << "\n--- Ver mas tarde ---\n";
                    int count = 0;
                    for (int id : watchLater) {
                        if (count++ < 5) cout << "  > " << movies[movieIndexMap.at(id)].title << "\n";
                    }
                    if (watchLater.size() > 5) cout << "  ... y " << watchLater.size() - 5 << " mas.\n";
                }

                const vector<int>& recommended = recEngine.recommendations();
                if (!recommended.empty()) {
                    cout << "\n--- Recomendadas para ti ---\n";
                    for (int id : recommended) {
                        const Movie& m = movies[movieIndexMap.at(id)];
                        cout << "  * " << m.title << " (" << m.genre << ")\n";
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
                if (cin.eof()) { // sin mas entrada: salir en vez de loopear infinito
                    cout << "\nFin de la entrada. Saliendo...\n";
                    saveState();
                    return;
                }
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
                    if (!getline(cin, path)) return;
                    loadData(path);
                    saveState();
                    break;
                }
                case 2:
                    searchMovies();
                    break;
                case 3:
                    cout << "\n--- Lista completa: Ver Mas Tarde ---\n";
                    if (watchLater.empty()) cout << "(Vacia)\n";
                    for (int id : watchLater) {
                        if (movieIndexMap.count(id))
                            cout << "- " << movies[movieIndexMap.at(id)].title << "\n";
                    }
                    break;
                case 4:
                    cout << "Saliendo...\n";
                    saveState();
                    return;
                default:
                    cout << "Opcion no valida.\n";
                    break;
            }
        }
    }

private:
    static string joinIds(const set<int>& ids) {
        string out;
        for (int id : ids) {
            if (!out.empty()) out += ',';
            out += to_string(id);
        }
        return out;
    }

    static vector<int> parseIdList(const string& s) {
        vector<int> ids;
        stringstream ss(s);
        string item;
        while (getline(ss, item, ',')) {
            try {
                if (!item.empty()) ids.push_back(stoi(item));
            } catch (...) {
                // id malformado: se ignora
            }
        }
        return ids;
    }

    // Orden: score descendente; empate -> orden original del dataset (id ascendente).
    static void sortRanked(vector<pair<int, int>>& ranked) {
        sort(ranked.begin(), ranked.end(),
             [](const pair<int, int>& a, const pair<int, int>& b) {
                 if (a.first != b.first) return a.first > b.first;
                 return a.second < b.second;
             });
    }

    // Busqueda por tag: puntua cada pelicula por cuantos tokens aparecen en el
    // campo indicado (comparacion case-insensitive por sub-string).
    void searchByTag(const function<const string&(const Movie&)>& field, const string& text) {
        vector<string> tokens = DataProcessor::getInstance().cleanAndSplitText(text);
        if (tokens.empty()) {
            string raw = toLowerCopy(text);
            raw.erase(remove_if(raw.begin(), raw.end(),
                                [](unsigned char c) { return isspace(c); }),
                      raw.end());
            if (raw.size() >= 2) tokens.push_back(raw);
        }
        if (tokens.empty()) {
            cout << "Busqueda de tag vacia (minimo 2 caracteres).\n";
            return;
        }

        vector<pair<int, int>> ranked; // (score, movieId)
        for (const Movie& m : movies) {
            const string low = toLowerCopy(field(m));
            int score = 0;
            for (const string& tok : tokens) {
                if (low.find(tok) != string::npos) ++score;
            }
            if (score > 0) ranked.emplace_back(score, m.id);
        }
        sortRanked(ranked);

        if (ranked.empty()) {
            cout << "No se encontraron coincidencias.\n";
            return;
        }
        browseResults(ranked);
    }

    // Paginacion de resultados (5 por pagina) + detalle con Like / Ver mas tarde.
    void browseResults(const vector<pair<int, int>>& rankedResults) {
        int current_start = 0;
        const int pageSize = 5;
        while (current_start < (int)rankedResults.size()) {
            int end = min(current_start + pageSize, (int)rankedResults.size());
            cout << "\n--- Resultados " << current_start + 1 << " - " << end
                 << " de " << rankedResults.size() << " ---\n";
            for (int i = current_start; i < end; ++i) {
                const Movie& m = movies[movieIndexMap.at(rankedResults[i].second)];
                cout << (i - current_start + 1) << ". " << m.title << " [" << m.genre << "]\n";
            }

            cout << "\nSeleccione un numero para ver detalles, 'n' para mas resultados, o 'q' para volver: ";
            string choice;
            if (!getline(cin, choice)) break;

            if (choice == "q") break;
            if (choice == "n") {
                if (current_start + pageSize >= (int)rankedResults.size()) {
                    cout << "No hay mas resultados.\n";
                    break;
                }
                current_start += pageSize;
                continue;
            }

            try {
                int localIndex = stoi(choice);
                if (localIndex >= 1 && localIndex <= end - current_start) {
                    int resultIndex = current_start + localIndex - 1;
                    int mid = rankedResults[resultIndex].second;
                    const Movie& m = movies[movieIndexMap.at(mid)];
                    cout << "\n///////////////////////////////////\n";
                    cout << "TITULO:   " << m.title << "\n";
                    cout << "DIRECTOR: " << m.director << "\n";
                    cout << "GENERO:   " << m.genre << "\n";
                    cout << "REPARTO:  " << m.cast << "\n";
                    cout << "SINOPSIS: " << m.plot.substr(0, 800) << (m.plot.length() > 800 ? "..." : "") << "\n";
                    cout << "\n///////////////////////////////////\n";

                    cout << "\n1. Like  2. Ver mas tarde  3. Volver\nOpcion: ";
                    string sub_choice;
                    if (!getline(cin, sub_choice)) break;
                    if (sub_choice == "1") {
                        likedMovies.insert(mid);
                        cout << "Agregado a tus Likes!\n";
                        likeSubject.notifyLikedMoviesChanged(); // Observer
                        saveState();
                    } else if (sub_choice == "2") {
                        watchLater.insert(mid);
                        cout << "Agregado a Ver mas tarde!\n";
                        saveState();
                    }
                } else {
                    cout << "Indice fuera de rango.\n";
                }
            } catch (...) {
                cout << "Opcion invalida.\n";
            }
        }
    }
};

int main() {
    StreamingPlatform app;
    app.restoreSession();
    app.displayUI();
    return 0;
}
