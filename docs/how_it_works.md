# Como funciona

Este documento explica, em profundidade, o algoritmo e as decisões de
implementação — útil como referência para estudar antes de explicar o
projeto em uma entrevista.

## 1. A ideia central

Em um arquivo comum (texto, principalmente), alguns bytes aparecem com
muito mais frequência do que outros. Se usarmos códigos binários de
**comprimento variável** — bits curtos para os bytes mais frequentes e
bits longos para os mais raros — o tamanho médio necessário para
representar o arquivo cai, desde que nenhum código seja prefixo de
outro (propriedade "prefix-free", que garante decodificação sem
ambiguidade).

O algoritmo de Huffman (1952) constrói o conjunto de códigos de
comprimento variável **ótimo** para uma dada distribuição de
frequências, usando uma árvore binária: cada folha é um símbolo, e o
código de um símbolo é o caminho da raiz até a folha (0 = esquerda,
1 = direita).

## 2. Construção da árvore — passo a passo

1. Conta-se a frequência de cada um dos 256 valores de byte possíveis
   no arquivo de entrada (`freq[256]`).
2. Cada byte com frequência > 0 vira uma folha e é inserido em uma
   **fila de prioridade (min-heap)** ordenada pela frequência.
3. Repetidamente:
   - Remove-se os dois nós de menor frequência do heap.
   - Cria-se um novo nó interno, cujo valor de frequência é a soma
     dos dois, e que aponta para eles como filho esquerdo e direito.
   - Esse novo nó volta para o heap.
4. Quando resta um único nó no heap, ele é a raiz da árvore de
   Huffman completa.

Símbolos raros acabam "afundados" mais fundo na árvore (código mais
longo); símbolos frequentes ficam perto da raiz (código mais curto).

**Complexidade:** com `n` símbolos distintos (no máximo 256 aqui),
cada extração/inserção no heap é `O(log n)`, e fazemos isso `O(n)`
vezes → construção da árvore em `O(n log n)`, com `n ≤ 256` — ou seja,
efetivamente constante em relação ao tamanho do arquivo. A parte que
de fato escala com o tamanho do arquivo é a leitura/contagem de bytes
e a escrita dos bits codificados, ambas `O(tamanho do arquivo)`.

## 3. Por que não é preciso salvar a árvore no arquivo comprimido?

A construção da árvore (passo 2 acima) é **determinística**: dado o
mesmo vetor de frequências e o mesmo algoritmo, sempre se chega à
mesma árvore (os empates no heap são resolvidos sempre da mesma forma,
já que a ordem de inserção dos símbolos — de 0 a 255 — é fixa).

Por isso, basta salvar no cabeçalho do arquivo `.huff` a **tabela de
frequências** (que símbolos existem e quantas vezes cada um aparece).
Na descompressão, reconstruímos a árvore do zero a partir dela e
obtemos exatamente os mesmos códigos usados na compressão. Isso evita
serializar a estrutura da árvore em si, que ocuparia mais espaço.

## 4. Codificação e decodificação

- **Compressão:** para cada byte do arquivo original, escreve-se a
  sequência de bits do seu código (obtida por uma travessia da árvore
  guardada em uma tabela `codes[256]`). Os bits são acumulados em um
  buffer de 1 byte (`BitWriter`) e só são gravados no disco quando 8
  bits se completam.
- **Descompressão:** para reconstruir cada byte original, caminha-se
  pela árvore a partir da raiz, lendo um bit por vez (`BitReader`):
  bit 0 vai para o filho esquerdo, bit 1 para o direito. Ao chegar em
  uma folha, o byte armazenado nela é o byte original; repete-se o
  processo do zero (voltando à raiz) até reconstruir todos os bytes.

## 5. Casos extremos tratados explicitamente

- **Arquivo vazio:** `num_symbols = 0`. Nenhuma árvore é construída;
  o cabeçalho por si só já basta para saber que a saída deve ser um
  arquivo vazio.
- **Um único símbolo distinto:** a árvore de Huffman "degenera" em uma
  única folha, para a qual não existe um código natural (o percurso
  da raiz até ela mesma tem comprimento zero). O código é forçado
  para `"0"` (ver `huffman_generate_codes` em `huffman.c`). Além
  disso, como sabemos que só existe um valor de byte possível, a
  compressão nem chega a escrever bits — o cabeçalho (byte + contagem)
  já é suficiente para a descompressão reconstruir o arquivo inteiro.
- **Dados de alta entropia (ex.: binário aleatório):** quando todos os
  bytes têm frequência parecida, o Huffman não consegue economizar
  bits (na prática, os códigos ficam todos com ~8 bits), e o overhead
  fixo do cabeçalho (9 bytes por símbolo distinto) faz o arquivo
  comprimido ficar **maior** que o original. Isso não é um bug: é uma
  consequência direta do limite teórico de compressão sem perdas
  (entropia de Shannon) — dados sem redundância estatística não podem
  ser comprimidos.

## 6. Limitações conhecidas (e por que estão documentadas, não escondidas)

- O alfabeto é fixo em 256 símbolos (bytes), não usa modelagem de
  ordem superior (como pares de bytes ou contexto), então não chega
  perto de compressores gerais como o `gzip` (que combina LZ77 +
  Huffman). Isso é intencional: o objetivo do projeto é demonstrar a
  implementação do algoritmo de Huffman em si, com clareza, não
  competir com bibliotecas de compressão de produção.
- O cabeçalho usa 9 bytes por símbolo distinto (1 de valor + 8 de
  frequência) — simples e robusto para arquivos de qualquer tamanho,
  mas não otimizado para minimizar overhead em arquivos muito
  pequenos.
