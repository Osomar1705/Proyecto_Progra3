#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <string>
#include <vector>
#include "Movie.h"

class Processor {
public:
    Processor() = default;
    ~Processor() = default;

    std::vector<Movie> loadMovies(const std::string& filepath);
    std::vector<std::string> cleanString(const std::string& input);
};

#endif // PROCESSOR_H
