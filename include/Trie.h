#ifndef TRIE_H
#define TRIE_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// Índice de sufijos sobre un conjunto de términos. Cada palabra insertada marca
// SOLO su nodo terminal con un id; los nodos intermedios ocupan O(1). Una consulta
// de prefijo/subcadena baja hasta el nodo y recolecta por DFS todos los ids del subárbol.
// Los nodos se poseen con unique_ptr: el árbol no se puede copiar superficialmente y
// la destrucción es automática (sin delete manual).
struct TrieNode {
    std::unordered_map<char, std::unique_ptr<TrieNode>> children;
    std::vector<int> termIds; // ids guardados solo donde termina una palabra (nodo terminal)
};

class Trie {
public:
    Trie();

    // Inserta una palabra y marca su nodo terminal con termId.
    void insert(const std::string& word, int termId);

    // Recolecta (DFS) todos los termId del subárbol alcanzado por `query`.
    // Devuelve los ids ordenados y sin duplicados; vacío si el camino no existe.
    std::vector<int> collect(const std::string& query) const;

    void clear();

private:
    std::unique_ptr<TrieNode> root;

    static void gather(const TrieNode* node, std::vector<int>& out);
};

#endif // TRIE_H
