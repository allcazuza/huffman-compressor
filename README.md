# Huffman Compressor

Compressor de arquivos sem perdas (lossless), em C puro, implementando o
algoritmo de codificação de Huffman do zero — sem bibliotecas externas de
compressão. O projeto inclui uma implementação própria de min-heap (fila
de prioridade), a construção da árvore de Huffman, e um esquema de I/O
bit a bit para empacotar códigos de comprimento variável em um arquivo
binário.

```
$ ./huffman compress relatorio.txt relatorio.huff
Arquivo original:   119200 bytes
Arquivo comprimido: 65412 bytes
Espaco economizado: 45.12%

$ ./huffman decompress relatorio.huff relatorio_restaurado.txt
Descompressao concluida: 119200 bytes escritos em 'relatorio_restaurado.txt'
```

## Por que este projeto

A ideia não é reinventar o `gzip`, e sim mostrar, de forma direta e
verificável, o domínio de conceitos fundamentais de Engenharia da
Computação:

- **Estruturas de dados:** implementação manual de uma fila de
  prioridade (min-heap binário) e de uma árvore binária.
- **Algoritmos gulosos:** construção ótima de códigos de comprimento
  variável via Huffman, com análise de complexidade.
- **Manipulação de bits e formatos binários:** leitura/escrita bit a
  bit, definição de um formato de arquivo próprio (`.huff`) com
  cabeçalho versionado.
- **Gerência de memória em C:** alocação/liberação manual de árvores e
  buffers, sem vazamentos (testado com `valgrind`, ver seção Testes).
- **Rigor com casos extremos:** arquivo vazio, arquivo com um único
  símbolo distinto, e dados de alta entropia são tratados
  explicitamente e testados — não ignorados.

## Como o algoritmo funciona (resumo)

1. Conta-se a frequência de cada byte (0–255) no arquivo de entrada.
2. Constrói-se a árvore de Huffman: os dois símbolos (ou sub-árvores)
   de menor frequência são repetidamente combinados em um nó pai, até
   sobrar uma única árvore. Símbolos frequentes acabam com códigos
   binários curtos; símbolos raros, com códigos mais longos.
3. Cada byte do arquivo é substituído pelo seu código de Huffman
   (sequência de bits), gerando um fluxo de bits compactado.
4. A tabela de frequências é salva no cabeçalho do arquivo `.huff` —
   isso é suficiente para reconstruir a **mesma** árvore na
   descompressão (a construção é determinística), sem precisar
   serializar a árvore em si.

Uma explicação bem mais detalhada, com a análise de complexidade e a
justificativa de cada decisão de design, está em
[`docs/how_it_works.md`](docs/how_it_works.md).

## Estrutura do projeto

```
huffman-compressor/
├── README.md
├── LICENSE
├── Makefile
├── .gitignore
├── src/
│   ├── main.c        # CLI, formato do arquivo .huff, compress/decompress
│   ├── huffman.h      # Interface: nó, construção da árvore, geração de códigos
│   ├── huffman.c      # Implementação da árvore de Huffman
│   ├── heap.h         # Interface do min-heap
│   ├── heap.c         # Min-heap binário (fila de prioridade)
│   ├── bitio.h        # Interface de I/O bit a bit
│   └── bitio.c        # BitWriter / BitReader
├── tests/
│   ├── run_tests.sh   # Testes de integração (roundtrip + edge cases)
│   └── test_files/
│       └── sample.txt
└── docs/
    └── how_it_works.md
```

## Como compilar e rodar

Pré-requisitos: `gcc` (ou outro compilador C11) e `make`.

```bash
git clone https://github.com/allcazuza/huffman-compressor.git
cd huffman-compressor
make
```

Isso gera o executável `./huffman`. Uso:

```bash
./huffman compress   <entrada>       <saida.huff>
./huffman decompress <entrada.huff>  <saida>
```

Exemplo rápido com o arquivo de teste incluso:

```bash
./huffman compress   tests/test_files/sample.txt sample.huff
./huffman decompress sample.huff restored.txt
diff tests/test_files/sample.txt restored.txt   # sem saída = arquivos idênticos
```

## Testes

O script `tests/run_tests.sh` comprime e descomprime quatro cenários e
verifica, byte a byte, que a saída é idêntica à entrada original:

```bash
make
./tests/run_tests.sh
```

Cenários cobertos:

| Cenário                          | O que valida                                              |
|-----------------------------------|-----------------------------------------------------------|
| Texto repetitivo (`sample.txt`)   | Caso comum — compressão real deve ocorrer                 |
| Arquivo vazio                     | `num_symbols == 0`, sem árvore, sem bits                  |
| Um único símbolo repetido         | Árvore degenerada (1 folha), código forçado               |
| Binário aleatório (`/dev/urandom`)| Alta entropia — sem ganho de compressão, mas sem corrupção |

Verificação de memória (opcional, requer `valgrind`):

```bash
valgrind --leak-check=full ./huffman compress tests/test_files/sample.txt /tmp/out.huff
```

## Resultados observados

| Tipo de arquivo                         | Tamanho original | Tamanho comprimido | Economia |
|------------------------------------------|-------------------|----------------------|----------|
| Texto em português, repetitivo (119 KB)   | 119.200 bytes     | 65.412 bytes         | ~45%     |
| Texto curto (302 bytes)                   | 302 bytes         | 488 bytes            | −62% (*) |
| Binário aleatório (5 KB)                  | 5.000 bytes       | 7.313 bytes          | −46% (*) |

(*) Nos casos de arquivos pequenos ou de alta entropia, o compressor
**aumenta** o tamanho do arquivo — e isso é esperado, não um defeito.
O motivo está explicado em detalhe em
[`docs/how_it_works.md`](docs/how_it_works.md#5-casos-extremos-tratados-explicitamente):
o cabeçalho tem um custo fixo por símbolo distinto, e dados sem
redundância estatística (entropia alta) não podem ser comprimidos sem
perdas — é um limite teórico (entropia de Shannon), não uma limitação
desta implementação específica.

## Possíveis extensões futuras

- Combinar com LZ77 (janela deslizante) para capturar repetições de
  sequências, não só de bytes individuais — essa é, essencialmente,
  a ideia por trás do DEFLATE (usado no `gzip`/`zip`).
- Empacotar a tabela de frequências de forma mais compacta para
  reduzir o overhead em arquivos pequenos.
- Suporte a compressão em streaming (sem carregar o arquivo inteiro
  em memória).

## Licença

Distribuído sob a licença MIT — veja [`LICENSE`](LICENSE).
