#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <vector>
#include <unordered_map>

struct TrieNode {
    std::unordered_map<char, TrieNode*> children;
    std::vector<int> movieIds;
    bool isEndOfWord = false;

    TrieNode() = default;
};

class Trie {
public:
    Trie();
    ~Trie();

    void insert(const std::string& word, int movieId);
    std::vector<int> search(const std::string& query);

private:
    TrieNode* root;

    void destroy(TrieNode* node);
};

#endif // TRIE_H
