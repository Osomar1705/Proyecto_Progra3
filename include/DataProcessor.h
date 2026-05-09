#pragma once
#include "Movie.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

using namespace std;

class DataProcessor {
private:
    unordered_set<string> stop_words;
    
    vector<string> parseCSVLine(const string& line);
    void loadStopWords();

public:
    DataProcessor();
    vector<Movie> loadMovies(const string& filename);
    
    vector<string> cleanAndSplitText(const string& text); 
};
