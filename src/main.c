/*
 * main.c
 * ------
 * CLI do compressor Huffman.
 *
 * Uso:
 *   huffman compress   <entrada> <saida.huff>
 *   huffman decompress <entrada.huff> <saida>
 *
 * Formato do arquivo .huff (little-endian):
 *
 *   offset  tamanho  campo
 *   0       4        magic "HUFC"
 *   4       1        versao (1)
 *   5       8        tamanho original em bytes (uint64)
 *   13      2        numero de simbolos distintos (uint16, 0..256)
 *   15      N*9      tabela de simbolos: para cada simbolo,
 *                    1 byte de valor + 8 bytes de frequencia (uint64)
 *   15+N*9  ...       fluxo de bits com os codigos de Huffman
 *                     (ausente se numero de simbolos for 0 ou 1)
 *
 * A tabela de frequencias e suficiente para reconstruir a MESMA
 * arvore de Huffman na descompressao (o algoritmo de construcao e
 * deterministico dado o mesmo vetor de frequencias), entao nao e
 * necessario serializar a arvore em si.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"
#include "bitio.h"

static void write_u64(FILE *fp, unsigned long long v) {
    for (int i = 0; i < 8; i++) {
        fputc((int)((v >> (8 * i)) & 0xFF), fp);
    }
}

static unsigned long long read_u64(FILE *fp) {
    unsigned long long v = 0;
    for (int i = 0; i < 8; i++) {
        v |= ((unsigned long long)(unsigned char)fgetc(fp)) << (8 * i);
    }
    return v;
}

static void write_u16(FILE *fp, unsigned short v) {
    fputc(v & 0xFF, fp);
    fputc((v >> 8) & 0xFF, fp);
}

static unsigned short read_u16(FILE *fp) {
    int lo = fgetc(fp);
    int hi = fgetc(fp);
    return (unsigned short)(lo | (hi << 8));
}

static unsigned char *read_entire_file(const char *path, long *out_size) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Erro ao abrir '%s': ", path);
        perror("");
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char *buf = malloc((size_t)(size > 0 ? size : 1));
    if (size > 0) {
        size_t read = fread(buf, 1, (size_t)size, fp);
        if ((long)read != size) {
            fprintf(stderr, "Erro de leitura em '%s'\n", path);
            exit(1);
        }
    }
    fclose(fp);
    *out_size = size;
    return buf;
}

static void compress(const char *input_path, const char *output_path) {
    long size;
    unsigned char *data = read_entire_file(input_path, &size);

    /* 1. Conta a frequencia de cada byte (0-255) no arquivo. */
    unsigned long freq[ALPHABET_SIZE] = {0};
    for (long i = 0; i < size; i++) {
        freq[data[i]]++;
    }

    /* 2. Constroi a arvore de Huffman e gera os codigos. */
    int num_symbols;
    Node *root = huffman_build_tree(freq, &num_symbols);

    char codes[ALPHABET_SIZE][MAX_CODE_LEN];
    memset(codes, 0, sizeof(codes));
    if (root != NULL) {
        huffman_generate_codes(root, codes);
    }

    /* 3. Escreve o cabecalho: magic, tamanho original, tabela de frequencias. */
    FILE *out = fopen(output_path, "wb");
    if (out == NULL) {
        fprintf(stderr, "Erro ao criar '%s': ", output_path);
        perror("");
        exit(1);
    }

    fputc('H', out);
    fputc('U', out);
    fputc('F', out);
    fputc('C', out);
    fputc(1, out); /* versao do formato */
    write_u64(out, (unsigned long long)size);
    write_u16(out, (unsigned short)num_symbols);

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (freq[i] > 0) {
            fputc(i, out);
            write_u64(out, (unsigned long long)freq[i]);
        }
    }

    /*
     * 4. Escreve o fluxo de bits codificado. Se houver 0 ou 1 simbolo
     * distinto, o cabecalho ja contem toda a informacao necessaria
     * para reconstruir o arquivo, entao nenhum bit e escrito.
     */
    if (num_symbols >= 2) {
        BitWriter bw;
        bitwriter_init(&bw, out);
        for (long i = 0; i < size; i++) {
            bitwriter_write_string(&bw, codes[data[i]]);
        }
        bitwriter_flush(&bw);
    }

    fclose(out);
    if (root != NULL) {
        huffman_free_tree(root);
    }
    free(data);

    /* Estatisticas para o usuario. */
    FILE *check = fopen(output_path, "rb");
    fseek(check, 0, SEEK_END);
    long compressed_size = ftell(check);
    fclose(check);

    printf("Arquivo original:   %ld bytes\n", size);
    printf("Arquivo comprimido: %ld bytes\n", compressed_size);
    if (size > 0) {
        double ratio = 100.0 * (1.0 - ((double)compressed_size / (double)size));
        printf("Espaco economizado: %.2f%%\n", ratio);
    }
}

static void decompress(const char *input_path, const char *output_path) {
    FILE *in = fopen(input_path, "rb");
    if (in == NULL) {
        fprintf(stderr, "Erro ao abrir '%s': ", input_path);
        perror("");
        exit(1);
    }

    char magic[4];
    for (int i = 0; i < 4; i++) {
        magic[i] = (char)fgetc(in);
    }
    if (memcmp(magic, "HUFC", 4) != 0) {
        fprintf(stderr, "Erro: '%s' nao e um arquivo .huff valido\n", input_path);
        fclose(in);
        exit(1);
    }
    fgetc(in); /* versao (nao usada por enquanto) */

    unsigned long long original_size = read_u64(in);
    unsigned short num_symbols = read_u16(in);

    unsigned long freq[ALPHABET_SIZE] = {0};
    unsigned char single_byte = 0;
    for (int i = 0; i < num_symbols; i++) {
        int byte_val = fgetc(in);
        unsigned long long f = read_u64(in);
        freq[byte_val] = (unsigned long)f;
        single_byte = (unsigned char)byte_val;
    }

    FILE *out = fopen(output_path, "wb");
    if (out == NULL) {
        fprintf(stderr, "Erro ao criar '%s': ", output_path);
        perror("");
        exit(1);
    }

    if (num_symbols == 0) {
        /* Arquivo original era vazio; nada a fazer. */
    } else if (num_symbols == 1) {
        /* Atalho: um unico byte repetido `original_size` vezes. */
        for (unsigned long long i = 0; i < original_size; i++) {
            fputc(single_byte, out);
        }
    } else {
        int dummy;
        Node *root = huffman_build_tree(freq, &dummy);

        BitReader br;
        bitreader_init(&br, in);

        unsigned long long written = 0;
        while (written < original_size) {
            Node *node = root;
            while (node->left != NULL || node->right != NULL) {
                int bit = bitreader_read_bit(&br);
                if (bit < 0) {
                    fprintf(stderr, "Erro: fim inesperado dos dados comprimidos\n");
                    exit(1);
                }
                node = (bit == 0) ? node->left : node->right;
            }
            fputc(node->byte, out);
            written++;
        }
        huffman_free_tree(root);
    }

    fclose(in);
    fclose(out);
    printf("Descompressao concluida: %llu bytes escritos em '%s'\n",
           original_size, output_path);
}

static void print_usage(const char *prog) {
    fprintf(stderr, "Uso:\n");
    fprintf(stderr, "  %s compress   <entrada> <saida.huff>\n", prog);
    fprintf(stderr, "  %s decompress <entrada.huff> <saida>\n", prog);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    const char *mode = argv[1];
    const char *input = argv[2];
    const char *output = argv[3];

    if (strcmp(mode, "compress") == 0) {
        compress(input, output);
    } else if (strcmp(mode, "decompress") == 0) {
        decompress(input, output);
    } else {
        fprintf(stderr, "Modo desconhecido: '%s'\n", mode);
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
