#include "InvertedIndex.h"

int InvertedIndex::addOccurrence(const std::string& term, int movieId) {
    auto it = termToId_.find(term);
    int termId;
    if (it == termToId_.end()) {
        termId = static_cast<int>(vocabulary_.size());
        termToId_.emplace(term, termId);
        vocabulary_.push_back(term);
        postings_.emplace_back();
    } else {
        termId = it->second;
    }

    std::vector<int>& plist = postings_[termId];
    if (plist.empty() || plist.back() != movieId) {
        plist.push_back(movieId);
    }
    return termId;
}

void InvertedIndex::clear() {
    termToId_.clear();
    vocabulary_.clear();
    postings_.clear();
}
