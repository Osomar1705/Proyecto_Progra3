#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include "Movie.h"
#include <string>
#include <vector>
#include <unordered_set>

// ---------------------------------------------------------
// PATRON DE DISENO: SINGLETON
// ---------------------------------------------------------
// Única instancia del procesador; comparte la tabla de stop-words.
class DataProcessor {
private:
    std::unordered_set<std::string> stop_words;

    DataProcessor(); // constructor privado
    void loadStopWords();
    std::vector<std::string> parseCSVLine(const std::string& line);

    // Lee el CSV respetando campos entrecomillados que abarcan varias líneas.
    // Devuelve un registro (record) completo por película.
    std::vector<std::string> readRecords(const std::string& filename);

    // Convierte un record en Movie. Si el record no tiene 8+ columnas, deja id = 0.
    Movie parseRecord(const std::string& record, int id);

public:
    static DataProcessor& getInstance() {
        static DataProcessor instance;
        return instance;
    }
    DataProcessor(const DataProcessor&) = delete;
    DataProcessor& operator=(const DataProcessor&) = delete;

    std::vector<Movie> loadMovies(const std::string& filename);           // paralelo
    std::vector<Movie> loadMoviesSequential(const std::string& filename); // secuencial (comparación)

    std::vector<std::string> cleanAndSplitText(const std::string& text);
};

#endif // DATAPROCESSOR_H
