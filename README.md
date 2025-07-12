# 🎥 Movie Recommendation Algorithm 

<details>
  <summary><b>🧭 Sumário</b></summary>
  <ol>
    <li><a href="#introducao">📘 Introdução</a></li>
    <li><a href="#fundamentacao">📖 Abordagem e Fundamentação Teórica</a></li>
    <li><a href="metodologia">🧪 Metodologia</a></li>
    <li><a href="#datasets">📈 Datasets</a></li>
    <li><a href="#estrutura">🧬 Estrutura do Projeto</a></li>
    <li><a href="#modelagem-e-implementacao">💻 Modelagem e Implementação</a></li>
    <li><a href="ambiente">🚀 Ambiente Compilação e Execução</a></li>
    <li><a href="analises-dos-resultados">📊 Análises dos Resultados</a></li>
    <li><a href="conclusao">✔️ Conclusão</a></li>
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

Algoritmos tradicionais como k-NN tradicional ou similaridade de cosseno direta, falham em oferecer baixa escabildiade e baixa velocidade de processamento para um alto volume de dados. O primeiro falha pois para encontrar os vizinhos mais próximos, calcula a distância ou similaridade de um ponto de consulta para todos os outros pontos no conjunto de dados. Isso leva a um alto custo computacional, que cresce quadraticamente (complexidade O(n²)) com o tamanho do dataset <a href="#ref1">[1]</a>, ou mais precisamente,  para a tarefa mais comum de uma única consulta de predição, a complexidade é descrita como O(n⋅d), frequentemente simplificada para O(n) quando a dimensionalidade é tratada como constante <a href="#ref2">[2]</a>. Para o <b>MovieLens 25M</b>, com mais de 162 mil usuários e 62 mil filmes, essa abordagem se torna inviável pois tanto o número de usuários e a dimensão de filmes a ser analisadas é alto. Adicionalmente para o segundo caso, calcular a similaridade para todos os pares de itens em um grande conjunto de dados é computacionalmente inviável <a href="#ref3">[3]</a>.
Além desses problemas,  fatores como alta dimensionalidade (conhecida com maldição da alta dimensionalidade), desempenho ineficiente para matrizes esparças e alto consumo de memória tornam essas abordagens pouco atrativivas <a href="#ref4">[4]</a>-<a href="#ref6">[6]</a>.

Em contraste, a técnica de <b>Hashing Sensível à Localidade (LSH) </b> se destaca como uma abordagem altamente eficiente para algoritmos de recomendação, superando as limitações de métodos tradicionais, especialmente em contextos de grandes volumes de dados <a href="#ref7">[7]</a>,<a href="#ref8">[8]</a>. O principal objetido da LSH  é agrupar itens de entrada semelhantes, associando-os a um mesmo hash com alta probabilidade, também conhecido como "buckets". Diferente das técnicas de hash convencionais, o LSH maximiza as colisões de hash em vez de minimizá-las <a href="#ref9">[9]</a>. Isso permite acelerar a busca por vizinhos próximos em conjuntos de dados de alta dimensionalidade, mitigando a "maldição da dimensionalidade" <a href="#ref10">[10]</a>.

Quando os dados são representados como vetores em um espaço de alta dimensão, a técnica de LSH mais comum utiliza hiperplanos aleatórios. O objetivo aqui é agrupar vetores que apontam em direções semelhantes, o que é medido pela <b>similaridade de cosseno</b>.  O processo funciona da seguinte forma:

* **Criação de Hiperplanos**: Vários hiperplanos aleatórios são gerados no espaço vetorial. Cada hiperplano divide o espaço em duas metades.

* **Geração do Hash**: Para cada vetor de dados, um "hash" é gerado com base em sua posição em relação a esses hiperplanos. Para cada hiperplano, se o vetor estiver de um lado, ele recebe um bit '1', e se estiver do outro lado, recebe um bit '0'.

* **Código de Hash**: Ao combinar os bits de todos os hiperplanos, cada vetor recebe um código de hash binário (uma sequência de 0s e 1s). Vetores que estão próximos no espaço angular (alta similaridade de cosseno) têm uma alta probabilidade de cair do mesmo lado de muitos dos hiperplanos aleatórios, resultando em códigos de hash idênticos ou muito semelhantes.

No contexto de LSH, esse código de hash binário serve como a chave para o bucket. A criação desses buckets de candidatos é a razão fundamental pela qual o LSH consegue mitigar a "maldição da dimensionalidade". A pergunta crucial que surge dessa abordagem é: qual é o verdadeiro ganho de desempenho e quais são os custos associados a essa aproximação?

Em contraste com uma busca por força bruta (k-NN), cujo custo computacional cresce de forma linear ou quadrática com o volume de dados (O(nd) ou O(n²)) e se torna rapidamente proibitivo, o LSH direciona a busca apenas para um pequeno subconjunto de itens promissores. Ao consultar apenas os candidatos que colidem no mesmo bucket, o tempo de busca é drasticamente reduzido para uma complexidade sublinear, representando o seu principal ganho. Contudo, essa eficiência não é gratuita e seu custo mais significativo reside na própria aproximação. O LSH aceita a possibilidade de erros, que se manifestam como falsos negativos, onde um vizinho real é perdido por não colidir em nenhum bucket, e falsos positivos, onde itens não similares colidem por acaso, exigindo verificações de distância desnecessárias que consomem tempo. A gestão desses erros constitui um segundo custo, o de engenharia, pois exige um ajuste fino dos parâmetros do algoritmo — como o número de tabelas de hash (L) e de funções por tabela (k) — para encontrar o balanço ideal entre a precisão dos resultados e a velocidade da consulta. Por fim, há o custo inicial de pré-processamento e memória, um investimento de tempo e recursos computacionais necessário para construir e armazenar as tabelas de hash antes que qualquer busca possa ser realizada.

A proposta desenvolvida neste estudo âncora-se na ideia simplista do k-NN e as vantagens da abordagem LSH. Para evitar essa busca exaustiva, o sistema utiliza LSH como um poderoso filtro inteligente na primeira etapa. Cada usuário, representado por seu perfil de avaliações de filmes, é mapeado para buckets LSH, que funcionam como "vizinhanças" de gostos, agrupando usuários com perfis de avaliação parecidos. Assim, quando é necessário encontrar os vizinhos mais próximos de um usuário, o sistema não varre todo o banco de dados. Em vez disso, ele usa o LSH para filtrar rapidamente um conjunto reduzido de candidatos promissores — aqueles que caíram nos mesmos buckets que o usuário alvo. Só então, dentro desse conjunto pré-filtrado e muito menor, é aplicado o cálculo de similaridade cosseno (KNN) para selecionar os K vizinhos mais próximos e, finalmente, gerar as recomendações.

Dessa forma, o sistema combina o melhor dos dois mundos: a velocidade da busca aproximada do LSH para identificar um bairro relevante de usuários e a precisão da métrica de similaridade do KNN para fazer a escolha final dentro do bucket. Esta abordagem em duas etapas resolve diretamente a questão da escalabilidade, permitindo que recomendações personalizadas sejam geradas de maneira eficiente, mesmo em um ecossistema com um número massivo de usuários e itens.

Ao gerar as recomenções faz necessário avaliar a qualidade das sugestões de filmes sugeridas pelo algoritmo. Em nossa abordagem de recomendação, cada filme candidato recebe um grau de preferência calculado a partir das avaliações de usuários semelhantes, de modo que os vizinhos com perfis mais próximos exercem maior influência. Para isso, combinamos cada avaliação com o grau de similaridade do usuário que a fez e, em seguida, normalizamos pelo total de similaridade acumulada, garantindo que opiniões mais alinhadas pesem mais no resultado final. Essa estimativa é então convertida para uma escala percentual, refletindo o nível de confiabilidade da recomendação. Por fim, apresentamos ao usuário apenas os títulos que ele ainda não viu, ordenados pelo valor percentual, de modo a destacar primeiro as opções com maior chance de agradar. </p>

<h2 id="datasets"> 📊 Datasets</h2>
O conjunto de dados utilizados no sistema de recomendação é o **MovieLens 25M**, disponível na plataforma Kaggle [`ml-25m`](https://www.kaggle.com/datasets/patriciabrezeanu/movielens-full-25-million-recommendation-data). Ele contém registros de classificação de filmes por estrelas, além de marcações com texto livre realizadas na plataforma oficial [MovieLens](http://movielens.org), um renomado serviço de recomendação de filmes.

Este extenso conjunto de dados apresenta as seguintes características gerais:

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

<h2 id="estrutura"> 🧬 Estrutura do Projeto</h2>

```text
📁 projeto/
├── 📂 data/              # Arquivos de entrada/saída
├── 📂 src/               # Códigos principais
├── 📂 results/           # Resultados e gráficos gerados
├── 📂 utils/             # Funções auxiliares
├── README.md
└── requirements.txt
```

<h2 id="modelagem-e-implementacao"> 💻 Modelagem e Implementação</h2>

A escolha do C++ para a implementação do sistema de recomendação de filmes foi motivada pela sua capacidade de oferecer controle de baixo nível e elevado desempenho computacional, características cruciais para processar eficazmente grandes volumes de dados. Esta robustez é complementada pela utilização estratégica de estruturas de dados otimizadas da Standard Template Library (STL) e pela paralelização intensiva via OpenMP, visando a maximização da eficiência e a escalabilidade do processamento, especialmente crítica para o manejo de datasets volumosos. A arquitetura adotada para o projeto é intrinsecamente modular, concebida para promover a clareza, a manutenibilidade e a extensibilidade. Esta organização lógica compreende um pipeline integral que se estende desde o processamento inicial e a curadoria dos dados até a fase final de geração de recomendações e a rigorosa avaliação do modelo. Para garantir uma execução sequencial e lógica, esses componentes foram articulados em fases distintas, a saber:

* ⚙️ **Fase 1:** <a href="#fase1-dados">Carregamento e Pré-processamento de Dados</a>
* 🏗️ **Fase 2:** <a href="#fase2-modelo">Construção de Componentes do Modelo Central</a>
* ⚡ **Fase 2.5:** <a href="#fase2_5-lsh">Construção do Modelo LSH</a>
* 💡 **Fase 3:** <a href="#fase3-recomendacoes">Geração de Recomendações LSH</a>
* 📊 **Módulo de Avaliação:** <a href="#modulo-avaliacao">Avaliação do Modelo</a>

### Definição de Tipos de Dados

Para promover a clareza, a legibilidade e a manutenibilidade do código-fonte, o projeto faz uso extensivo de **alias de tipos (type aliases)**. Essas definições, centralizadas no [arquivo `types.hpp`](src/types.hpp), padronizam as estruturas de dados complexas e conferem maior expressividade aos nomes das variáveis e parâmetros, refletindo seu propósito dentro do algoritmo. O detalhamento de cada tipo será apresentado contextualmente ao longo da descrição das fases de implementação, no momento de sua primeira utilização relevante.

<h3 id="fase1-dados">⚙️ Fase 1: Carregamento e Pré-processamento de Dados</h3>

<p align="justify">
A primeira fase da execução do sistema consiste no carregamento e na preparação dos dados. Este processo é fundamental para garantir a qualidade e a eficiência das recomendações geradas. As etapas são executadas na seguinte ordem:
</p>

<ol>
  <li>
    <p align="justify">
      <strong>Leitura dos Dados Brutos</strong>:
    </p>
    <ul>
      <li>O sistema inicia lendo o arquivo de avaliações, localizado em <code>../dataset/ratings.csv</code> , utilizando a função <code>readRatingsCSV</code>. Cada linha do CSV, contendo <code>userId</code>, <code>movieId</code> e <code>rating</code>, é processada e armazenada em uma estrutura de dados <code>UserRatingsLog</code>.</li>
      <li>Paralelamente, os títulos dos filmes são carregados a partir do arquivo <code>../dataset/movies.csv</code>, pela função <code>readMovieTitles</code> , associando cada <code>movieId</code> ao seu respectivo título em uma estrutura <code>MovieTitlesMap</code>.</li>
    </ul>
  </li>
  <li>
    <p align="justify">
      <strong>Contagem e Filtragem de Entidades</strong>:
    </p>
    <ul>
      <li>Para otimizar a base de dados, é realizada uma contagem do número de avaliações por usuário e por filme através da função <code>countEntityRatings</code>.</li>
      <li>Em seguida, o sistema aplica um filtro para manter apenas entidades com um número mínimo de interações. De acordo com o arquivo de configuração <code>config.hpp</code>, são considerados válidos os usuários e filmes que possuem pelo menos <strong>5 avaliações</strong> (<code>MIN_RATINGS_PER_ENTITY = 5</code>). Esta identificação é feita pela função <code>identifyValidEntities</code>.</li>
      <li>A função <code>filterUserRatingsLog</code> é então chamada para remover todos os usuários que não atendem ao critério mínimo e, para os usuários restantes, remove as avaliações de filmes que também não atendem ao limiar.</li>
    </ul>
  </li>
  <li>
    <p align="justify">
      <strong>Geração dos Arquivos de Saída do Pré-processamento</strong>:
    </p>
    <ul>
      <li><strong>Arquivo de Dados Filtrado</strong>: O conjunto de dados já processado e filtrado é salvo no arquivo <code>filtered_dataset.dat</code> , utilizando a função <code>writeFilteredRatingsToFile</code>. O formato de saída segue o padrão <code>usuario_id item_id1:nota1 item_id2:nota2 ...</code>:.</li>
  </li>
</ol>

<h3 id="fase2-modelo">🏗️ Fase 2: Construção de Componentes do Modelo Central</h3>

A Fase 2 do processo de implementação concentra-se na transformação dos dados de avaliações pré-processados em estruturas otimizadas, essenciais para os cálculos de similaridade subjacentes ao algoritmo de recomendação. Este conjunto de operações é implementado no módulo `recommender_engine.cpp/.hpp`.

Os principais processos realizados nesta fase são:

1.  **Mapeamento de Avaliações para Matriz Esparsa de Usuário-Item:**
    * Esta função é responsável por converter o formato inicial de log de avaliações, representado por `UserRatingsLog` (um `std::unordered_map` onde cada `UserID` é mapeado a um `std::vector` de pares `(MovieID, Rating)`), em uma `UserItemMatrix`. A `UserItemMatrix` é uma estrutura `std::unordered_map<int, std::unordered_map<int, float>>`, que, de forma qualitativa, age como uma representação esparsa da matriz de avaliações. Sua escolha é estratégica para **otimização de acesso**: ao aninhar `unordered_maps`, o sistema permite que, dado um `UserID` e um `MovieID`, a avaliação correspondente seja acessada diretamente com complexidade média de tempo constante ($O(1)$). Isso é fundamental para a eficiência em operações futuras, como o cálculo do produto escalar na similaridade de cosseno, onde a busca por avaliações específicas é frequente e a esparsidade dos dados evita a alocação desnecessária de memória para filmes não avaliados.

2.  **Pré-cálculo e Armazenamento das Normas de Usuários**
    * A `computeUserNorms` calcula a norma Euclidiana (L2) para o vetor de avaliações de cada usuário presente na recém-construída `UserItemMatrix`. As normas são então armazenadas na estrutura `UserNormsMap`, que é um `std::unordered_map<int, float>`. O **foco principal na otimização** aqui reside no **pré-cálculo e armazenamento dessas magnitudes**. A similaridade de cosseno, métrica central para determinar a semelhança entre usuários, exige a divisão pelo produto das normas dos vetores comparados. Ao ter as normas já computadas e acessíveis em $O(1)$, o sistema evita a repetição de um cálculo de raiz quadrada e somas para cada par de usuários durante as milhões de comparações de similaridade que ocorrem nas fases de busca por vizinhos. Este processo é adicionalmente **paralelizado utilizando OpenMP**, o que distribui a carga computacional de cálculo de normas entre múltiplos núcleos de processamento, acelerando significativamente a preparação dos dados para a fase LSH em grandes datasets.

Ao final desta fase, o sistema possui a `UserItemMatrix` e a `UserNormsMap` totalmente construídas, representando os perfis de usuários em um formato otimizado para as operações de hashing sensível à localidade e busca por vizinhos.

<h3 id="fase2_5-lsh">⚡ Fase 2.5: Construção do Modelo LSH</h3>

<p align="justify">
Esta é a fase central da otimização, onde as estruturas de dados do <i>Locality-Sensitive Hashing</i> são construídas. O objetivo é criar um índice que permita agrupar usuários com perfis de avaliação similares em "buckets" compartilhados, eliminando a necessidade de comparar cada usuário com todos os outros.
</p>

<ol>
  <li>
    <p align="justify">
      <p align="justify">
  <strong>Mapeamento de Dimensões</strong>:
</p>
<ul>
  <li>
    Os <code>movieId</code>s originais, que são esparsos e não sequenciais, são mapeados para um vetor de índices densos. Essa etapa é crucial para criar uma representação vetorial consistente para cada usuário. A estrutura <code>MovieIdToDenseIdxMap</code> funciona como um "dicionário tradutor".
  </li>
  <li>
    Como o código processa os <code>movieIds</code> únicos em ordem crescente (por usar um <code>std::set</code>), a atribuição dos índices densos é previsível e estável. Veja um exemplo simplificado:
  </li>
</ul>

| Original `movieId` (Esparso) | Novo `índice` (Denso) |
| :--------------------------- | :-------------------- |
| 1                            | 0                     |
| 8                            | 1                     |
| 780                          | 2                     |
| 59315                        | 3                     |
| ...e assim por diante        | ...                   |

<ul>
  <li>
    Com esse mapeamento, o perfil de avaliações de qualquer usuário pode ser convertido em um vetor de tamanho fixo (onde o tamanho é o número total de filmes únicos). A posição (índice) no vetor corresponde diretamente ao "Novo <code>índice</code> (Denso)" do filme.
  </li>
  <li>
    <strong>Exemplo prático:</strong> Se um usuário avaliou apenas os filmes <code>movieId=780</code> (nota 5.0) e <code>movieId=1</code> (nota 4.0), seu vetor de perfil seria:
    <br>
    <code>perfil_usuario = [ 4.0, 0.0, 5.0, 0.0, ... ]</code>
    <ul>
        <li>O valor <strong>4.0</strong> está no <strong>índice 0</strong> (pois <code>movieId=1</code> foi mapeado para 0).</li>
        <li>O valor <strong>5.0</strong> está no <strong>índice 2</strong> (pois <code>movieId=780</code> foi mapeado para 2).</li>
    </ul>
  </li>
  <li>
    É este vetor denso que o algoritmo LSH utiliza para realizar os cálculos de produto escalar com os hiperplanos.
  </li>
</ul>
  </li>
  <li>
    <p align="justify">
      <strong>Geração dos Hiperplanos Aleatórios</strong>:
    </p>
    <ul>
      <li>A função <code>generateSingleHyperplaneSet</code> é chamada em um laço para gerar múltiplos conjuntos de hiperplanos aleatórios. Cada conjunto, do tipo <code>HyperplaneSet</code>, corresponde a uma tabela de hash LSH.</li>
      <li>O número de tabelas de hash (<code>NUM_LSH_TABLES = 10</code>) e o número de hiperplanos por tabela (<code>NUM_HYPERPLANES_PER_TABLE = 16</code>) são determinados pelas constantes no arquivo <code>config.hpp</code>.</li>
      <li>Cada hiperplano é um vetor cujos componentes são amostrados de uma distribuição normal padrão (<code>std::normal_distribution</code>), garantindo a aleatoriedade necessária para a técnica.</li>
    </ul>
  </li>
  <li>
    <p align="justify">
      <strong>Construção das Tabelas de Hash (Buckets)</strong>:
    </p>
    <ul>
      <li>A função <code>buildLSHTables</code> itera sobre cada usuário da matriz e, para cada uma das tabelas de hash, calcula um valor de hash através da função <code>computeLSHHash</code>.</li>
      <li><code>computeLSHHash</code> projeta o vetor de avaliações do usuário em cada hiperplano (via produto escalar). O sinal do resultado (positivo ou negativo) determina o bit correspondente (1 ou 0) no hash final, que é do tipo <code>LSHHashValue</code>.</li>
      <li>Usuários com o mesmo hash são inseridos no mesmo "bucket" (uma lista de <code>userId</code>s) dentro de uma <code>LSHBucketMap</code>. Este processo é paralelizado com OpenMP (<code>#pragma omp parallel for</code>) para maior eficiência na construção das tabelas.</li>
    </ul>
  </li>
</ol>

<h3 id="fase3-recomendacoes">💡 Fase 3: Geração de Recomendações LSH</h3>

<h3 id="modulo-avaliacao">📊 Módulo de Avaliação: Avaliação do Modelo</h3>


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