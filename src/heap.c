#include <stdlib.h>
#include "heap.h"

MinHeap *heap_create(int capacity) {
    MinHeap *heap = malloc(sizeof(MinHeap));
    heap->data = malloc(sizeof(Node *) * (size_t)capacity);
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

void heap_free(MinHeap *heap) {
    free(heap->data);
    free(heap);
}

static void swap(Node **a, Node **b) {
    Node *tmp = *a;
    *a = *b;
    *b = tmp;
}

/* Reposiciona o elemento em `idx` subindo enquanto for menor que o pai. */
static void sift_up(MinHeap *heap, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (heap->data[parent]->freq <= heap->data[idx]->freq) {
            break;
        }
        swap(&heap->data[parent], &heap->data[idx]);
        idx = parent;
    }
}

/* Reposiciona o elemento em `idx` descendo até restaurar a propriedade de heap. */
static void sift_down(MinHeap *heap, int idx) {
    for (;;) {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        int smallest = idx;

        if (left < heap->size && heap->data[left]->freq < heap->data[smallest]->freq) {
            smallest = left;
        }
        if (right < heap->size && heap->data[right]->freq < heap->data[smallest]->freq) {
            smallest = right;
        }
        if (smallest == idx) {
            break;
        }
        swap(&heap->data[idx], &heap->data[smallest]);
        idx = smallest;
    }
}

void heap_insert(MinHeap *heap, Node *node) {
    heap->data[heap->size] = node;
    sift_up(heap, heap->size);
    heap->size++;
}

Node *heap_extract_min(MinHeap *heap) {
    if (heap->size == 0) {
        return NULL;
    }
    Node *min = heap->data[0];
    heap->size--;
    heap->data[0] = heap->data[heap->size];
    sift_down(heap, 0);
    return min;
}

int heap_size(MinHeap *heap) {
    return heap->size;
}
