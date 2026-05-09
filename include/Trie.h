#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// Nodo del Trie de Sufijos: cada nodo representa un prefijo de algun sufijo
// insertado. movieIds guarda los IDs de peliculas que contienen ese substring.
struct TrieNode {
    std::unordered_map<char, TrieNode*> children;
    std::unordered_set<int> movieIds;

    TrieNode() = default;
};

// Trie de Sufijos: para cada palabra W de longitud n se insertan los n sufijos
// W[0..], W[1..], ..., W[n-1..]. Esto permite buscar substrings (no solo
// prefijos) en O(|query|) sin re-escanear el texto.
class Trie {
public:
    Trie();
    ~Trie();

    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

    // Inserta todos los sufijos de `word` y asocia `movieId` a cada nodo del camino.
    void insert(const std::string& word, int movieId);

    // Devuelve los IDs de peliculas que contienen `query` como substring de
    // alguna palabra indexada.
    std::vector<int> search(const std::string& query) const;

private:
    TrieNode* root;

    void destroy(TrieNode* node);
};

#endif // TRIE_H
