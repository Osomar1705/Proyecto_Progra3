#pragma once
#include "Movie.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

using namespace std;

class Processor {
private:
    unordered_set<string> stop_words;
    
    vector<string> parseCSVLine(const string& line);
    void loadStopWords();

public:
    Processor();
    vector<Movie> loadMovies(const string& filename);
    
    vector<string> cleanAndSplitText(const string& text); 
};