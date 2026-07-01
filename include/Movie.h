#ifndef MOVIE_H
#define MOVIE_H

#include <string>
#include <vector>

struct Movie {
    int id = 0; // default 0: distingue registros inválidos/sin llenar (evita UB al filtrar)
    std::string title;
    std::string director;
    std::string cast;
    std::string genre;
    std::string plot;
    std::vector<std::string> clean_words;
    std::vector<std::string> clean_title;
    std::vector<std::string> clean_genre;
};

#endif // MOVIE_H
