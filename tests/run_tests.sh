#!/usr/bin/env bash
#
# run_tests.sh
# ------------
# Testes de integração simples: comprime e descomprime cada arquivo
# de teste e verifica, byte a byte, que o resultado é idêntico ao
# original. Cobre o caso comum (texto), o caso vazio e o caso de
# um único símbolo distinto (edge case da árvore de 1 folha).
#
# Uso: ./tests/run_tests.sh   (execute a partir da raiz do projeto)

set -e

BIN=./huffman
TMP_DIR=$(mktemp -d)
FAIL=0

trap 'rm -rf "$TMP_DIR"' EXIT

check_roundtrip() {
    local name="$1"
    local input="$2"
    local huff="$TMP_DIR/${name}.huff"
    local restored="$TMP_DIR/${name}.restored"

    "$BIN" compress "$input" "$huff" > /dev/null
    "$BIN" decompress "$huff" "$restored" > /dev/null

    if cmp -s "$input" "$restored"; then
        echo "[OK]   $name"
    else
        echo "[FAIL] $name"
        FAIL=1
    fi
}

# Caso 1: texto real, repetitivo
check_roundtrip "sample_text" "tests/test_files/sample.txt"

# Caso 2: arquivo vazio
: > "$TMP_DIR/empty.txt"
check_roundtrip "empty_file" "$TMP_DIR/empty.txt"

# Caso 3: um único símbolo distinto, repetido muitas vezes
python3 -c "open('$TMP_DIR/single.txt','w').write('x' * 1000)"
check_roundtrip "single_symbol" "$TMP_DIR/single.txt"

# Caso 4: dados binários aleatórios (pior caso para o algoritmo)
head -c 4096 /dev/urandom > "$TMP_DIR/random.bin"
check_roundtrip "random_binary" "$TMP_DIR/random.bin"

if [ "$FAIL" -eq 0 ]; then
    echo
    echo "Todos os testes passaram."
else
    echo
    echo "Algum teste falhou."
    exit 1
fi
