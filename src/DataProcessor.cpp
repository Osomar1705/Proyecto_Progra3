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

#include "Utils.h"

vector<Movie> DataProcessor::loadMoviesSequential(const string& filename) {
    vector<Movie> movies;
    ifstream file(filename);
    string line;
    
    if (!file.is_open()) return movies;
    
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
            string text_to_clean = m.title + " " + m.director + " " + m.genre + " " + m.plot;
            m.clean_words = cleanAndSplitText(text_to_clean);
            m.clean_title = cleanAndSplitText(m.title);
            m.clean_genre = cleanAndSplitText(m.genre);
            movies.push_back(m);
        }
    }
    file.close();
    return movies;
}

vector<Movie> DataProcessor::loadMovies(const string& filename) {
    ifstream file(filename);
    string line;
    if (!file.is_open()) return {};
    getline(file, line); // Cabecera
    
    vector<string> lines;
    while(getline(file, line)) {
        if(!line.empty()) lines.push_back(line);
    }
    file.close();

    vector<Movie> movies;
    parallel_map(lines, movies, [this](const string& l, int index) -> Movie {
        vector<string> columns = parseCSVLine(l);
        Movie m;
        if (columns.size() >= 8) {
            m.id = index + 1;
            m.title = columns[1];
            m.director = columns[3];
            m.cast = columns[4];
            m.genre = columns[5];
            m.plot = columns[7];
            string text_to_clean = m.title + " " + m.director + " " + m.genre + " " + m.plot;
            m.clean_words = cleanAndSplitText(text_to_clean);
            m.clean_title = cleanAndSplitText(m.title);
            m.clean_genre = cleanAndSplitText(m.genre);
        }
        return m;
    });

    // Filtrar aquellas lineas invalidas (si las hay)
    vector<Movie> valid_movies;
    for(auto& m : movies) {
        if(m.id != 0) valid_movies.push_back(std::move(m));
    }
    return valid_movies;
}
