#ifndef HEAP_H
#define HEAP_H

#include "huffman.h"

/*
 * heap.h
 * ------
 * Min-heap binário implementado sobre um vetor, usado como fila de
 * prioridade para escolher, a cada passo do algoritmo de Huffman,
 * os dois nós de menor frequência. Implementado manualmente (sem
 * depender de bibliotecas externas) para deixar explícita a
 * complexidade O(log n) de inserção/remoção.
 */

typedef struct {
    Node **data;
    int size;
    int capacity;
} MinHeap;

MinHeap *heap_create(int capacity);
void heap_free(MinHeap *heap);

/* Insere um nó no heap. O(log n). */
void heap_insert(MinHeap *heap, Node *node);

/* Remove e retorna o nó de menor frequência. O(log n). */
Node *heap_extract_min(MinHeap *heap);

int heap_size(MinHeap *heap);

#endif /* HEAP_H */
