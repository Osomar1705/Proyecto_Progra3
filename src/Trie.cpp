#include "Trie.h"

Trie::Trie() : root(new TrieNode()) {}

Trie::~Trie() {
    destroy(root);
    root = nullptr;
}

void Trie::destroy(TrieNode* node) {
    if (!node) return;
    for (auto& kv : node->children) {
        destroy(kv.second);
    }
    delete node;
}

void Trie::insert(const std::string& word, int movieId) {
    if (word.empty()) return;

    // Insertar cada sufijo de la palabra. Asociar movieId a cada nodo del camino
    // permite que cualquier prefijo de un sufijo (= cualquier substring de word)
    // recupere la pelicula en O(|substring|).
    for (size_t start = 0; start < word.size(); ++start) {
        TrieNode* node = root;
        for (size_t i = start; i < word.size(); ++i) {
            char c = word[i];
            auto it = node->children.find(c);
            if (it == node->children.end()) {
                TrieNode* child = new TrieNode();
                node->children.emplace(c, child);
                node = child;
            } else {
                node = it->second;
            }
            node->movieIds.insert(movieId);
        }
    }
}

std::vector<int> Trie::search(const std::string& query) const {
    if (query.empty()) return {};

    TrieNode* node = root;
    for (char c : query) {
        auto it = node->children.find(c);
        if (it == node->children.end()) return {};
        node = it->second;
    }
    return std::vector<int>(node->movieIds.begin(), node->movieIds.end());
}
