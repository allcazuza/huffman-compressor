#ifndef HUFFMAN_H
#define HUFFMAN_H

/*
 * huffman.h
 * ---------
 * Define a estrutura de nó da árvore de Huffman e as operações
 * de alto nível: construir a árvore a partir de uma tabela de
 * frequências e gerar os códigos binários de cada símbolo.
 */

#define ALPHABET_SIZE 256   /* um símbolo por valor de byte (0-255) */
#define MAX_CODE_LEN  256   /* comprimento máximo teórico de um código */

typedef struct Node {
    unsigned char byte;      /* válido apenas em nós-folha */
    unsigned long freq;
    struct Node *left;
    struct Node *right;
} Node;

/* Cria um nó (folha se left == right == NULL, interno caso contrário). */
Node *huffman_create_node(unsigned char byte, unsigned long freq, Node *left, Node *right);

/*
 * Constrói a árvore de Huffman a partir da tabela de frequências.
 * `num_symbols` recebe a quantidade de símbolos distintos (0..256).
 * Retorna NULL se num_symbols == 0 (arquivo vazio).
 */
Node *huffman_build_tree(const unsigned long freq[ALPHABET_SIZE], int *num_symbols);

/*
 * Preenche `codes[byte]` com a string de bits ('0'/'1' + '\0') que
 * representa o código de Huffman daquele byte. Assume-se que
 * `codes` foi zerado previamente pelo chamador.
 */
void huffman_generate_codes(Node *root, char codes[ALPHABET_SIZE][MAX_CODE_LEN]);

/* Libera recursivamente todos os nós da árvore. */
void huffman_free_tree(Node *root);

#endif /* HUFFMAN_H */
