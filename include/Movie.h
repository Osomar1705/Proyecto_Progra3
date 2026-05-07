#ifndef MOVIE_H
#define MOVIE_H

#include <string>
#include <vector>

struct Movie {
    int id;
    std::string title;
    std::string synopsis;
    std::vector<std::string> tags;
    float ranking;
};

#endif // MOVIE_H
