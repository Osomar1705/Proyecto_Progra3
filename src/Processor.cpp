#include "Processor.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

Processor::Processor() {
    loadStopWords();
}

void Processor::loadStopWords() {
    stop_words = {"the", "a", "an", "and", "or", "but", "in", "on", "at", "to", "for", "of", "with", "is", "are", "was", "were"};
}

// Separa las columnas del CSV respetando las comillas y los escapes ""
vector<string> Processor::parseCSVLine(const string& line) {
    vector<string> result;
    string current_cell;
    bool in_quotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            // Comilla escapada "" dentro de campo quoted -> caracter literal "
            if (in_quotes && i + 1 < line.size() && line[i + 1] == '"') {
                current_cell += '"';
                ++i;
            } else {
                in_quotes = !in_quotes;
            }
        } else if (c == ',' && !in_quotes) {
            result.push_back(current_cell);
            current_cell.clear();
        } else {
            current_cell += c;
        }
    }
    result.push_back(current_cell);
    return result;
}

vector<string> Processor::cleanAndSplitText(const string& text) {
    vector<string> words;
    string current_word = "";

    for (char c : text) {
        if (isalnum(static_cast<unsigned char>(c))) {
            current_word += static_cast<char>(tolower(static_cast<unsigned char>(c)));
        } else if (!current_word.empty()) {
            if (stop_words.find(current_word) == stop_words.end()) {
                words.push_back(current_word);
            }
            current_word = "";
        }
    }
    if (!current_word.empty() && stop_words.find(current_word) == stop_words.end()) {
        words.push_back(current_word);
    }

    return words;
}

// Cuenta el numero de comillas " en un buffer (necesario para detectar registros multilinea)
static size_t countQuotes(const string& s) {
    size_t n = 0;
    for (char c : s) if (c == '"') ++n;
    return n;
}

// Lee el archivo completo, soportando registros que abarcan varias lineas (plots con \n)
vector<Movie> Processor::loadMovies(const string& filename) {
    vector<Movie> movies;
    ifstream file(filename);
    string line;

    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << filename << endl;
        return movies;
    }

    getline(file, line); // descartar header

    int current_id = 1;
    string record;
    while (getline(file, line)) {
        if (record.empty()) {
            record = line;
        } else {
            record += '\n';
            record += line;
        }

        // Si el numero de comillas es impar, el registro continua en la siguiente linea
        if (countQuotes(record) % 2 != 0) {
            continue;
        }

        vector<string> columns = parseCSVLine(record);
        record.clear();

        if (columns.size() >= 8) {
            Movie m;
            m.id = current_id++;
            m.title = columns[1];
            m.director = columns[3];
            m.cast = columns[4];
            m.genre = columns[5];
            m.plot = columns[7];

            // Indexar palabras de title, director, genre y plot (lo que el proyecto pide)
            auto title_words    = cleanAndSplitText(m.title);
            auto director_words = cleanAndSplitText(m.director);
            auto genre_words    = cleanAndSplitText(m.genre);
            auto plot_words     = cleanAndSplitText(m.plot);

            m.clean_words.reserve(title_words.size() + director_words.size()
                                  + genre_words.size() + plot_words.size());
            m.clean_words.insert(m.clean_words.end(), title_words.begin(),    title_words.end());
            m.clean_words.insert(m.clean_words.end(), director_words.begin(), director_words.end());
            m.clean_words.insert(m.clean_words.end(), genre_words.begin(),    genre_words.end());
            m.clean_words.insert(m.clean_words.end(), plot_words.begin(),     plot_words.end());

            movies.push_back(std::move(m));
        }
    }

    file.close();
    return movies;
}
