#include "DataProcessor.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iterator>

using namespace std;

DataProcessor::DataProcessor() {
    loadStopWords();
}

void DataProcessor::loadStopWords() {
    stop_words = {
        "the", "a", "an", "and", "or", "but", "in", "on", "at", "to", "for", "of", "with",
        "is", "are", "was", "were", "it", "this", "that", "these", "those", "by", "from",
        "as", "be", "been", "being", "have", "has", "had", "do", "does", "did", "which",
        "who", "whom", "where", "when", "how", "why", "all", "any", "both", "each", "few",
        "more", "most", "other", "some", "such", "no", "nor", "not", "only", "own", "same",
        "so", "than", "too", "very", "s", "t", "can", "will", "just", "don", "should", "now"
    };

    // Stop-words en español (requerido por el enunciado)
    vector<string> spanish_stops = {
        "el", "la", "los", "las", "un", "una", "unos", "unas", "y", "e", "ni", "que",
        "en", "a", "de", "con", "por", "para", "como", "si", "o", "u", "su", "sus",
        "este", "esta", "esto", "estos", "estas", "ese", "esa", "eso", "esos", "esas",
        "mi", "tu", "me", "te", "se", "lo", "le", "les", "ha", "han", "hay"
    };

    for (const string& word : spanish_stops) {
        stop_words.insert(word);
    }
}

vector<string> DataProcessor::parseCSVLine(const string& line) {
    vector<string> result;
    string current_cell;
    bool in_quotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        if (c == '"') {
            if (in_quotes && i + 1 < line.length() && line[i + 1] == '"') {
                current_cell += '"'; // comilla escapada ("")
                i++;
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

vector<string> DataProcessor::cleanAndSplitText(const string& text) {
    vector<string> words;
    string current_word = "";

    for (char c : text) {
        if (isalnum(static_cast<unsigned char>(c))) {
            current_word += static_cast<char>(tolower(static_cast<unsigned char>(c)));
        } else {
            if (!current_word.empty()) {
                if (stop_words.find(current_word) == stop_words.end() && current_word.length() > 1) {
                    words.push_back(current_word);
                }
                current_word = "";
            }
        }
    }
    if (!current_word.empty() && stop_words.find(current_word) == stop_words.end() && current_word.length() > 1) {
        words.push_back(current_word);
    }

    return words;
}

// Un registro CSV puede abarcar varias líneas físicas cuando un campo entrecomillado
// (la sinopsis) contiene saltos de línea. Acumulamos líneas hasta que las comillas
// queden balanceadas: recién ahí el registro está completo.
vector<string> DataProcessor::readRecords(const string& filename) {
    vector<string> records;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << filename << endl;
        return records;
    }

    string line;
    getline(file, line); // Cabecera

    string record;
    bool inQuotedField = false;
    while (getline(file, line)) {
        // CSV con finales CRLF leído en modo binario/Linux: descartar el '\r' colgante
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (record.empty()) {
            record = line;
        } else {
            record += '\n'; // el salto de línea es parte del contenido del campo
            record += line;
        }

        for (char c : line) {
            if (c == '"') inQuotedField = !inQuotedField;
        }
        if (inQuotedField) continue; // comilla sin cerrar -> el registro sigue

        if (!record.empty()) records.push_back(record);
        record.clear();
    }
    if (!record.empty()) records.push_back(record); // registro final (dato malformado)

    file.close();
    return records;
}

// El CSV tiene 8 columnas (Release Year, Title, Origin/Ethnicity, Director, Cast, Genre, Wiki Page, Plot)
Movie DataProcessor::parseRecord(const string& record, int id) {
    Movie m; // id = 0 por defecto
    vector<string> columns = parseCSVLine(record);
    if (columns.size() < 8) return m; // inválido -> queda id = 0

    m.id = id;
    m.title = columns[1];
    m.director = columns[3];
    m.cast = columns[4];
    m.genre = columns[5];
    m.plot = columns[7];

    m.clean_title = cleanAndSplitText(m.title);
    m.clean_genre = cleanAndSplitText(m.genre);

    // El índice de búsqueda cubre título, director, reparto, género y sinopsis.
    // Los tokens de título y género se reutilizan para no tokenizar dos veces.
    m.clean_words = m.clean_title;
    vector<string> rest = cleanAndSplitText(m.director + " " + m.cast + " " + m.plot);
    m.clean_words.insert(m.clean_words.end(),
                         make_move_iterator(rest.begin()), make_move_iterator(rest.end()));
    m.clean_words.insert(m.clean_words.end(), m.clean_genre.begin(), m.clean_genre.end());
    return m;
}

// Versión SECUENCIAL: parsea record por record en un solo hilo.
vector<Movie> DataProcessor::loadMoviesSequential(const string& filename) {
    vector<string> records = readRecords(filename);
    vector<Movie> movies;
    movies.reserve(records.size());

    int id = 1;
    for (const string& rec : records) {
        Movie m = parseRecord(rec, id);
        if (m.id != 0) {
            movies.push_back(std::move(m));
            ++id;
        }
    }
    return movies;
}

// Versión PARALELA: la lectura del archivo es secuencial (I/O), pero la tokenización
// pesada (parseRecord -> cleanAndSplitText) se reparte entre hilos sobre records
// independientes. Luego se filtran inválidos y se renumeran los ids de forma contigua.
vector<Movie> DataProcessor::loadMovies(const string& filename) {
    vector<string> records = readRecords(filename);

    vector<Movie> mapped;
    parallel_map(records, mapped, [this](const string& rec, int index) -> Movie {
        return parseRecord(rec, index + 1); // id no-nulo para records válidos
    });

    vector<Movie> movies;
    movies.reserve(mapped.size());
    int id = 1;
    for (auto& m : mapped) {
        if (m.id != 0) {
            m.id = id++; // ids contiguos, iguales al loader secuencial
            movies.push_back(std::move(m));
        }
    }
    return movies;
}
