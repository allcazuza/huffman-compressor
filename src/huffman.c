#include <stdlib.h>
#include <string.h>
#include "huffman.h"
#include "heap.h"

Node *huffman_create_node(unsigned char byte, unsigned long freq, Node *left, Node *right) {
    Node *node = malloc(sizeof(Node));
    node->byte = byte;
    node->freq = freq;
    node->left = left;
    node->right = right;
    return node;
}

void huffman_free_tree(Node *root) {
    if (root == NULL) {
        return;
    }
    huffman_free_tree(root->left);
    huffman_free_tree(root->right);
    free(root);
}

Node *huffman_build_tree(const unsigned long freq[ALPHABET_SIZE], int *num_symbols) {
    MinHeap *heap = heap_create(ALPHABET_SIZE);
    int count = 0;

    /* Passo 1: cada símbolo presente vira uma folha e entra no heap. */
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (freq[i] > 0) {
            heap_insert(heap, huffman_create_node((unsigned char)i, freq[i], NULL, NULL));
            count++;
        }
    }
    *num_symbols = count;

    if (count == 0) {
        heap_free(heap);
        return NULL; /* arquivo vazio */
    }

    /*
     * Passo 2: enquanto houver mais de um nó no heap, remove os dois
     * de menor frequência e cria um nó interno pai cuja frequência é
     * a soma dos dois filhos. Esse novo nó volta para o heap.
     * Ao final, sobra um único nó: a raiz da árvore de Huffman.
     */
    while (heap_size(heap) > 1) {
        Node *a = heap_extract_min(heap);
        Node *b = heap_extract_min(heap);
        Node *parent = huffman_create_node(0, a->freq + b->freq, a, b);
        heap_insert(heap, parent);
    }

    Node *root = heap_extract_min(heap);
    heap_free(heap);
    return root;
}

static void generate_codes_rec(Node *node, char *buffer, int depth,
                                char codes[ALPHABET_SIZE][MAX_CODE_LEN]) {
    if (node == NULL) {
        return;
    }

    /* Nó-folha: grava o código acumulado no buffer para esse símbolo. */
    if (node->left == NULL && node->right == NULL) {
        if (depth == 0) {
            /*
             * Caso especial: a árvore inteira é uma única folha
             * (arquivo com apenas um símbolo distinto). Sem essa
             * regra, o código gerado seria uma string vazia, o que
             * não é um código válido. Forçamos o código "0".
             */
            codes[node->byte][0] = '0';
            codes[node->byte][1] = '\0';
        } else {
            buffer[depth] = '\0';
            strcpy(codes[node->byte], buffer);
        }
        return;
    }

    buffer[depth] = '0';
    generate_codes_rec(node->left, buffer, depth + 1, codes);

    buffer[depth] = '1';
    generate_codes_rec(node->right, buffer, depth + 1, codes);
}

void huffman_generate_codes(Node *root, char codes[ALPHABET_SIZE][MAX_CODE_LEN]) {
    char buffer[MAX_CODE_LEN];
    generate_codes_rec(root, buffer, 0, codes);
}
