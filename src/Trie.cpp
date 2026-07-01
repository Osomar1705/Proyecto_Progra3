#include "Trie.h"
#include <algorithm>

Trie::Trie() : root(std::make_unique<TrieNode>()) {}

void Trie::insert(const std::string& word, int termId) {
    TrieNode* current = root.get();
    for (char c : word) {
        std::unique_ptr<TrieNode>& next = current->children[c];
        if (!next) next = std::make_unique<TrieNode>();
        current = next.get();
    }
    // Payload lives only at the terminal node. Suffixes of a single word are all
    // distinct, so the only possible repeat is the same termId inserted twice.
    if (current->termIds.empty() || current->termIds.back() != termId) {
        current->termIds.push_back(termId);
    }
}

void Trie::gather(const TrieNode* node, std::vector<int>& out) {
    if (!node) return;
    out.insert(out.end(), node->termIds.begin(), node->termIds.end());
    for (const auto& pair : node->children) {
        gather(pair.second.get(), out);
    }
}

std::vector<int> Trie::collect(const std::string& query) const {
    const TrieNode* current = root.get();
    for (char c : query) {
        auto it = current->children.find(c);
        if (it == current->children.end()) {
            return {};
        }
        current = it->second.get();
    }

    std::vector<int> result;
    gather(current, result);
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

void Trie::clear() {
    root->children.clear();
    root->termIds.clear();
}
