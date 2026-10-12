#include "bitio.h"

void bitwriter_init(BitWriter *bw, FILE *fp) {
    bw->fp = fp;
    bw->buffer = 0;
    bw->bit_count = 0;
}

void bitwriter_write_bit(BitWriter *bw, int bit) {
    bw->buffer = (unsigned char)((bw->buffer << 1) | (bit & 1));
    bw->bit_count++;
    if (bw->bit_count == 8) {
        fputc(bw->buffer, bw->fp);
        bw->buffer = 0;
        bw->bit_count = 0;
    }
}

void bitwriter_write_string(BitWriter *bw, const char *bits) {
    for (const char *p = bits; *p != '\0'; p++) {
        bitwriter_write_bit(bw, *p - '0');
    }
}

void bitwriter_flush(BitWriter *bw) {
    if (bw->bit_count > 0) {
        bw->buffer = (unsigned char)(bw->buffer << (8 - bw->bit_count));
        fputc(bw->buffer, bw->fp);
        bw->buffer = 0;
        bw->bit_count = 0;
    }
}

void bitreader_init(BitReader *br, FILE *fp) {
    br->fp = fp;
    br->buffer = 0;
    br->bit_count = 0;
}

int bitreader_read_bit(BitReader *br) {
    if (br->bit_count == 0) {
        int c = fgetc(br->fp);
        if (c == EOF) {
            return -1;
        }
        br->buffer = (unsigned char)c;
        br->bit_count = 8;
    }
    int bit = (br->buffer >> 7) & 1;
    br->buffer = (unsigned char)(br->buffer << 1);
    br->bit_count--;
    return bit;
}
