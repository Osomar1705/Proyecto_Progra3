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

void Trie::insert(const std::string& word, int termId) {
    TrieNode* current = root;
    for (char c : word) {
        TrieNode*& next = current->children[c];
        if (!next) next = new TrieNode();
        current = next;
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
        gather(pair.second, out);
    }
}

std::vector<int> Trie::collect(const std::string& query) const {
    const TrieNode* current = root;
    for (char c : query) {
        auto it = current->children.find(c);
        if (it == current->children.end()) {
            return {};
        }
        current = it->second;
    }

    std::vector<int> result;
    gather(current, result);
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

void Trie::clear() {
    for (auto& pair : root->children) {
        destroy(pair.second);
    }
    root->children.clear();
    root->termIds.clear();
}
