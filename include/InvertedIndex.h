#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <string>
#include <vector>
#include <unordered_map>

// Mapea términos únicos a las películas que los contienen. Cada término se guarda
// una sola vez (vocabulario deduplicado) con una posting list de ids de película,
// en vez de repetir el término por cada aparición.
class InvertedIndex {
public:
    // Registra que `movieId` contiene `term`. Devuelve el id estable del término.
    // Las películas se agregan en orden de id no decreciente, lo que mantiene cada
    // posting list ordenada y colapsa duplicados con un chequeo O(1) al final.
    int addOccurrence(const std::string& term, int movieId);

    int termCount() const { return static_cast<int>(vocabulary_.size()); }

    // Devuelve el id de un término exacto, o -1 si no está en el vocabulario.
    int findTerm(const std::string& term) const;

    const std::string& term(int termId) const { return vocabulary_[termId]; }
    const std::vector<int>& postings(int termId) const { return postings_[termId]; }

    void clear();

private:
    std::unordered_map<std::string, int> termToId_;
    std::vector<std::string> vocabulary_;
    std::vector<std::vector<int>> postings_;
};

#endif // INVERTED_INDEX_H
