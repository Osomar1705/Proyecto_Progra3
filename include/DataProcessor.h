#pragma once
#include "Movie.h"
#include <string>
#include <vector>
#include <unordered_set>

class DataProcessor {
private:
    std::unordered_set<std::string> stop_words;

    std::vector<std::string> parseCSVLine(const std::string& line);
    void loadStopWords();

public:
    DataProcessor();
    std::vector<Movie> loadMovies(const std::string& filename);

    std::vector<std::string> cleanAndSplitText(const std::string& text);
};
