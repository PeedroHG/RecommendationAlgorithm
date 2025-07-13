# 🎥 Movie Recommendation Algorithm 

<details>
  <summary><b>🧭 Sumário</b></summary>
  <ol>
    <li><a href="#introducao">📘 Introdução</a></li>
    <li><a href="#fundamentacao">📖 Abordagem e Fundamentação Teórica</a></li>
    <li><a href="#datasets">📈 Datasets</a></li>
    <li><a href="metodologia">🧪 Metodologia</a></li>
    <li><a href="#estrutura">🧬 Estrutura do Projeto</a></li>
    <li><a href="#modelagem-e-implementacao">💻 Modelagem e Implementação</a></li>
    <li><a href="#resultados">📊 Análises dos Resultados</a></li>
    <li><a href="#conclusao">✔️ Conclusão</a></li>
    <li><a href="arquivos">📁 Arquivos Adicionais</a></li>
    <li><a href="ambiente">🚀 Ambiente Compilação e Execução</a></li>
    <li><a href="#autores">👥 Autores</a></li>
    <li><a href="#referencias">📚 Referências</a></li>
  </ol>
</details>

<h2 id="introducao">📘 Introdução</h2>

<p align="justify">
Com o rápido avanço tecnológico e a popularização da internet, o volume de dados gerados diariamente cresceu exponencialmente. Essa vasta quantidade de informação, embora rica, apresenta um desafio significativo: como processá-la e transformá-la em algo útil e significativo para os usuários?

Essa sobrecarga de informação pode levar a uma sensação de desorientação. Em plataformas de streaming de vídeo, por exemplo, a imensa disponibilidade de títulos, apesar de ser uma vantagem, pode gerar insatisfação, pois o usuário se sente sobrecarregado com tantas opções de escolha. Nesse contexto, os sistemas de recomendação surgem como uma solução. Eles atuam como filtros inteligentes, avaliando as informações disponíveis sobre o histórico e os prováveis interesses dos usuários para, então, sugerir conteúdos. As recomendações geradas por esses sistemas não apenas reduzem a sobrecarga de escolha, mas também ajudam os usuários a descobrir o que é mais relevante e adequado aos seus gostos. Assim, transformam a abundância de dados em uma experiência personalizada e satisfatória.

Este projeto tem como objetivo desenvolver um protótipo de algoritmo de recomendação de filmes, baseado na análise dos perfis dos usuários e nas características dos itens disponíveis. A abordagem adotada busca identificar padrões de preferências e comportamentos para oferecer sugestões personalizadas, aprimorando a experiência do usuário e a relevância das recomendações.
</p>

<h2 id="fundamentacao">📖 Abordagem e Fundamentação Teórica</h2>
<p align="justify">
No contexto deste estudo, onde foi empregado o conjunto de dados <b>MovieLens 25M</b> caracterizado pelo elevado volume de registros e alta dimensionalidade, a escolha de um método eficiente para gerar recomendações se faz uma tarefa desafiadora,  pois é preciso conciliar escalabilidade, velocidade de processamento e qualidade preditiva diante da esparsidade e complexidade do dataset.

Algoritmos populares como k-NN tradicional ou similaridade de cosseno direta, falham em oferecer baixa escabilidade e baixa velocidade de processamento para um alto volume de dados. O primeiro falha pois, para encontrar os vizinhos mais próximos, calcula a distância ou similaridade de um ponto de consulta para todos os outros pontos no conjunto de dados. Isso leva a um alto custo computacional, que cresce quadraticamente (complexidade O(n²)) com o tamanho do dataset <a href="#ref1">[1]</a>, ou mais precisamente,  para a tarefa mais comum de uma única consulta de predição, a complexidade é descrita como O(n⋅d), frequentemente simplificada para O(n) quando a dimensionalidade é tratada como constante <a href="#ref2">[2]</a>. Para o <b>MovieLens 25M</b>, com mais de 162 mil usuários e 62 mil filmes, essa abordagem se torna inviável pois tanto o número de usuários quanto a dimensão de filmes a ser analisadas é alto. Adicionalmente, no segundo caso, calcular a similaridade para todos os pares de itens em um grande conjunto de dados é computacionalmente inviável <a href="#ref3">[3]</a>.
Além desses problemas,  fatores como alta dimensionalidade (conhecida como maldição da alta dimensionalidade), desempenho ineficiente para matrizes esparças e alto consumo de memória tornam essas abordagens pouco atraentes <a href="#ref4">[4]</a>-<a href="#ref6">[6]</a>.

Em contraste, a técnica de <b>Hashing Sensível à Localidade (LSH) </b> se destaca como uma abordagem altamente eficiente para algoritmos de recomendação, superando as limitações de métodos tradicionais, especialmente em contextos de grandes volumes de dados <a href="#ref7">[7]</a>,<a href="#ref8">[8]</a>. O principal objetivo da LSH  é agrupar itens de entrada semelhantes, associando-os a um mesmo hash com alta probabilidade, também conhecido como "buckets". Diferente das técnicas de hash convencionais, o LSH maximiza as colisões de hash em vez de minimizá-las <a href="#ref9">[9]</a>. Isso permite acelerar a busca por vizinhos próximos em conjuntos de dados de alta dimensionalidade, mitigando a "maldição da dimensionalidade" <a href="#ref10">[10]</a>.

Quando os dados são representados como vetores em um espaço de alta dimensão, a técnica de LSH mais comum utiliza hiperplanos aleatórios.O objetivo aqui é agrupar vetores que apontam em direções semelhantes, o que é medido pela <b>similaridade de cosseno</b>. O processo funciona da seguinte forma:

* **Criação de Hiperplanos**: Vários hiperplanos aleatórios são gerados no espaço vetorial. Cada hiperplano divide o espaço em duas metades.
* **Geração do Hash**: Para cada vetor de dados, um "hash" é gerado com base em sua posição em relação a esses hiperplanos. Para cada hiperplano, se o vetor estiver de um lado, ele recebe um bit '1', e se estiver do outro lado, recebe um bit '0'.
* **Código de Hash**: Ao combinar os bits de todos os hiperplanos, cada vetor recebe um código de hash binário (uma sequência de 0s e 1s). Vetores que estão próximos no espaço angular (alta similaridade de cosseno) têm uma alta probabilidade de cair do mesmo lado de muitos dos hiperplanos aleatórios, resultando em códigos de hash idênticos ou muito semelhantes.

No contexto de LSH, esse código de hash binário serve como a chave para o bucket. A criação desses buckets de candidatos é a razão fundamental pela qual o LSH consegue mitigar a "maldição da dimensionalidade". A pergunta crucial que surge dessa abordagem é: qual é o verdadeiro ganho de desempenho e quais são os custos associados a essa aproximação?

Em contraste com uma busca por força bruta (k-NN), cujo custo computacional cresce de forma linear ou quadrática com o volume de dados (O(nd) ou O(n²)) e se torna rapidamente proibitivo, o LSH direciona a busca apenas para um pequeno subconjunto de itens promissores. Ao consultar apenas os candidatos que colidem no mesmo bucket, o tempo de busca é drasticamente reduzido para uma complexidade sublinear, representando o seu principal ganho. Contudo, essa eficiência não é gratuita e seu custo mais significativo reside na própria aproximação. O LSH aceita a possibilidade de erros, que se manifestam como falsos negativos, onde um vizinho real é perdido por não colidir em nenhum bucket, e falsos positivos, onde itens não similares colidem por acaso, exigindo verificações de distância desnecessárias que consomem tempo. A gestão desses erros constitui um segundo custo, o de engenharia, pois exige um ajuste fino dos parâmetros do algoritmo — como o número de tabelas de hash (L) e de funções por tabela (k) — para encontrar o balanço ideal entre a precisão dos resultados e a velocidade da consulta. Por fim, há o custo inicial de pré-processamento e memória, um investimento de tempo e recursos computacionais necessário para construir e armazenar as tabelas de hash antes que qualquer busca possa ser realizada.

A proposta desenvolvida neste estudo âncora-se na ideia simplista do k-NN e as vantagens da abordagem LSH. Para evitar essa busca exaustiva, o sistema utiliza LSH como um poderoso filtro inteligente na primeira etapa. Cada usuário, representado por seu perfil de avaliações de filmes, é mapeado para buckets LSH, que funcionam como "vizinhanças" de gostos, agrupando usuários com perfis de avaliação parecidos. Assim, quando é necessário encontrar os vizinhos mais próximos de um usuário, o sistema não varre todo o banco de dados. Em vez disso, ele usa o LSH para filtrar rapidamente um conjunto reduzido de candidatos promissores — aqueles que caíram nos mesmos buckets que o usuário-alvo. Só então, dentro desse conjunto pré-filtrado e muito menor, é aplicado o cálculo de similaridade cosseno (k-NN) para selecionar os k vizinhos mais próximos e, finalmente, gerar as recomendações.

Dessa forma, o sistema combina o melhor dos dois mundos: a velocidade da busca aproximada do LSH para identificar um conjunto relevante de usuários e a precisão da métrica de similaridade do k-NN para fazer a escolha final dentro do bucket. Esta abordagem em duas etapas resolve diretamente a questão da escalabilidade, permitindo que recomendações personalizadas sejam geradas de maneira eficiente, mesmo em um ecossistema com um número massivo de usuários e itens.

Ao gerar as recomendações é necessário avaliar a qualidade das sugestões de filmes sugeridas pelo algoritmo. Em nossa abordagem de recomendação, cada filme candidato recebe um grau de preferência calculado a partir das avaliações de usuários semelhantes, de modo que os vizinhos com perfis mais próximos exercem maior influência. Para isso, combinamos cada avaliação com o grau de similaridade do usuário que a fez e, em seguida, normalizamos pelo total de similaridade acumulada, garantindo que opiniões mais alinhadas pesem mais no resultado final. Essa estimativa é então convertida para uma escala percentual, refletindo o nível de confiabilidade da recomendação. Por fim, apresentamos ao usuário apenas os títulos que ele ainda não viu, ordenados pelo valor percentual, de modo a destacar primeiro as opções com maior chance de agradar o usuário-alvo. </p>

<h2 id="datasets"> 📊 Datasets</h2>

O conjunto de dados utilizado no sistema de recomendação é o **MovieLens 25M**, disponível na plataforma Kaggle [`ml-25m`](https://www.kaggle.com/datasets/patriciabrezeanu/movielens-full-25-million-recommendation-data). Ele contém registros de classificação de filmes por estrelas, além de marcações com texto livre realizadas na plataforma oficial [MovieLens](http://movielens.org), um renomado serviço de recomendação de filmes.

O conjunto de dados possui as seguintes características:

-   **Total de avaliações:** 25.000.095 classificações
-   **Total de marcações (tags):** 1.093.360 tags aplicadas
-   **Quantidade de filmes:** 62.423 filmes incluídos
-   **Período de coleta:** 9 de janeiro de 1995 a 21 de novembro de 2019
-   **Usuários:** 162.541 usuários selecionados aleatoriamente

Para alimentar o sistema de recomendação, os dados utilizados foram especificamente os arquivos `movies.csv` e `ratings.csv`.

Em relação aos usuários e à estrutura dos dados:
-   Todos os usuários selecionados avaliaram pelo menos **20 filmes**.
-   Todos os arquivos do conjunto de dados são formatados em `.csv`, contendo uma única linha de cabeçalho.
-   Colunas que contêm vírgulas (`,`) são escapadas utilizando **aspas duplas** (`"`).
-   Os arquivos são codificados em **UTF-8**.

### Estrutura dos Arquivos Usados

#### `ratings.csv`
Este arquivo contém todas as avaliações dos usuários. Cada linha, após o cabeçalho, representa uma **avaliação de um filme por um usuário**, no seguinte formato:

```csv
userId,movieId,rating,timestamp
```

#### `movies.csv`
Este arquivo contém informações sobre os filmes presentes no conjunto de dados MovieLens 25M. Cada linha, após o cabeçalho, representa um filme e segue o seguinte formato:

```csv
movieId,title,genres
```

<h2 id="metodologia"> 🧪 Metodologia</h2>

A construção do algoritmo de recomendação ancorou-se nas técnicas k-NN e LSH discutidas nas seções anteriores. Para tal fim, foram desenvolvidas 3 etapas, enumeradas da seguinte maneira:

1. **Pré-processamento**: adotou-se um protocolo de seleção de usuários com pelo menos 50 avaliações distintas e de filmes avaliados por no mínimo 50 usuários, garantindo representatividade estatística e reduzindo ruídos provenientes de interações esparsas; em seguida, eliminaram-se registros duplicados e entradas inconsistentes — incluindo linhas com valores ausentes ou ratings fora do intervalo permitido — para assegurar a integridade dos dados.
2. **Construção do modelo**:
Após o pré-processamento, os dados filtrados são convertidos em uma matriz usuário-item, estrutura fundamental para operações de similaridade e recomendação. Cada linha representa um usuário e cada coluna um filme, com os valores correspondendo às avaliações atribuídas.
Em seguida, são calculadas as normas dos vetores de cada usuário, etapa essencial para o cálculo eficiente da similaridade cosseno.
O modelo LSH (Locality-Sensitive Hashing) é então construído:
Gera-se um conjunto de hiperplanos aleatórios para cada tabela LSH, projetando os vetores dos usuários em múltiplas tabelas hash, o que permite agrupar usuários similares em buckets comuns.
Essa estrutura possibilita a busca aproximada de vizinhos mais próximos (k-NN) de forma eficiente, mesmo em grandes volumes de dados, reduzindo drasticamente o custo computacional em relação à busca exaustiva.
3. **Geração das recomendações**:
Para cada usuário-alvo, o sistema utiliza o LSH para identificar rapidamente um conjunto de vizinhos mais próximos (usuários similares).
A partir desses vizinhos, calcula-se um score de recomendação para cada filme não avaliado pelo usuário, ponderando as avaliações dos vizinhos pela similaridade e aplicando filtros adicionais, como limiar mínimo de similaridade e comparação com a média do usuário.
Caso não haja recomendações principais, um mecanismo de fallback sugere filmes com alguma similaridade residual.
As recomendações finais são ordenadas pelo score percentual e apresentadas ao usuário, juntamente com o título do filme e o grau de recomendação, permitindo uma avaliação clara e interpretável dos resultados.

Para avaliar a qualidade da recomendação, o sistema utiliza o score de recomendação atribuído a cada filme para o usuário-alvo. Esse score é calculado com base na média ponderada das avaliações dos vizinhos mais similares, considerando apenas aqueles cuja similaridade ultrapassa um limiar definido. O score reflete o grau de confiança do sistema de que o usuário irá apreciar o filme sugerido. A fim de tornar essa avaliação mais intuitiva, o score é convertido para uma escala percentual de 0 a 100%.

<h2 id="estrutura"> 🧬 Estrutura do Projeto</h2>

```text
📁 projeto/
├── 📂 datasets/          # Arquivos de entrada
├── 📂 src/               # Códigos principais
├── 📂 outcome/           # Arquivos de saída
├── 📂 include/           # Arquivos de cabeçalho
├── README.md
```

<h2 id="modelagem-e-implementacao"> 💻 Modelagem e Implementação</h2>

A escolha do C++ para a implementação do sistema de recomendação de filmes foi motivada pela sua capacidade de oferecer controle de baixo nível e elevado desempenho computacional, características cruciais para processar eficazmente grandes volumes de dados. Esta robustez é complementada pela utilização estratégica de estruturas de dados otimizadas da Standard Template Library (STL) e pela paralelização intensiva via OpenMP, visando a maximização da eficiência e a escalabilidade do processamento, especialmente crítica para o manejo de datasets volumosos. A arquitetura adotada para o projeto é intrinsecamente modular, concebida para promover a clareza, a manutenibilidade e a extensibilidade. Esta organização lógica compreende um pipeline integral que se estende desde o processamento inicial e a curadoria dos dados até a fase final de geração de recomendações e a rigorosa avaliação do modelo. Para garantir uma execução sequencial e lógica, esses componentes foram articulados em fases distintas, a saber:

* ⚙️ **Fase 1:** <a href="#fase1-dados">Carregamento e Pré-processamento de Dados</a>
* 🏗️ **Fase 2:** <a href="#fase2-modelo">Construção de Componentes do Modelo Central</a>
* ⚡ **Fase 2.5:** <a href="#fase2_5-lsh">Construção do Modelo LSH</a>
* 💡 **Fase 3:** <a href="#fase3-recomendacoes">Geração de Recomendações LSH</a>
* 📊 **Módulo de Avaliação:** <a href="#modulo-avaliacao">Avaliação do Modelo</a>

### Definição de Tipos de Dados

Para promover a clareza, a legibilidade e a manutenção do código-fonte, o projeto faz uso extensivo de **alias de tipos (type aliases)**. Essas definições, centralizadas no [arquivo `types.hpp`](src/types.hpp), padronizam as estruturas de dados complexas e conferem maior expressividade aos nomes das variáveis e parâmetros, refletindo seu propósito dentro do algoritmo. O detalhamento de cada tipo será apresentado contextualmente ao longo da descrição das fases de implementação, no momento de sua primeira utilização relevante.

<h3 id="fase1-dados">⚙️ Fase 1: Carregamento e Pré-processamento de Dados</h3>

<p align="justify">
A fase inicial do sistema é dedicada à ingestão e à filtragem do dataset. O processo é projetado para transformar os dados brutos em um conjunto de informações estruturado, denso e de alta qualidade, otimizado para as etapas subsequentes de modelagem e recomendação. Para isso, adota-se um pipeline de I/O e transformação de dados de alta performance.
</p>

<ol>
  <li>
    <p align="justify">
      <strong>Estratégia de Leitura e Parsing de Alta Performance</strong>
    </p>
    <p align="justify">
      Para mitigar o gargalo de desempenho associado à leitura de arquivos massivos e garantir a máxima eficiência na extração dos dados, uma estratégia de I/O otimizada com <strong>memory-mapped files (mmap)</strong> é implementada na função <code>readRatingsCSV</code>.
    </p>
    <ul>
      <li><strong>Leitura com <code>mmap</code>:</strong> Em vez de ler o arquivo <code>ratings.csv</code> em blocos com `ifstream`, o sistema utiliza `mmap` para mapear o arquivo diretamente na memória virtual do processo. Isso elimina cópias de dados entre o kernel e o espaço do usuário, sendo uma das formas mais rápidas de ler arquivos grandes em sistemas POSIX. O arquivo se comporta como um grande array de bytes na memória, permitindo acesso e processamento direto.</li>
      <li><strong>Parsing Paralelo e Eficiente:</strong> O conteúdo mapeado é então dividido em "chunks" (pedaços) que são distribuídos entre múltiplas threads usando OpenMP (<code>#pragma omp parallel</code>). Cada thread analisa as linhas de seu chunk de forma independente, utilizando ferramentas modernas de C++ para máxima performance: <code>std::string_view</code> para evitar a criação de novas strings e <code>std::from_chars</code> para conversões numéricas rápidas e sem alocação de memória.</li>
      <li><strong>Armazenamento sem Concorrência:</strong> Para evitar condições de corrida (race conditions) durante a inserção paralela, cada thread armazena os dados que processou em um log de avaliações local (<code>thread_local_logs</code>). Ao final da fase paralela, esses logs locais são combinados sequencialmente no mapa principal, uma operação rápida que consolida os resultados.</li>
    </ul>
  </li>
  <li>
    <p align="justify">
      <strong>Critérios de Filtragem do Dataset</strong>
    </p>
    <p align="justify">
      O propósito da filtragem é aumentar a densidade e a confiabilidade do conjunto de dados, um passo fundamental para a eficácia de algoritmos de filtragem colaborativa. O sistema aplica um limiar mínimo de relevância (<code>MIN_RATINGS_PER_ENTITY</code>) em um processo paralelizado de múltiplas etapas.
    </p>
    <ul>
      <li><strong>Contagem Paralela e <i>Lock-Free</i>:</strong> A função <code>countEntityRatings</code> contabiliza o número de avaliações por usuário e por filme. Para otimizar essa contagem, ela é paralelizada de forma a evitar o uso de locks (que serializariam o acesso): cada thread escreve as contagens de filmes em seu próprio mapa local. Após a conclusão, esses mapas locais são agregados no mapa global.</li>
      <li><strong>Identificação e Aplicação do Filtro:</strong> A função <code>identifyValidEntities</code> armazena os IDs das entidades que atendem ao limiar em um <code>std::unordered_set</code>, que permite consultas de validação com complexidade de tempo média de O(1). Em seguida, <code>filterUserRatingsLog</code> aplica esse filtro, também de forma paralela, onde cada thread processa um subconjunto de usuários, mantendo apenas as avaliações de filmes válidos.</li>
    </ul>
  </li>
  <li>
    <p align="justify">
      <strong>Geração dos Artefatos de Saída com Paralelização OpenMP</strong>
    </p>
    <p align="justify">
      Ao final do processo, os dados refinados são serializados e persistidos em arquivos que servirão de base para as fases seguintes. A escrita desses arquivos é otimizada com paralelismo.
    </p>
    <ul>
      <li><strong>Dataset Filtrado:</strong> A função <code>writeFilteredRatingsToFile</code> escreve os dados refinados no arquivo <code>filtered_dataset.dat</code>. A geração das strings de saída é feita em paralelo: cada thread formata as linhas para um subconjunto de usuários e as armazena em um vetor de strings. Ao final, todas as linhas são concatenadas em uma única string gigante, que é escrita no arquivo com uma única chamada de I/O, minimizando o acesso ao disco.</li>
      <li><strong>Geração de Amostra para Exploração:</strong> A função <code>writeRandomUserIdsToExplore</code> seleciona uma amostra aleatória de usuários e salva seus IDs no arquivo <code>explore.dat</code>. Crucialmente, ela utiliza um gerador de números aleatórios com uma semente fixa (<code>std::mt19937 generator(42)</code>). Isso garante que a seleção de usuários seja sempre a mesma a cada execução do programa, o que é vital para a <strong>reprodutibilidade</strong> dos experimentos e análises.</li>
    </ul>
  </li>
  <li>
    <p align="justify">
      <strong>Análise Conceitual das Estruturas e Funções Chave</strong>
    </p>
    <p align="justify">
      A alta performance desta fase é resultado direto da escolha criteriosa de estruturas de dados e funções. Abaixo, detalhamos o porquê de cada escolha em comparação com suas alternativas.
    </p>
    <ul>
      <li>
        <strong><code>mmap</code> vs. <code>std::ifstream</code></strong>
        <ul>
          <li><strong>Conceito (<code>mmap</code>):</strong> Mapeia um arquivo diretamente no espaço de endereço virtual, tratando-o como um array na memória. Isso elimina a sobrecarga de copiar dados entre o buffer do kernel e o do aplicativo, tornando-o ideal para leituras de arquivos muito grandes.</li>
          <li><strong>Justificativa:</strong> Para o dataset de 25 milhões de avaliações, `mmap` é significativamente mais rápido do que a leitura tradicional com `std::ifstream`, pois minimiza as chamadas de sistema e as cópias de dados.</li>
        </ul>
      </li>
      <li>
        <strong><code>std::unordered_map</code> vs. <code>std::map</code></strong>
        <ul>
          <li><strong>Conceito (<code>unordered_map</code>):</strong> É uma tabela de hash que oferece acesso, inserção e remoção em tempo médio <strong>constante, O(1)</strong>.</li>
          <li><strong>Justificativa:</strong> Para agrupar e contar avaliações por ID, a velocidade de acesso é a prioridade. Como a ordem dos IDs não é importante para esta tarefa, a complexidade O(1) do `std::unordered_map` é superior à O(log n) do `std::map` (uma árvore binária balanceada).</li>
        </ul>
      </li>
      <li>
        <strong><code>std::unordered_set</code> vs. <code>std::vector</code></strong>
        <ul>
          <li><strong>Conceito (<code>unordered_set</code>):</strong> Uma tabela de hash que armazena elementos únicos. Sua principal força é verificar a existência de um elemento em tempo médio <strong>constante, O(1)</strong>.</li>
          <li><strong>Justificativa:</strong> Na etapa de filtragem, a operação mais comum é verificar se um ID de usuário ou filme é "válido". O `std::unordered_set` responde a essa pergunta em tempo O(1), sendo a escolha mais rápida. Uma busca em um `std::vector` seria linear (O(n)), o que seria proibitivamente lento.</li>
        </ul>
      </li>
      <li>
        <strong><code>std::string_view</code> vs. <code>std::string</code></strong>
        <ul>
          <li><strong>Conceito (<code>string_view</code>):</strong> É um objeto "não-proprietário" que aponta para uma sequência de caracteres existente sem criar cópias.</li>
          <li><strong>Justificativa:</strong> Ao processar milhões de linhas de um arquivo CSV, o uso de `std::string_view` para o parsing evita milhões de alocações de memória que ocorreriam ao criar substrings com `std::string`, resultando em um ganho de performance massivo.</li>
        </ul>
      </li>
        <li>
        <strong><code>std::from_chars</code> vs. <code>std::stoi/stof</code></strong>
        <ul>
          <li><strong>Conceito (<code>std::from_chars</code>):</strong> Uma função de conversão de baixo nível que não aloca memória e não lança exceções, tornando-a mais rápida e previsível.</li>
          <li><strong>Justificativa:</strong> É a ferramenta ideal para um parser de alta performance, pois converte os dados numéricos de forma eficiente e sem a sobrecarga do tratamento de exceções ou dependência de localidade, comuns em `stoi` ou `stof`.</li>
        </ul>
      </li>
    </ul>
  </li>
</ol>


<h3 id="fase2-modelo">🏗️ Fase 2: Construção de Componentes do Modelo Central</h3>

A Fase 2 do processo de implementação concentra-se na transformação dos dados de avaliações pré-processados em estruturas otimizadas, essenciais para os cálculos de similaridade subjacentes ao algoritmo de recomendação. Este conjunto de operações é implementado no módulo `recommender_engine.cpp/.hpp`.

Os principais processos realizados nesta fase são:

1.  **Mapeamento de Avaliações para Matriz Esparsa de Usuário-Item:**
    * Esta função é responsável por converter o formato inicial de log de avaliações, representado por `UserRatingsLog` (um `std::unordered_map` onde cada `UserID` é mapeado a um `std::vector` de pares `(MovieID, Rating)`), em uma `UserItemMatrix`. A `UserItemMatrix` é uma estrutura `std::unordered_map<int, std::unordered_map<int, float>>`, que, de forma qualitativa, age como uma representação esparsa da matriz de avaliações. Sua escolha é estratégica para **otimização de acesso**: ao aninhar `unordered_maps`, o sistema permite que, dado um `UserID` e um `MovieID`, a avaliação correspondente seja acessada diretamente com complexidade média de tempo constante ($O(1)$). Isso é fundamental para a eficiência em operações futuras, como o cálculo do produto escalar na similaridade de cosseno, onde a busca por avaliações específicas é frequente e a esparsidade dos dados evita a alocação desnecessária de memória para filmes não avaliados.

2.  **Pré-cálculo e Armazenamento das Normas de Usuários**
    * A `computeUserNorms` calcula a norma Euclidiana  para o vetor de avaliações de cada usuário presente na recém-construída `UserItemMatrix`. As normas são então armazenadas na estrutura `UserNormsMap`, que é um `std::unordered_map<int, float>`. O **foco principal na otimização** aqui reside no **pré-cálculo e armazenamento dessas magnitudes**. A similaridade de cosseno, métrica central para determinar a semelhança entre usuários, exige a divisão pelo produto das normas dos vetores comparados. Ao ter as normas já computadas e acessíveis em $O(1)$, o sistema evita a repetição de um cálculo de raiz quadrada e somas para cada par de usuários durante as milhões de comparações de similaridade que ocorrem nas fases de busca por vizinhos. Este processo é adicionalmente **paralelizado utilizando OpenMP**, o que distribui a carga computacional de cálculo de normas entre múltiplos núcleos de processamento, acelerando significativamente a preparação dos dados para a fase LSH em grandes datasets.

Ao final desta fase, o sistema possui a `UserItemMatrix` e a `UserNormsMap` totalmente construídas, representando os perfis de usuários em um formato otimizado para as operações de hashing sensível à localidade e busca por vizinhos.

<h3 id="fase2_5-lsh">⚡ Fase 2.5: Construção do Modelo LSH</h3>

A construção de um modelo de Locality-Sensitive Hashing (LSH) mapeia dados de alta dimensão para agrupar itens semelhantes em "buckets", preservando suas relações geométricas. No caso da similaridade de cosseno, a LSH utiliza hiperplanos aleatórios para converter vetores de dados em assinaturas binárias. Dessa forma, pontos com orientação similar no espaço original acabam no mesmo bucket, o que otimiza a busca por vizinhos próximos.

O processo consiste em quatro etapas essenciais e interligadas:

1.  **Vetorização dos Dados:** Converter cada item do dataset em um vetor numérico de alta dimensionalidade.
2.  **Indexação com Funções Hash:** Gerar uma assinatura hash para cada vetor usando um conjunto de funções LSH (neste caso, hiperplanos aleatórios). Essa assinatura determina o bucket onde o item será armazenado.
3.  **Amplificação com Múltiplas Tabelas:** Utilizar várias tabelas de hash independentes para aumentar a probabilidade de que itens verdadeiramente similares colidam em ao menos uma delas, reduzindo a chance de falsos negativos.
4.  **Recuperação e Refinamento:** Para uma consulta, primeiro coletar um conjunto de candidatos dos buckets correspondentes nas tabelas e, em seguida, aplicar a métrica de similaridade exata (e mais custosa) apenas a esse grupo reduzido para encontrar os vizinhos mais próximos.

**Vetorização dos Dados:**
* A primeira etapa para a construção da LSH reside na vetorização e normalização do espaço de características. Para tal fim, é necessário inicialmente mapear a dimensão do dataset. A dimensão é baseada no número de filmes únicos. Essa escolha se deve ao fato de o número de filmes ser menor que o número de usuários, reduzindo dessa forma a dimensionalidade do problema. Os `movieIds` originais, que são esparsos e não sequenciais, são mapeados para um vetor de índices densos. Essa etapa é crucial para criar uma representação vetorial consistente para cada usuário.

    O funcionamento dessa parte consiste em percorrer todos os usuários e todos os filmes avaliados, coletando todos os IDs únicos de filmes presentes na matriz usuário-item. Cada filme recebe um índice sequencial (0, 1, 2, ...), criando um mapeamento de ID real para índice denso. Posteriormente, a variável `dimensionality` armazena o número total de filmes únicos, que será a dimensão dos vetores e dos hiperplanos usados no LSH. O código também faz uma checagem de segurança: se a dimensionalidade for zero, mas a matriz não estiver vazia, emite um erro.

    A escolha da estrutura <code>std::set</code> reside na natureza intrínseca da estrutura que funciona como um container que armazena elementos sem duplicatas e de forma ordenada, permitindo inserção e busca eficientes; assim, ao percorrer todos os filmes avaliados por todos os usuários e inseri-los no set, o código assegura que cada filme será contado apenas uma vez, facilitando o cálculo da dimensionalidade do espaço de itens e o mapeamento para índices densos, etapas essenciais para a construção eficiente do modelo LSH.

**Indexação com Funções Hash**
* A segunda etapa, consiste na criação das funções hash. Neste projeto, a família de funções de hash para a similaridade de cosseno é baseada em projeções aleatórias (hiperplanos). Para uma arquitetura LSH com $L$ tabelas de hash e $K$ funções de hash por tabela, são gerados $L\cdot K$ vetores de hiperplanos, {$h_1,h_2,...,h_{L*K}$, onde cada $h\in R^D$. Os componentes de cada vetor $h$ são amostrados independentemente de uma distribuição estável, tipicamente a Normal Padrão, $N(0,1)$, para garantir isotropia (não há direção privilegiada), partições balanceadas (50% de chance em cada lado) e invariância rotacional.

    O conjunto de hiperplanos aleatórios para a tabela LSH de índices $i$ é construído pela função `generateSingleHyperplaneSet`.

    Cada hiperplano é um vetor de números aleatórios com dimensão igual ao número de filmes únicos, e o conjunto de hiperplanos define como os usuários serão "hashados" (projetados) no espaço LSH. A diversidade dos hiperplanos determinam a capacidade do LSH de agrupar usuários similares nos mesmos buckets, tornando a busca por vizinhos eficientes e relevantes. Sem essa geração de hiperplanos aleatórios, o LSH não funcionaria corretamente, pois não haveria uma maneira eficiente de dividir o espaço de usuários em regiões de similaridade.

    ```cpp
    HyperplaneSet generateSingleHyperplaneSet(int num_hyperplanes, int dimensionality, std::mt19937& rng) {
        HyperplaneSet hyperplane_set;
        hyperplane_set.reserve(num_hyperplanes);
        std::normal_distribution`<float>` distribution(0.0, 1.0); // Distribuição normal padrão

        for (int i = 0; i < num_hyperplanes; ++i) {
            Hyperplane plane(dimensionality);
            for (int j = 0; j < dimensionality; ++j) {
                plane[j] = distribution(rng);
            }
            hyperplane_set.push_back(plane);
        }
        return hyperplane_set;
    }
    ```


**Amplificação com Múltiplas Tabelas:**

* A terceira etapa, reside na implementação do cálculo do hash LSH para um usuário, usando o conjunto de hiperplanos. O objetivo é transformar o vetor de avaliações do usuário em um valor binário (hash) que representa em que lado de cada hiperplano o usuário está, agrupando usuários similares em buckets. O processo de hashing consiste em

* Para cada hiperplano do conjunto:
  * Calcula o produto escalar entre o vetor de avaliações do usuário e o vetor do hiperplano.
  * Isso significa multiplicar cada nota dada pelo usuário pelo peso correspondente do hiperplano (associado ao filme), somando tudo.
  * Se o resultado do produto escalar for maior ou igual a zero, o bit correspondente do hash é ativado (colocado como 1).
  * Caso contrário, o bit permanece 0.
  * Cada hiperplano define um "lado" do espaço: o hash indica de que lado do hiperplano o usuário está.

    ```cpp
    LSHHashValue computeLSHHash(const std::unordered_map<int, float>& user_ratings,
                                const HyperplaneSet& hyperplane_set,
                                const MovieIdToDenseIdxMap& movie_to_idx) {
        LSHHashValue hash = 0;
        if (hyperplane_set.empty()) return hash; // Nenhum hiperplano, hash 0

        // Verifica se NUM_HYPERPLANES_PER_TABLE (tamanho de hyperplane_set) excede os bits de LSHHashValue
        if (hyperplane_set.size() > sizeof(LSHHashValue) * 8) {
            std::cerr << "Error: Number of hyperplanes exceeds bits in LSHHashValue type!" << std::endl;
            // Lida com o erro, talvez truncando ou usando um tipo de hash maior se necessário.
            // Por agora, vamos prosseguir, mas isso é uma verificação importante.
        }

        for (size_t i = 0; i < hyperplane_set.size(); ++i) {
            const auto& plane = hyperplane_set[i];
            float dot_product = 0.0f;
            for (const auto& rating_entry : user_ratings) {
                int movie_id = rating_entry.first;
                float rating = rating_entry.second;
                auto it_idx = movie_to_idx.find(movie_id);
                if (it_idx != movie_to_idx.end()) {
                    int dense_idx = it_idx->second;
                    if (dense_idx < static_cast`<int>`(plane.size())) {
                        dot_product += rating * plane[dense_idx];
                    }
                }
            }
            if (dot_product >= 0) {
                hash |= (1ULL << i);
            }
        }
        return hash;
    ```

    Ao final, o hash é um número inteiro, onde cada bit representa a posição do usuário em relação a um hiperplano. Usuários com perfis parecidos tendem a ter hashes parecidos, caindo nos mesmos "baldes" (buckets) do LSH.

**Recuperação e Refinamento**

*    A quarta etapa consiste finalmente na construção das tabelas hash utilizando a função <code>buildLSHTables</code>. O objetivo principal dessa função é organizar os usuários em grupos (buckets) de acordo com a similaridade de seus perfis, de forma eficiente e escalável.

     O processo começa convertendo a matriz de avaliações dos usuários em um vetor, facilitando o processamento paralelo. Para cada conjunto de hiperplanos (cada tabela LSH), a função percorre todos os usuários e calcula, para cada um, um hash LSH. Esse hash é obtido projetando o vetor de avaliações do usuário nos hiperplanos e verificando, para cada hiperplano, de que lado o usuário está. O resultado é um valor binário que serve como chave para o bucket onde o usuário será inserido.

     A construção das tabelas é feita de forma paralela: cada thread calcula os hashes de um subconjunto de usuários e armazena temporariamente os resultados. Depois, em uma etapa sequencial, todos os usuários são inseridos nos buckets correspondentes de cada tabela LSH. Isso garante eficiência mesmo em bases de dados grandes, aproveitando o poder de processamento de múltiplos núcleos.

     O resultado final é um conjunto de tabelas onde cada bucket contém os IDs dos usuários que compartilham o mesmo hash LSH para aquele conjunto de hiperplanos. Assim, quando o sistema precisa encontrar usuários similares a um alvo, ele pode buscar apenas nos buckets correspondentes, reduzindo drasticamente o número de comparações necessárias.

<h3 id="fase3-recomendacoes">💡 Fase 3: Geração de Recomendações LSH</h3>

<p align="justify">
  Esta é a fase mais importante do sistema, onde o modelo Locality-Sensitive Hashing (LSH), construído na etapa anterior, é efetivamente utilizado para gerar recomendações de filmes. O processo é desenhado para ser escalável e performático, processando um conjunto de usuários-alvo em paralelo para gerar um arquivo de saída com as Top-N recomendações para cada um.
</p>

<ol>
  <li>
    <p align="justify"><strong>Carregamento dos Usuários-Alvo e Preparação</strong></p>
    <ul>
      <li><p align="justify">O processo inicia com a leitura do arquivo <code>explore.dat</code> através da função <code>loadExploreUserIds</code>. Este arquivo contém a lista de IDs de usuários para os quais as recomendações devem ser geradas.</p></li>
      <li><p align="justify">Em seguida, o arquivo de saída <code>output.dat</code> é preparado pela função <code>prepareRecommendationsOutput</code>, que escreve um cabeçalho informativo contendo os hiperparâmetros da execução (K, N, L, etc.), garantindo a rastreabilidade dos resultados.</p></li>
    </ul>
  </li>

  <li>
    <p align="justify"><strong>Processamento Paralelizado de Recomendações</strong></p>
    <ul>
      <li><p align="justify">O núcleo desta fase é a função <code>generateRecommendationsForUsers</code>, que orquestra a geração de recomendações para múltiplos usuários de forma concorrente.</p></li>
      <li><p align="justify"><strong>Paralelismo com OpenMP:</strong> A iteração sobre a lista de usuários-alvo é paralelizada utilizando a diretiva <code>#pragma omp parallel for</code>. Essa abordagem é fundamental para a performance, pois divide o trabalho entre os núcleos de processamento disponíveis na CPU. Em vez de processar um usuário de cada vez, o sistema processa vários simultaneamente, transformando um problema de complexidade O(N * C) (onde N é o número de usuários e C o custo por usuário) em aproximadamente O((N / P) * C) (onde P é o número de núcleos), garantindo alta escalabilidade.</p></li>
      <li><p align="justify"><strong>Coleta de Resultados Paralelos:</strong> Os resultados de cada thread (uma string formatada por usuário) são armazenados em um <code>std::vector&lt;std::string&gt;</code> pré-alocado com o tamanho exato da lista de usuários. Esta é uma decisão de design crucial para o paralelismo: ao fazer com que cada thread escreva em uma posição única e pré-determinada do vetor (<code>user_outputs[idx]</code>), elimina-se a necessidade de sincronização (locks) para proteger o acesso ao contêiner, um padrão conhecido como "embarrassingly parallel". A alternativa, usar <code>push_back</code> em um vetor compartilhado, exigiria um <code>#pragma omp critical</code>, o que serializaria as inserções e criaria um gargalo de performance.</p></li>
    </ul>
  </li>

  <li>
    <p align="justify"><strong>Pipeline de Recomendação por Usuário</strong></p>
    <p align="justify">Para cada usuário, a função <code>processUserRecommendations</code> executa um pipeline bem definido:</p>
    <ol type="a">
      <li>
        <p align="justify"><strong>Busca por Vizinhos Aproximados (ANN):</strong> A função <code>findApproximateKNearestNeighborsLSH</code> é invocada. Ela usa as tabelas LSH para identificar um conjunto de usuários candidatos e os armazena em um <code>std::set&lt;int&gt;</code>.<br>
          <strong>Justificativa da Estrutura:</strong> O <code>std::set</code> foi escolhido por garantir automaticamente a unicidade dos candidatos, uma vez que um mesmo usuário pode ser encontrado em múltiplos "buckets" de LSH. A alternativa, um <code>std::vector</code>, exigiria uma etapa posterior de ordenação e remoção de duplicatas (<code>sort</code> + <code>unique</code>), que seria mais verbosa e computacionalmente cara. Embora um <code>std::unordered_set</code> (tabela de hash, O(1) de inserção) seja teoricamente mais rápido que o <code>std::set</code> (árvore, O(log n) de inserção), o número de candidatos por usuário é relativamente pequeno, tornando a diferença de performance negligenciável e a simplicidade do <code>std::set</code> uma escolha válida.</p>
      </li>
      <li>
        <p align="justify"><strong>Cálculo das Recomendações:</strong> Com a lista de vizinhos, a função <code>generateRecommendationsLSH</code> calcula os scores para os filmes candidatos. A pontuação é uma média ponderada das notas dadas pelos vizinhos, onde o peso é a similaridade do vizinho. O trecho-chave da lógica de agregação é:</p>
        <pre><code class="language-cpp">
// Em generateRecommendationsLSH.cpp
// Para cada vizinho e seus filmes avaliados...
if (target_user_seen_movies.count(movie_id)) {
  continue; // Ignora filme já visto
}
// Acumula a nota ponderada pela similaridade
movie_weighted_score_sum_local[thread_id][movie_id] += rating * similarity_score;
// Acumula a soma das similaridades para a normalização
movie_similarity_sum_local[thread_id][movie_id] += similarity_score;
</code></pre>
      </li>
      <li><p align="justify"><strong>Filtragem e Ordenação:</strong> As recomendações podem passar por um filtro que remove filmes com score previsto inferior à média de avaliação do próprio usuário-alvo. Por fim, a lista é ordenada de forma decrescente pela pontuação para extrair as Top-N.</p></li>
    </ol>
  </li>

  <li>
    <p align="justify"><strong>Formatação e Persistência dos Resultados</strong></p>
    <ul>
      <li>
        <p align="justify">Os resultados de cada usuário são formatados em uma string legível usando <code>std::ostringstream</code>.Uma <code>std::ostringstream</code> é uma stream que opera sobre um buffer de string em memória. Ela é mais eficiente para construir strings complexas de forma incremental do que concatenações repetidas com <code>std::string::operator+=</code>. O motivo é que concatenações sucessivas podem forçar múltiplas realocações de memória à medida que a string cresce. A <code>ostringstream</code> gerencia seu buffer interno de forma mais inteligente, muitas vezes dobrando seu tamanho quando o espaço acaba, o que minimiza o número de operações de alocação e cópia, resultando em melhor performance.</p>
      </li>
      <li><p align="justify">O vetor com as strings de todos os usuários é então percorrido, e seu conteúdo é escrito no arquivo <code>output.dat</code>, finalizando o processo.</p></li>
    </ul>
  </li>
</ol>


<h3 id="modulo-avaliacao">✅ Avaliação do Modelo</h3>

<p align="justify">A avaliação da qualidade das recomendações geradas pelo sistema é um processo multifacetado que vai além de métricas de precisão tradicionais, como o RMSE. Em vez disso, o foco está em como o sistema quantifica a relevância de cada sugestão e a apresenta de forma transparente e interpretável para o usuário. Essa avaliação ocorre em duas etapas principais, refletidas nas funções <code>generateRecommendationsLSH</code> e <code>processUserRecommendations</code>.</p>

O núcleo da avaliação reside no cálculo de uma **pontuação de recomendação prevista** para cada filme. Este score é uma média ponderada das avaliações dos vizinhos mais próximos (usuários com gostos similares), onde o peso de cada avaliação é a similaridade de cosseno entre o vizinho e o usuário-alvo. A fórmula matemática para a pontuação prevista ($P_{u,i}$) para um usuário $u$ e um item (filme) $i$ é:

$$P_{u,i} = \frac{\sum_{v \in N} sim(u,v) \cdot r_{v,i}}{\sum_{v \in N} |sim(u,v)|}$$

Onde cada componente da fórmula significa:

* **$N$**: É o conjunto de vizinhos do usuário $u$ que também avaliaram o item $i$.
* **$sim(u, v)$**: É a similaridade de cosseno (ou outra métrica) entre o usuário-alvo $u$ e um vizinho $v$.
* **$r_{v,i}$**: É a nota (rating) que o vizinho $v$ deu ao item $i$.

Em essência, a opinião de vizinhos mais parecidos tem um peso maior, tornando a recomendação mais personalizada e confiável.
<p align="justify">Para refinar ainda mais a qualidade, o sistema pode aplicar um filtro opcional que considera a média de avaliação do próprio usuário-alvo. Se ativado, apenas os filmes com uma pontuação prevista superior a essa média são recomendados, focando em sugestões que têm maior probabilidade de superar as expectativas do usuário.</p>

<p align="justify">Finalmente, a apresentação dos resultados é, por si só, uma forma de avaliação. O sistema exibe a <strong>similaridade média dos vizinhos</strong>, que funciona como um indicador de confiança: quanto maior a similaridade, mais confiável é a base da recomendação. Além disso, a pontuação prevista é convertida para um percentual de relevância (de 0 a 100%) e as recomendações são ranqueadas em ordem decrescente. Isso permite que o usuário compreenda não apenas quais filmes são sugeridos, mas também o quão forte é cada recomendação e em que base de similaridade ela se apoia.</p>

<h2 id="resultados">📊 Análise de Resultados</h2>

Após o pré-processamento, com as restrições impostas, foram identificados 102.492 usuários e 13.176 filmes válidos, representando uma redução de aproximadamente 37% no número de usuários e 79% no número de filmes em relação ao conjunto original, que continha 162.541 usuários e 62.423 filmes

Na tentativa de validar o modelo proposto e analisar a melhor configuração dos parâmetros que conciliem desempenho e performance, foram examinados as combinações relacionadas ao número de tabelas hash e o número de hiperplanos. Ambos parâmetros influenciam diretamente na qualidade da recomendação e na performance. Atrelado a essas duas características avaliou-se para um usuário -  usuário 3 (escolha aleatória) - , o valor da similaridade média deste usuário e com seus vizinhos. A adoção dessa abordagem está inserida no contexto de que: a qualidade das recomendações dos filmes basea-se na multiplicação da similaridade e a nota dada ao filme pelo vizinho. Assim, quanto maior a similaridade, maior a tendência de boas indicações para o usuário-alvo.
Todas as modificações atreladas aos parâmetros ocorreram no arquivo config.hpp. Foram utilizados os seguintes valores:
<pre><code>// Recommendation & Filtering Parameters
const int MIN_RATINGS_PER_ENTITY = 50; 
const int K_NEIGHBORS = 10;
const size_t NUM_RANDOM_USERS_TO_EXPLORE = 50;
const int TOP_N_RECOMMENDATIONS = 5;

// cd Parameters lsh
const int NUM_LSH_TABLES = x ;
const int NUM_HYPERPLANES_PER_TABLE = y;    
</code></pre>


1. **Variação do número de tabelas hash e número de hiperplanos constante**

Para o arranjo proposto foram executados testes sucessivos em que o número de hiperplanos foram mantidos constante <code> NUM_HYPERPLANES_PER_TABLE = 16 </code>, e o número de tabelas hash variando de 1 até 16. A escolha para o número de hiperplanos está atrelado ao fato de possuir o conjunto de dados com mais de 100 mil usuários, que em binário, são necessários 16 bits. Os dados obtidos os de tempo de execução e valor da similaridade média encontra-se no gráfico abaixo:

<p align="center">
  <img src="https://hackmd.io/_uploads/SyX1erg8eg.png" alt="Diagrama LSH" width="500"/>
  <br>
  <em>Figura 1: Variação no número de tabelas hash e número de hiperplanos constante. As barras em azul estão atreladas ao tempo de execução e a linha em vermelha a similaridade média. O aumento no número de tabelas hash aumenta o tempo de execução, mas existe um limiar para o incremento da similaridade que se mantém em um valor constante de 0.47 após o valor de 7.</em>
</p>

Uma interpretação do gráfico indica que existe um limiar para o valor da similaridade (0.47) a medida que o número de tabelas hash aumenta. E como esperado, o aumento no número de tabelas hash provoca um incremento no tempo de execução. Devido a natureza do problema, em que perfomance, isto é, menor tempo de execução é uma vantagem, a definição mais consistente do número de tabelas hash é de 7. Uma vez que apresenta um balanço de tempo e valor de similaridade.Quanto aos possíveis picos de tempo, uma explicação plausível para as flutuações observadas no sistema reside na variabilidade da alocação de recursos computacionais durante o processamento paralelo. Interferências como carga concorrente do sistema operacional, balanceamento imperfeito entre threads ou até mesmo variações na distribuição dos usuários entre os buckets podem introduzir essas instabilidades pontuais no tempo de execução.


2. **Variação do número de tabelas hash e hiperplanos**

A abordagem da variação do número de hiperplanos e número de tabelas hash consistiu em variar ambos os parâmetros de 1 a 16. Os resultados obtidos encontram-se no gráfico abaixo:


<p align="center">
  <img src="https://hackmd.io/_uploads/HkkGxBlLxl.png" alt="Diagrama LSH" width="500"/>
  <br>
  <em>Figura 2: Variação no número de tabelas hash e número de hiperplanos. As barras em azul estão atreladas ao tempo de exeução e a linha em vermelha a similaridade média. O aumento no número de tabelas hash e hiperplanos aumenta o tempo de execução, e provoca uma redução do valor da similaridade que cai de aproximadamente 0.60 para 0.47. Verifica-se que o valor de 7 para ambos os parâmetros mantém performace e qualidade de recomendação.</em>
</p>

Uma análise do gráfico permite concluir que o aumento de ambos os parâmetros provoca um aumento no tempo de execução, e uma redução no valor de similaridade. Características nos quais não são desejadas. A redução se faz jus pois a medida que se aumenta a quantidade de hiperplanos gera-se buckets cada vez mais específicos reduzindo assim a probabilidade de colisões e consequentemente de usuários dentro de um bucket. Dessa forma, partindo dos resultados do gráfico infere-se que o valor de  7 tabelas hash e 7 hiperplanos mantém a qualidade e performance do algoritmo. 


3. **Variação do números de hiperplanos e número de tabelas hash constante**

Basedo no resultado anterior em que se variou o número de hiperplanos de e manteve-se o número de tabelas hash, no qual definiu-se o valor de 7 para ambos esses parâmetros. Manteve-se o número de tabelas hash em 7 e variou-se o número de hiperplanos. O gráfico abaixo apresenta os resultados obtidos.


<p align="center">
  <img src="https://hackmd.io/_uploads/rJIXxSx8el.png" alt="Diagrama LSH" width="500"/>
  <br>
  <em>Figura 3: Variação no número de hiperplanos e  número de tabelas hash constante. As barras em azul estão atreladas ao tempo de exeução e a linha em vermelha a similaridade média. O aumento no número de tabelas hash aumenta o tempo de execução, e provoca uma redução no valor da similaridade que cai de aproximadamente 0.60 para 0.27. Verifica-se que o valor de 7 mantém performance e qualidade de recomendação.</em>
</p>

A investigação do gráfico mostrou que o aumento do número de hiperplanos provoca uma redução da similaridade e um aumento no tempo de execução. Este resultado já foi observado em análises anteriores. Observando os valores, novamente para 7 hiperplanos nota-se uma consistência para a similaridade e tempo de execução. No entanto, nota-se que para o intervalo de 5-7 um valor constante para similaridade e tempo de execução. Assim, a escolha para o número de hiplerplanos pode estar dentro desse intervalo, uma vez que não indicou provocar alterações substâncias nos resultados. 

Analisando os três combinações propostas, verificamos que o número mínimo de tabelas hash é 7 e o número de hiperplanos é 5. Esses valores equilibram performance e qualidade de recomendação.

<h3> Teste de recomendações </h3>

Afim de verificar a qualidade das recomendações geradas pelo algoritmo foram testados dois casos:
1. **Recomendações em um mesmo bucket**

    Para verificar a consistência da geração de recomendações em uma mesma execução, o arquivo explore.dat foi manipulado para incluir um usuário duplicado. Essa abordagem permite analisar se, ao processar o mesmo usuário mais de uma vez, o sistema retorna recomendações idênticas, evidenciando que o algoritmo de LSH está determinístico e que usuários que caem no mesmo bucket recebem resultados consistentes. Caso haja divergência nas recomendações para o mesmo usuário, isso pode indicar algum fator aleatório não controlado ou instabilidade na construção dos buckets LSH.
    Para o teste, escolheu-se o usuário 1, cujos resultados para a execução estão disponíveis abaixo:
    ````
    User ID: 1 | Similaridade Media: 0.23
      Recommended Movies (MovieID: Score | Title):
      - 750: 95.0% | Dr. Strangelove or: How I Learned to Stop Worrying and Love the Bomb (1964),Comedy|War
      - 3083: 94.1% | All About My Mother (Todo sobre mi madre) (1999),Drama
      - 2019: 90.2% | Seven Samurai (Shichinin no samurai) (1954),Action|Adventure|Drama
      - 50: 87.8% | Usual Suspects, The (1995)
      - 3910: 86.4% | Dancer in the Dark (2000),Drama|Musical

    User ID: 1 | Similaridade Media: 0.23
     Recommended Movies (MovieID: Score | Title):
      - 750: 95.0% | Dr. Strangelove or: How I Learned to Stop Worrying and Love the Bomb (1964),Comedy|War
      - 3083: 94.1% | All About My Mother (Todo sobre mi madre) (1999),Drama
      - 2019: 90.2% | Seven Samurai (Shichinin no samurai) (1954),Action|Adventure|Drama
      - 50: 87.8% | Usual Suspects, The (1995)
      - 3910: 86.4% | Dancer in the Dark (2000),Drama|Musical
     ````
     Observa-se que, para o usuário 1, as recomendações geradas nas duas ocorrências são absolutamente idênticas, tanto na lista de filmes sugeridos quanto nos respectivos scores e títulos. Além disso, a similaridade média dos vizinhos encontrados permanece exatamente igual nas duas execuções (0.23). Isso evidencia que o sistema está funcionando de forma determinística: ao processar o mesmo usuário, mesmo que duplicado no arquivo de entrada, o algoritmo LSH+KNN retorna resultados consistentes e reprodutíveis.

    Esse comportamento confirma que a construção dos buckets LSH, a busca de vizinhos e a geração das recomendações não dependem de fatores aleatórios não controlados, e que o uso de uma semente fixa para o gerador de números aleatórios está garantindo a estabilidade do sistema. Portanto, usuários que caem no mesmo bucket (ou, neste caso, o mesmo usuário processado duas vezes) recebem sempre as mesmas recomendações, o que é desejável para análises comparativas e para garantir confiabilidade nos resultados do sistema de recomendação.
    
    Um outro fator observado, é que a indicação de filmes que reflete o perfil do usuário mostra-se consistente. Através do arquivo auxiliar `user_most_frequent_genres.dat` gerado em Python, é possível mapear a quantidade de filmes assistidos por gênero pelo usuário-alvo.
    ````
    User 1:
      Drama: 53
      Comedy: 23
      Romance: 18
      Adventure: 11
      Crime: 8
      Thriller: 5
      War: 5
      Musical: 5
      Sci-Fi: 5
      Fantasy: 5
      Mystery: 4
      Action: 4
      Children: 3
      Animation: 2
      Film-Noir: 1
      Western: 1
      Documentary: 1
      Horror: 1
    ````
     
     Quando se compara a lista de filmes recomendados para o usuário 1 com o seu perfil de gêneros mais assistidos, observa-se uma forte coerência entre as sugestões do sistema e os interesses reais do usuário. Por exemplo, entre os cinco filmes recomendados, predominam os gêneros Drama, Comedy, War, Musical e Adventure, que estão entre os mais frequentes no histórico do usuário. Filmes como "Dr. Strangelove" (Comedy|War), "All About My Mother" (Drama), "Seven Samurai" (Action|Adventure|Drama), "Usual Suspects" (Crime|Thriller) e "Dancer in the Dark" (Drama|Musical) refletem diretamente essa preferência.

    Essa correspondência reforça a qualidade do algoritmo de recomendação, mostrando que, além de ser determinístico e estável, o sistema é capaz de capturar nuances do perfil do usuário e sugerir títulos alinhados com seus gostos predominantes. 
    
2. **Recomendação para um usuário específico**

   Para exemplificar o funcionamento do sistema de recomendação diante de perfis com preferência marcante por um gênero específico, foi selecionado o usuário 2177, que apresenta alto consumo de filmes de drama segundo o arquivo user_most_frequent_genres.dat. A seguir, são apresentadas as recomendações geradas para esse usuário, permitindo observar como o algoritmo responde a padrões de consumo bem definidos e prioriza títulos alinhados às preferências individuais.
    
    ````
    User 2177:
      Drama: 2116
      Comedy: 1692
      Romance: 943
      Thriller: 794
      Action: 655
      Adventure: 532
      Crime: 497
      Sci-Fi: 300
      Horror: 273
      Children: 269
      Mystery: 269
      Fantasy: 259
      Musical: 256
      War: 205
      Western: 147
      Animation: 103
      Film-Noir: 68
      Documentary: 45
      IMAX: 19
    ````
    O resultado da recomendação
    ````
    User ID: 2177 | Similaridade Media: 0.63
      Recommended Movies (MovieID: Score | Title):
      - 177593: 100.0% | Three Billboards Outside Ebbing, Missouri (2017)
      - 116136: 100.0% | Olive Kitteridge (2014),Drama
      - 26395: 100.0% | Rutles: All You Need Is Cash, The (1978)
      - 47721: 95.7% | Red Balloon, The (Ballon rouge, Le) (1956)
      - 74508: 95.0% | Persuasion (2007),Drama|Romance
    ````
    
    É possível perceber que o sistema de recomendação reconhece o padrão de consumo do usuário 2177, que possui uma forte preferência pelo gênero "Drama" (com 2116 filmes assistidos desse tipo). Entre os filmes recomendados, destacam-se títulos como "Olive Kitteridge (2014)" e "Persuasion (2007)", ambos classificados como Drama, além de outros filmes que frequentemente combinam Drama com Romance ou outros gêneros apreciados pelo usuário. A similaridade média elevada (0.63) indica que as recomendações são altamente relevantes para o perfil do usuário, evidenciando que o algoritmo LSH+KNN é capaz de identificar e priorizar os gêneros mais consumidos, proporcionando sugestões alinhadas às preferências individuais.
3. **Recomendação para um usuário diverso**
    
    Para verificar a performance do algoritmo de recomendação em cenários onde o usuário possui um perfil muito diverso — ou seja, consome filmes de vários gêneros com frequência relativamente equilibrada — é importante analisar se o sistema consegue identificar vizinhos relevantes e sugerir títulos que realmente reflitam essa diversidade. Usuários com gostos amplos representam um desafio para sistemas baseados em similaridade, pois podem ter menos sobreposição direta com outros perfis, tornando a busca por vizinhos próximos mais difícil.
    Tomando um exemplo, o usuário 2
    ````
    User 3:
      Action: 334
      Thriller: 239
      Drama: 232
      Sci-Fi: 224
      Adventure: 198
      Comedy: 176
      Crime: 132
      IMAX: 81
      Fantasy: 78
      Mystery: 60
      Romance: 60
      Animation: 50
      Children: 48
      Horror: 45
      War: 26
      Western: 8
      Musical: 6
      Film-Noir: 5
      Documentary: 3
      (no genres listed): 1
    ````
      E ao comparar sua recomendação:
      
      ````
      User ID: 3 | Similaridade Media: 0.57
      Recommended Movies (MovieID: Score | Title):
      - 85774: 100.0% | Senna (2010),Documentary
      - 171011: 100.0% | Planet Earth II (2016),Documentary
      - 98491: 94.9% | Paperman (2012),Animation|Comedy|Romance
      - 159817: 93.3% | Planet Earth (2006),Documentary
      - 47: 92.5% | Seven (a.k.a. Se7en) (1995),Mystery|Thriller
      ````
      Nota-se que, apesar do perfil extremamente diverso do usuário 3 — que consome filmes de praticamente todos os gêneros com frequência significativa — o sistema de recomendação conseguiu sugerir títulos que refletem essa variedade. Entre os filmes recomendados, há documentários ("Senna", "Planet Earth II", "Planet Earth"), animação e comédia romântica ("Paperman"), além de suspense e mistério ("Seven"). Isso demonstra que o algoritmo LSH+KNN é capaz de identificar vizinhos relevantes mesmo para usuários com gostos amplos, e de gerar recomendações que abrangem diferentes gêneros presentes no histórico do usuário.

    Além disso, a similaridade média dos vizinhos encontrados (0.57) é relativamente alta, indicando que o sistema conseguiu localizar perfis próximos, mesmo diante da diversidade de interesses. A presença de documentários entre os principais recomendados, por exemplo, mostra que o sistema é sensível a nuances do perfil do usuário, não se limitando apenas aos gêneros mais populares, mas também incluindo títulos de nicho que fazem parte do seu histórico.

<h3>Comsumo de mémoria</h3>

Para avaliar o consumo de memória do algoritmo, é fundamental considerar os parâmetros utilizados, pois eles impactam diretamente a quantidade de dados armazenados e processados durante a execução. A análise do uso de memória é importante para garantir que o sistema seja eficiente e escalável, especialmente ao lidar com grandes volumes de dados ou ao ajustar parâmetros como o número de tabelas LSH, hiperplanos.


<div style="display: flex; justify-content: center; gap: 40px; flex-wrap: wrap; margin-bottom: 10px;">

  <div style="text-align: center;">
    <img src="https://hackmd.io/_uploads/H1b5JuxUxe.png" alt="memoria_1_1" width="400"/>
    <div>(a)</div>
  </div>

  <div style="text-align: center;">
    <img src="https://hackmd.io/_uploads/S1Zq1Ox8gx.png" alt="memoria_5_5" width="400"/>
    <div>(b)</div>
  </div>

  <div style="text-align: center; margin-top: 20px;">
    <img src="https://hackmd.io/_uploads/r1Zq1OlIgx.png" alt="memoria_7_5" width="400"/>
    <div>(c)</div>

  </div>

  <div style="text-align: center; margin-top: 20px;">
    <img src="https://hackmd.io/_uploads/HkZcJOxIeg.png" alt="memoria_16_16" width="400"/>
    <div>(d)</div>
  </div>

</div>

<p style="text-align: center; font-size: 14px; color: #4b5563; margin-top: 10px;">
  <em>Figura 1: Uso de memória para diferentes configurações de parâmetros LSH associado ao número de tabelas hash e hiperplanos (lxk) — (a) 1x1, (b) 5x5, (c) 7x5 e (d) 16x16. Nota-se que a medida que se aumenta os paramêtros o consumo de memória em uso se mantém prolongado, no entanto, com pico constante de 853.5 MB. </em>
</p>


Os gráficos apresentados na Figura 1 ilustram o uso de memória para diferentes configurações dos parâmetros LSH, variando o número de tabelas hash e hiperplanos (l × k): (a) 1×1, (b) 5×5, (c) 7×5 e (d) 16×16. Observa-se que, à medida que os parâmetros aumentam, o consumo de memória se mantém elevado por mais tempo durante a execução. Isso ocorre porque o aumento no número de tabelas e hiperplanos faz com que o sistema construa e mantenha estruturas de dados mais complexas e volumosas, exigindo que essas estruturas permaneçam acessíveis até o término do processamento. Como resultado, a memória é intensamente utilizada por um período prolongado, refletindo o maior volume de dados processados. No entanto, o pico de uso permanece constante em torno de 850 MB, indicando que o algoritmo consegue manter o consumo máximo de memória sob controle mesmo em cenários mais exigentes. Isso demonstra a eficiência do sistema em gerenciar recursos computacionais, permitindo ajustes nos parâmetros sem comprometer a estabilidade do ambiente.

<h2 id="conclusao">✔️ Conclusão</h2>

<p align="justify">
Este projeto demonstrou com sucesso a viabilidade e a eficácia da implementação de um sistema de recomendação de filmes capaz de lidar com a escala e a complexidade de um grande volume de dados, como o do dataset <b>MovieLens 25M</b>. Ao confrontar diretamente as limitações de escalabilidade de algoritmos tradicionais como o k-NN, cuja complexidade computacional se torna proibitiva em cenários de alta dimensionalidade, a abordagem híbrida adotada se mostrou uma solução robusta e performática.
</p>

<p align="justify">
A implementação em C++, otimizada com paralelização via OpenMP e estruturas de dados eficientes da STL, estabeleceu uma base sólida para o processamento de milhões de avaliações. O ponto central do sucesso do projeto foi a aplicação estratégica da técnica de <b>Hashing Sensível à Localidade (LSH)</b> como um mecanismo de filtragem inteligente. O LSH mitigou efetivamente a "maldição da dimensionalidade", reduzindo drasticamente o espaço de busca ao agrupar usuários com perfis de gosto similares em "buckets" com alta probabilidade. Essa busca aproximada, com sua complexidade sublinear, resolveu o principal gargalo de performance identificado na fundamentação teórica. A análise de resultados validou empiricamente a metodologia, revelando que uma configuração otimizada com 7 tabelas LSH e 5 hiperplanos oferece o melhor equilíbrio entre o tempo de execução e a qualidade das recomendações, mantendo a similaridade média em um patamar elevado sem incorrer em custos computacionais excessivos.
</p>

<p align="justify">
Ao combinar a velocidade do LSH para a seleção de um conjunto de candidatos promissores com a precisão da similaridade de cosseno para a seleção final dos vizinhos mais próximos, o sistema alcançou um equilíbrio notável entre eficiência e qualidade das recomendações. A validação qualitativa, comparando as sugestões geradas com os perfis de gênero dos usuários, confirmou que o algoritmo é capaz de gerar recomendações coerentes e personalizadas tanto para usuários de nicho quanto para aqueles com gostos diversificados. Além disso, a reprodutibilidade do sistema foi assegurada, demonstrando que, para um mesmo usuário, as recomendações são determinísticas e consistentes.
</p>

<p align="justify">
Como trabalhos futuros, o sistema poderia ser expandido de várias maneiras. A primeira seria a implementação de um mecanismo de atualização online para as tabelas LSH, permitindo que o modelo se adapte a novas avaliações em tempo real sem a necessidade de reconstruir todos os índices. Adicionalmente, a exploração de outros métodos de LSH, como aqueles baseados em distribuições p-estáveis, ou a incorporação de dados contextuais, como as tags e os gêneros dos filmes, no processo de vetorização, poderia refinar ainda mais a relevância e a personalização das sugestões, aprimorando a experiência do usuário final.
</p>

<h2 id="ambiente">🚀 Ambiente Compilação e Execução</h2>

Este projeto foi executado utilizando:
  * **Sistema Operacional**: Linux Ubuntu WSL @ Windows 11 (Windows Subsystem for Linux).
  * **Compilador**: GCC 13.3.0
  * **Hardware**: 13th Gen Intel(R) Core(TM) i7-13620H @ 2.40GHz, 24GB RAM, 512GB SSD, NVIDIA GeForce RTX 4050.
  

Para a compilação do projeto é necessario seguir alguns passos antes da execução: 

* **Clonar o repositório** 
<pre> git clone https://github.com/PeedroHG/RecommendationAlgorithm 
cd RecommendationAlgorithm  </pre>
* **Organização dos arquivos de entrada** 

Este projeto utiliza dados do conjunto [MovieLens 25M](https://grouplens.org/datasets/movielens/25m/).
Após o download, é necessario certificar de que os seguintes arquivos estejam na pasta dataset/ na raiz do projeto
```text
📂 dataset/ 
├── ratings.csv
├── movies.csv
```
* **Compilar o projeto**

Para compilar o projeto é utilizado um arquivo Makefile como script de automação para compilar  projeto. Esse arquivo define regras que simplificam o processo da compilação.

Para compilar o projeto de maneira padrão, de acordo com o arquio Makefile
<pre>make</pre>
Após compilar o projeto, o mesmo pode ser executado
<pre>make run</pre>
Ao final desse processo, os resultados do projeto executado estará no arquivo output.dat

Para a limpeza dos arquivos gerados
<pre>make clean</pre>

<h2 id="arquivos">📁 Arquivos Adicionais</h2>

<b>Geração de Perfis de Gênero</b>

Para fundamentar a análise qualitativa das recomendações geradas pelo algoritmo principal, foi desenvolvido um script auxiliar em Python. O objetivo deste módulo é processar os dados brutos e construir um perfil detalhado para cada usuário, quantificando seus interesses com base nos gêneros dos filmes que já assistiram.

O script que pode ser encontrado no [Google Collab](https://colab.research.google.com/drive/1h6yweLM0G7yanU6hY_rLDDC489Di15fv?usp=sharing), utiliza como entrada os arquivos ```ratings.csv``` e ```movies.csv``` e aplica o mesmo critério de filtragem do pipeline em C++  (considera apenas usuários que avaliaram 50 ou mais filmes). Para cada usuário válido, o script analisa seu histórico, extrai os gêneros de cada filme e agrega a contagem total por gênero. O resultado é um arquivo de saída ```user_most_frequent_genres.dat``` que mapeia cada ID de usuário ao seu perfil de consumo, como no exemplo abaixo:

```
User 1:
  Drama: 53
  Comedy: 23
  Romance: 18
  Adventure: 11
  Crime: 8
  ...
```

Este perfil de gênero serve como uma base de referência essencial na seção seguinte de Análise de Resultados. Com ele, é possível não apenas verificar se as recomendações geradas são coerentes com os gostos predominantes de um usuário, mas também classificar os usuários entre perfis de nicho (concentrados em poucos gêneros) e perfis dispersos (com interesses variados), permitindo uma avaliação mais robusta da eficácia do algoritmo em diferentes cenários.



<h2 id="autores"> 👥 Autores</h2>

* [<img src="https://cdn.jsdelivr.net/npm/simple-icons@latest/icons/github.svg" width="24" alt="GitHub">](https://github.com/peixotoigor) Igor Peixoto
* [<img src="https://cdn.jsdelivr.net/npm/simple-icons@latest/icons/github.svg" width="24" alt="GitHub">](https://github.com/PeedroHG) Pedro Galvão
* [<img src="https://cdn.jsdelivr.net/npm/simple-icons@latest/icons/github.svg" width="24" alt="GitHub">](https://github.com/joaopaulocruzdefaria) João Cruz
* [<img src="https://cdn.jsdelivr.net/npm/simple-icons@latest/icons/github.svg" width="24" alt="GitHub">](https://github.com/Lucas-Roseno) Lucas Roseno
* [<img src="https://cdn.jsdelivr.net/npm/simple-icons@latest/icons/github.svg" width="24" alt="GitHub">](https://github.com/LuizFernandosq) Luiz Santos

<details id="referencias">
  <summary><b>📚 Referências</b></summary>
  <ol>
    <li id="ref1">ALMANSOORI, Mahmood KM; MESZAROS, Andras; TELEK, Miklos. Fast and memory-efficient approximate minimum spanning tree generation for large datasets. Arabian Journal for Science and Engineering, v. 50, n. 2, p. 1233-1246, 2025.</li>
    <li id="ref2">PANDIT, Gaurav; RÖDER, Michael; NGONGA NGOMO, Axel-Cyrille. Evaluating Approximate Nearest Neighbour Search Systems on Knowledge Graph Embeddings. In: European Semantic Web Conference. Springer, Cham, 2025. p. 59-76.</li>
    <li id="ref3">YAMAMURA, Hugo Hiroshi. R – E. 2025. Disponível em: https://acervodigital.ufpr.br/xmlui/bitstream/handle/1884/93822/R%20-%20E%20-%20HUGO%20HIROSHI%20YAMAMURA.pdf?sequence=1&isAllowed=y. Acesso em: 2 jun. 2025.</li>
    <li id="ref4">ANASTASIU, David C.; KARYPIS, George. Fast parallel cosine k-nearest neighbor graph construction. In: 2016 6th Workshop on Irregular Applications: Architecture and Algorithms (IA3). IEEE, 2016. p. 50-53.</li>
    <li id="ref5">HYVÖNEN, Ville et al. Fast k-nn search. arXiv preprint arXiv:1509.06957, 2015.</li>
    <li id="ref6">FU, Jiuzhou; ZHAO, Dongfang. MPAD: A New Dimension-Reduction Method for Preserving Nearest Neighbors in High-Dimensional Vector Search. arXiv preprint arXiv:2504.16335, 2025.</li>
    <li id="ref7">RABBANI, Tahseen; BORNSTEIN, Marco; HUANG, Furong. Large-scale distributed learning via private on-device lsh. Advances in Neural Information Processing Systems, v. 36, p. 16153-16171, 2023.</li>
    <li id="ref8">ZHANG, Kunpeng; FAN, Shaokun; WANG, Harry Jiannan. An efficient recommender system using locality sensitive hashing. 2018.</li>
    <li id="ref9">LESKOVEC, J.; RAJARAMAN, A.; ULLMAN, J. D. Mining of Massive Datasets, Cambridge University Press, Cambridge [em linha]. 2014.</li>
    <li id="ref10">INDYK, Piotr; MOTWANI, Rajeev. Approximate nearest neighbors: towards removing the curse of dimensionality. In: Proceedings of the thirtieth annual ACM symposium on Theory of computing. 1998. p. 604-613.</li>
  </ol>
</details>
