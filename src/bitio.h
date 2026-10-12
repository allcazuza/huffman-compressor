#ifndef BITIO_H
#define BITIO_H

#include <stdio.h>

/*
 * bitio.h
 * -------
 * Utilitários para escrever e ler bits individuais em um FILE*,
 * acumulando-os em um buffer de 1 byte. Necessário porque os
 * códigos de Huffman têm comprimento variável (não múltiplo de 8),
 * mas o sistema de arquivos só trabalha em bytes.
 */

typedef struct {
    FILE *fp;
    unsigned char buffer;
    int bit_count; /* quantos bits já estão acumulados no buffer (0-7) */
} BitWriter;

typedef struct {
    FILE *fp;
    unsigned char buffer;
    int bit_count; /* quantos bits ainda não foram consumidos do buffer */
} BitReader;

void bitwriter_init(BitWriter *bw, FILE *fp);
void bitwriter_write_bit(BitWriter *bw, int bit);
void bitwriter_write_string(BitWriter *bw, const char *bits);
/* Descarrega bits pendentes, preenchendo com zeros à direita. */
void bitwriter_flush(BitWriter *bw);

void bitreader_init(BitReader *br, FILE *fp);
/* Retorna 0, 1, ou -1 em caso de fim de arquivo. */
int bitreader_read_bit(BitReader *br);

#endif /* BITIO_H */
