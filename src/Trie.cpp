#include "Trie.h"
#include <algorithm>

Trie::Trie() : root(new TrieNode()) {}

Trie::~Trie() {
    destroy(root);
}

void Trie::destroy(TrieNode* node) {
    if (!node) return;
    for (auto& pair : node->children) {
        destroy(pair.second);
    }
    delete node;
}

void Trie::insert(const std::string& word, int movieId) {
    TrieNode* current = root;
    for (char c : word) {
        if (current->children.find(c) == current->children.end()) {
            current->children[c] = new TrieNode();
        }
        current = current->children[c];
        
        // Evitar duplicados de IDs en el mismo nodo
        if (std::find(current->movieIds.begin(), current->movieIds.end(), movieId) == current->movieIds.end()) {
            current->movieIds.push_back(movieId);
        }
    }
    current->isEndOfWord = true;
}

std::vector<int> Trie::search(const std::string& query) {
    TrieNode* current = root;
    for (char c : query) {
        if (current->children.find(c) == current->children.end()) {
            return {};
        }
        current = current->children[c];
    }
    return current->movieIds;
}

void Trie::clear() {
    for (auto& pair : root->children) {
        destroy(pair.second);
    }
    root->children.clear();
    root->movieIds.clear();
    root->isEndOfWord = false;
}
