#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <string>
#include <vector>
#include <unordered_map>

// Maps unique terms to the movies that contain them. Each distinct term is stored
// once (deduplicated vocabulary) with a posting list of movie ids, instead of
// repeating the term for every movie occurrence.
class InvertedIndex {
public:
    // Register that `movieId` contains `term`. Returns the term's stable id.
    // Movies are expected to be added in non-decreasing id order, which keeps every
    // posting list sorted and lets duplicates collapse with an O(1) tail check.
    int addOccurrence(const std::string& term, int movieId);

    int termCount() const { return static_cast<int>(vocabulary_.size()); }

    // Returns the id of an exact term, or -1 if the term is not in the vocabulary.
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
