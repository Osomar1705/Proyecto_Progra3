#ifndef TRIE_H
#define TRIE_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// A suffix index over a set of terms. Each inserted word tags ONLY its terminal
// node with a payload id, keeping intermediate nodes at O(1) space. A prefix/substring
// query walks to the matching node and gathers every payload in the subtree via DFS.
// Nodes are owned through unique_ptr, so the tree cannot be shallow-copied and
// destruction is automatic (no manual delete).
struct TrieNode {
    std::unordered_map<char, std::unique_ptr<TrieNode>> children;
    std::vector<int> termIds; // payloads stored only where a word ends (terminal node)
};

class Trie {
public:
    Trie();

    // Insert a word, tagging its terminal node with termId.
    void insert(const std::string& word, int termId);

    // Collect every termId stored in the subtree reached by `query` (DFS).
    // Returns the ids sorted and deduplicated; empty if the path does not exist.
    std::vector<int> collect(const std::string& query) const;

    void clear();

private:
    std::unique_ptr<TrieNode> root;

    static void gather(const TrieNode* node, std::vector<int>& out);
};

#endif // TRIE_H
