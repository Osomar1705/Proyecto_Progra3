#ifndef MOVIE_H
#define MOVIE_H
#pragma once
#include <string>
using namespace std;
#include <vector>

struct Movie {
    int id;
    string title;
    string director;
    string cast;
    string genre;
    string plot;
    vector<string> clean_words;
};

#endif  
