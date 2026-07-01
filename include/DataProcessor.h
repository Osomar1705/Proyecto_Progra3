#pragma once
#include "Movie.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

using namespace std;

// ---------------------------------------------------------
// PATRON DE DISENO: SINGLETON
// ---------------------------------------------------------
class DataProcessor {
private:
    unordered_set<string> stop_words;
    
    DataProcessor(); // Private constructor
    void loadStopWords();
    vector<string> parseCSVLine(const string& line);

public:
    static DataProcessor& getInstance() {
        static DataProcessor instance;
        return instance;
    }
    
    DataProcessor(DataProcessor const&) = delete;
    void operator=(DataProcessor const&) = delete;

    vector<Movie> loadMovies(const string& filename); // Parallel
    vector<Movie> loadMoviesSequential(const string& filename); // Sequential for comparison
    
    vector<string> cleanAndSplitText(const string& text); 
};
