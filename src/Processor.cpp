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
    stop_words = {
        "the", "a", "an", "and", "or", "but", "in", "on", "at", "to", "for", "of", "with", 
        "is", "are", "was", "were", "it", "this", "that", "these", "those", "by", "from", 
        "as", "be", "been", "being", "have", "has", "had", "do", "does", "did", "which", 
        "who", "whom", "where", "when", "how", "why", "all", "any", "both", "each", "few", 
        "more", "most", "other", "some", "such", "no", "nor", "not", "only", "own", "same", 
        "so", "than", "too", "very", "s", "t", "can", "will", "just", "don", "should", "now"
    };
}

// Separa las columnas del CSV respetando las comillas
vector<string> Processor::parseCSVLine(const string& line) {
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

vector<string> Processor::cleanAndSplitText(const string& text) {
    vector<string> words;
    string current_word = "";
    
    for (char c : text) {
        if (isalnum(c)) {
            current_word += (char)tolower((unsigned char)c); 
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

// Lee el archivo completo
vector<Movie> Processor::loadMovies(const string& filename) {
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
        
        if (columns.size() >= 8) {
            Movie m;
            m.id = current_id++;
            m.title = columns[1];
            m.director = columns[3];
            m.cast = columns[4];
            m.genre = columns[5];
            m.plot = columns[7];
            
            movies.push_back(m);
        }
    }
    
    file.close();
    return movies;
}