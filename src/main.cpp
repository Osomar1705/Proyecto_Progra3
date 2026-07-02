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
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#endif

#include "Movie.h"
#include "DataProcessor.h"
#include "InvertedIndex.h"
#include "Trie.h"
#include "Observer.h"
#include "Strategy.h"
#include "Utils.h"

using namespace std;

// Utilidades de consola: color, cajas y formato de la interfaz de texto.
namespace ui {
    const int WIDTH=64;

#ifdef _WIN32
    HANDLE hConsole=GetStdHandle(STD_OUTPUT_HANDLE);
    WORD defaultColor=7;

    const WORD COLOR_TITLE=FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; //los titulos de color cian
    const WORD COLOR_ACCENT =FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;//para resaltar de color amarillo
    const WORD COLOR_TAG=FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;//etiquetas de color magenta
    const WORD COLOR_OK= FOREGROUND_GREEN | FOREGROUND_INTENSITY;  // las confirmaciones son de verde
    const WORD COLOR_DIM= FOREGROUND_INTENSITY; //textos secundarios de color gris

    void initConsole() {
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo(hConsole, &info)) defaultColor = info.wAttributes;
    }
    void setColor(WORD c) { SetConsoleTextAttribute(hConsole, c); }
    void resetColor(){ SetConsoleTextAttribute(hConsole, defaultColor); }
#else
    using ColorT=int;
    const ColorT COLOR_TITLE=0,COLOR_ACCENT=0, COLOR_TAG=0,COLOR_OK=0,COLOR_DIM=0;
    void initConsole(){}
    void setColor(ColorT){}
    void resetColor(){}
#endif

    void printColor(const string& text,decltype(COLOR_TITLE) c) {
        setColor(c);
        cout<<text;
        resetColor();}

    string truncate(const string& text,size_t maxW) {
        if (text.size() <= maxW) return text;
        if (maxW <= 3) return text.substr(0, maxW);
        return text.substr(0, maxW-3)+"...";}

    string pad(const string& text, size_t targetW) {
        if (text.size()>=targetW) return text;
        return text+string(targetW-text.size(),' ');}

    void rule(char corner = '+') {
        cout<<corner<<string(WIDTH, '-')<< corner<< "\n";}

    void boxLine(const string& text) {
        string shown = truncate(text, WIDTH - 2);
        cout << "|" << pad(" " + shown, WIDTH) << "|\n";}

    void boxLineColor(const string& text, decltype(COLOR_TITLE) c) {
        string shown = truncate(text, WIDTH - 2);
        string padded = pad(" " + shown, WIDTH);
        cout << "|";
        printColor(padded, c);
        cout << "|\n";}

    void banner() {
        rule('*');
        boxLine("");
        boxLineColor("STREAMING PLATFORM", COLOR_TITLE);
        boxLine("Plataforma de busqueda y administracion de peliculas");
        boxLine("");
        rule('*');}

    void section(const string& heading) {
        cout << "\n";
        rule();
        boxLineColor(heading, COLOR_TITLE);
        rule();}

    void movieRow(int number, const string& title, const string& genre) {
        string num = pad(to_string(number) + ".", 4);
        string t = pad(truncate(title, 38), 38);
        cout << "  ";
        printColor(num, COLOR_ACCENT);
        cout << " " << t << " ";
        printColor("[" + truncate(genre, 14) + "]", COLOR_TAG);
        cout << "\n";}

    void menuItem(const string& key, const string& label) {
        cout << "   ";
        printColor("[" + key + "]", COLOR_ACCENT);
        cout << " " << label << "\n";}
}

void insertTermSuffixes(Trie& suffixIndex, const string& word, int termId) {
    for (size_t i = 0; i < word.length(); ++i) {
        suffixIndex.insert(word.substr(i), termId);}
}


void buildIndexes(const vector<Movie>& movies, InvertedIndex& invIndex, Trie& suffixIndex) {
    for (const Movie& m : movies) {
        for (const string& word : m.clean_words) {
            invIndex.addOccurrence(word, m.id);}
    }
    for (int termId = 0; termId < invIndex.termCount(); ++termId) {
        insertTermSuffixes(suffixIndex, invIndex.term(termId), termId);}
}

unordered_set<int> matchToken(const string& token, const InvertedIndex& invIndex, const Trie& suffixIndex) {
    unordered_set<int> matched;
    for (int termId : suffixIndex.collect(token)) {
        const vector<int>& plist = invIndex.postings(termId);
        matched.insert(plist.begin(), plist.end());}
    return matched;
}

string toLowerCopy(const string& s) {
    string out = s;
    transform(out.begin(), out.end(), out.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return out;
}

// PATRON OBSERVER: observador concreto. Recalcula las recomendaciones (aplicando
// la Strategy) cada vez que el LikeSubject notifica un cambio en los likes.
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


// PATRON FACADE: fachada unica que orquesta carga, indices, busqueda,
// recomendaciones y persistencia. Usa Singleton (DataProcessor), Strategy y
// Observer (LikeSubject -> RecommendationEngine).
class StreamingPlatform {
private:
    InvertedIndex invIndex;
    Trie suffixIndex;
    vector<Movie> movies;
    unordered_map<int, int> movieIndexMap;
    set<int> watchLater;
    set<int> likedMovies;
    string csvPath;

    LikeSubject likeSubject;
    RecommendationEngine recEngine;

    static constexpr const char* STATE_FILE = "platform_state.txt";

public:
    StreamingPlatform() : recEngine(likedMovies, movies, movieIndexMap) {
        likeSubject.attach(&recEngine);
    }


    // Persistencia de la sesion (likes + ver mas tarde) entre ejecuciones.
    void saveState() const {
        ofstream out(STATE_FILE);
        if (!out.is_open()) return;
        out << "csv=" << csvPath << "\n";
        out << "liked=" << joinIds(likedMovies) << "\n";
        out << "watchlater=" << joinIds(watchLater) << "\n";
    }

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

        cout<<"Cargando y procesando datos en paralelo (Programacion Paralela)...\n";
        auto t0_par = chrono::steady_clock::now();
        vector<Movie> loadedMovies = processor.loadMovies(path);
        auto t1_par = chrono::steady_clock::now();
        auto ms_par = chrono::duration_cast<chrono::milliseconds>(t1_par - t0_par).count();

        if (loadedMovies.empty()) {
            cout << "Error: No se pudieron cargar las peliculas. Verifica la ruta.\n";
            return;}

        cout<<"Tiempos de carga:\n";
        cout<<"- Secuencial: "<< ms_seq<< " ms ("<<seqCount<<" peliculas)\n";
        cout<<"- Paralelo:   "<< ms_par<< " ms ("<< loadedMovies.size() << " peliculas)\n";

        movies=move(loadedMovies);
        csvPath=path;
        movieIndexMap.clear();
        invIndex.clear();
        suffixIndex.clear();
        watchLater.clear();
        likedMovies.clear();

        for (size_t i = 0; i < movies.size(); ++i) {
            movieIndexMap[movies[i].id] = static_cast<int>(i);}
        likeSubject.notifyLikedMoviesChanged();

        cout << "Exito! Se cargaron " << movies.size() << " peliculas.\n";
        cout << "Indexando (indice invertido + indice de sufijos)... ";
        auto t0 = chrono::steady_clock::now();
        buildIndexes(movies, invIndex, suffixIndex);
        auto t1 = chrono::steady_clock::now();
        auto ms = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();
        cout << "Completado en " << ms << " ms (" << invIndex.termCount() << " terminos unicos).\n";}

    void searchMovies() {
        if (movies.empty()) {
            cout << "Por favor, carga los datos primero (Opcion 1).\n";
            return;}

        ui::section("BUSCAR PELICULAS");
        cout <<"   Palabra, frase o sub-palabra.\n";
        cout <<"   Tags: director:<nombre>  cast:<actor>  genre:<genero>\n";
        cout <<"   > ";
        string query;
        if (!getline(cin, query)) return;

        size_t colon = query.find(':');
        if (colon != string::npos) {
            string tag = toLowerCopy(query.substr(0, colon));
            string rest = query.substr(colon + 1);
            if (tag == "director") {
                searchByTag([](const Movie& m) -> const string& { return m.director; }, rest);
                return;}
            if (tag == "cast" || tag == "casting") {
                searchByTag([](const Movie& m) -> const string& { return m.cast; }, rest);
                return;}
            if (tag == "genre" || tag == "genero") {
                searchByTag([](const Movie& m) -> const string& { return m.genre; }, rest);
                return;}
            cout << "Tag desconocido '" << tag << "'. Tags validos: director, cast, genre.\n";
            return;
        }

        vector<string> query_tokens = DataProcessor::getInstance().cleanAndSplitText(query);
        if (query_tokens.empty()) {
            string raw = toLowerCopy(query);
            if (raw.size() >= 3) query_tokens.push_back(raw);}
        if (query_tokens.empty()) {
            cout << "Busqueda invalida: ingresa al menos una palabra util (3+ caracteres).\n";
            return;}

        // ALGORITMO DE IMPORTANCIA: por cada token, una pelicula suma +1 si aparece
        // como sub-palabra, +2 si ademas es palabra exacta (binary_search sobre la
        // posting list) y +3 si ademas esta dentro de una palabra del titulo.
        // Desempate: orden original del dataset (id ascendente).
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
                    s += 2;}
                const Movie& m = movies[movieIndexMap.at(id)];
                for (const string& w : m.clean_title) {
                    if (w.find(token) != string::npos) { s += 3; break; }}
                scores[id] += s;
            }
        }

        vector<pair<int, int>> rankedResults;
        rankedResults.reserve(scores.size());
        for (auto const& [id, score] : scores) {
            rankedResults.emplace_back(score, id);}
        sortRanked(rankedResults);

        if (rankedResults.empty()) {
            cout<<"No se encontraron coincidencias.\n";
            return;}
        browseResults(rankedResults);
    }

    void displayUI() {
        bool firstRun = true;
        while (true) {
            if (firstRun) {
                ui::banner();
                firstRun = false;}

            if (!movies.empty()) {
                if (!watchLater.empty()) {
                    ui::section("VER MAS TARDE");
                    int count = 0, idx = 1;
                    for (int id : watchLater) {
                        if (count++ < 5) {
                            const Movie& m = movies[movieIndexMap.at(id)];
                            ui::movieRow(idx++, m.title, m.genre);}
                    }
                    if (watchLater.size() > 5)
                        cout << "   ... y " << (watchLater.size() - 5) << " mas.\n";}

                const vector<int>& recommended = recEngine.recommendations();
                if (!recommended.empty()) {
                    ui::section("RECOMENDADAS PARA TI");
                    int idx = 1;
                    for (int id : recommended) {
                        const Movie& m = movies[movieIndexMap.at(id)];
                        ui::movieRow(idx++, m.title, m.genre);}
                }
            }

            ui::section("MENU PRINCIPAL");
            ui::menuItem("1", "Cargar datos (CSV)");
            ui::menuItem("2", "Buscar peliculas");
            ui::menuItem("3", "Ver lista completa 'Ver mas tarde'");
            ui::menuItem("4", "Salir");
            cout << "\n   Opcion: ";

            int option = 0;
            if (!(cin >> option)) {
                if (cin.eof()) {
                    cout << "\n"; ui::printColor("Fin de la entrada. Saliendo...", ui::COLOR_DIM); cout << "\n";
                    saveState();
                    return;}
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                ui::printColor("Entrada invalida.", ui::COLOR_ACCENT); cout << "\n";
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            switch (option) {
                case 1: {
                    cout<< "   Ruta del archivo CSV (ej. wiki_movie_plots_deduped.csv): ";
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
                    ui::section("LISTA COMPLETA: VER MAS TARDE");
                    if (watchLater.empty()) {
                        cout << "   "; ui::printColor("(Vacia)", ui::COLOR_DIM); cout << "\n";
                    } else {
                        int idx = 1;
                        for (int id : watchLater) {
                            if (movieIndexMap.count(id)) {
                                const Movie& m = movies[movieIndexMap.at(id)];
                                ui::movieRow(idx++, m.title, m.genre);
                            }
                        }
                    }
                    break;
                case 4:
                    ui::printColor("Saliendo...", ui::COLOR_OK); cout << "\n";
                    saveState();
                    return;
                default:
                    ui::printColor("Opcion no valida.", ui::COLOR_ACCENT); cout << "\n";
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
            }
        }
        return ids;
    }

    static void sortRanked(vector<pair<int, int>>& ranked) {
        sort(ranked.begin(), ranked.end(),
             [](const pair<int, int>& a, const pair<int, int>& b) {
                 if (a.first != b.first) return a.first > b.first;
                 return a.second < b.second;
             });
    }

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
            cout<<"Busqueda de tag vacia (minimo 2 caracteres).\n";
            return;
        }

        vector<pair<int, int>> ranked;
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
            return;}
        browseResults(ranked);
    }

    void browseResults(const vector<pair<int, int>>& rankedResults) {
        int current_start = 0;
        const int pageSize = 5;
        while (current_start < (int)rankedResults.size()) {
            int end = min(current_start + pageSize, (int)rankedResults.size());
            ui::section("RESULTADOS " + to_string(current_start + 1) + "-" + to_string(end) +
                        " de " + to_string(rankedResults.size()));
            for (int i = current_start; i < end; ++i) {
                const Movie& m = movies[movieIndexMap.at(rankedResults[i].second)];
                ui::movieRow(i - current_start + 1, m.title, m.genre);
            }

            cout << "\n   [#] Ver detalle    [n] Mas resultados    [q] Volver\n   Opcion: ";
            string choice;
            if (!getline(cin, choice)) break;

            if (choice=="q") break;
            if (choice=="n") {
                if (current_start + pageSize >= (int)rankedResults.size()) {
                    cout << "   "; ui::printColor("No hay mas resultados.", ui::COLOR_DIM); cout << "\n";
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
                    showMovieDetail(mid);
                } else {
                    cout << "   "; ui::printColor("Indice fuera de rango.", ui::COLOR_ACCENT); cout << "\n";
                }
            } catch (...) {
                cout << "   "; ui::printColor("Opcion invalida.", ui::COLOR_ACCENT); cout << "\n";
            }
        }
    }

    void showMovieDetail(int mid) {
        const Movie& m = movies[movieIndexMap.at(mid)];
        cout << "\n";
        ui::rule();
        ui::boxLineColor(m.title, ui::COLOR_TITLE);
        ui::rule();
        ui::boxLine("Director: "+ m.director);
        ui::boxLine("Genero:   "+ m.genre);
        ui::boxLine("Reparto:  "+ m.cast);
        ui::rule();
        string plot = m.plot.substr(0, 800) + (m.plot.length() > 800 ? "..." : "");
        printWrapped(plot);
        ui::rule();

        cout << "   ";
        ui::printColor("[1]",ui::COLOR_ACCENT);
        cout << " Like    ";
        ui::printColor("[2]",ui::COLOR_ACCENT);
        cout << " Ver mas tarde    ";
        ui::printColor("[3]",ui::COLOR_ACCENT);
        cout << " Volver\n   Opcion: ";
        string sub_choice;
        if (!getline(cin, sub_choice)) return;
        if (sub_choice == "1") {
            likedMovies.insert(mid);
            cout << "   ";
            ui::printColor("Agregado a tus Likes!", ui::COLOR_OK);
            cout << "\n";
            likeSubject.notifyLikedMoviesChanged();
            saveState();
        } else if (sub_choice == "2") {
            watchLater.insert(mid);
            cout << "   ";
            ui::printColor("Agregado a Ver mas tarde!", ui::COLOR_OK);
            cout << "\n";
            saveState();}
    }

    void printWrapped(const string& text) {
        const size_t w = ui::WIDTH - 2;
        stringstream ss(text);
        string word, line;
        auto flush = [&]() { if (!line.empty()) { ui::boxLine(line); line.clear(); } };
        while (ss >> word) {
            if (line.empty()) line = word;
            else if (line.size() + 1 + word.size() <= w) line += " " + word;
            else { flush(); line = word; }}
        flush();
    }
};

int main() {
    ui::initConsole();
    StreamingPlatform app;
    app.restoreSession();
    app.displayUI();
    return 0;}
