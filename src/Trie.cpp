#include "Trie.h"

Trie::Trie() : root(new TrieNode()) {
    // TODO: inicializar cualquier estado adicional si se requiere.
}

Trie::~Trie() {
    destroy(root);
    root = nullptr;
}

void Trie::destroy(TrieNode* node) {
    // TODO: recorrer recursivamente todos los hijos y liberar memoria.
    (void)node;
}

void Trie::insert(const std::string& word, int movieId) {
    // TODO: recorrer/crear nodos por cada caracter de `word` y registrar
    // `movieId` en el nodo final (o en cada nodo, segun la estrategia).
    (void)word;
    (void)movieId;
}

std::vector<int> Trie::search(const std::string& query) {
    // TODO: descender por el Trie segun los caracteres de `query` y devolver
    // los IDs de peliculas asociados al nodo encontrado.
    (void)query;
    return {};
}
