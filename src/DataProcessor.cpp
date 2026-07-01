#include "DataProcessor.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

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
    
    // Spanish Stop Words (Required by task)
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
            if (in_quotes && i + 1 < line.length() && line[i+1] == '"') {
                current_cell += '"'; // Escaped quote
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

vector<Movie> DataProcessor::loadMovies(const string& filename) {
    vector<Movie> movies;
    ifstream file(filename);
    string line;
    
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << filename << endl;
        return movies;
    }
    
    getline(file, line); // Cabecera
    
    int current_id = 1;
    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> columns = parseCSVLine(line);
        
        // El CSV tiene 8 columnas (Release Year, Title, Origin/Ethnicity, Director, Cast, Genre, Wiki Page, Plot)
        if (columns.size() >= 8) {
            Movie m;
            m.id = current_id++;
            m.title = columns[1];
            m.director = columns[3];
            m.cast = columns[4];
            m.genre = columns[5];
            m.plot = columns[7];
            
            // "Preparar las palabras para que estén listas para ser ingresadas a la estructura"
            // Combinamos los campos relevantes para el procesamiento
            string text_to_clean = m.title + " " + m.director + " " + m.genre + " " + m.plot;
            m.clean_words = cleanAndSplitText(text_to_clean);

            // Listas por campo usadas por el motor de recomendaciones (similitud por
            // género y título). Antes quedaban vacías y las recomendaciones no puntuaban.
            m.clean_title = cleanAndSplitText(m.title);
            m.clean_genre = cleanAndSplitText(m.genre);
            
            movies.push_back(m);
        }
    }
    
    file.close();
    return movies;
}
