<h1 align="center">Movie Recommendation Algorithm </h1>

## 📃 Introdução
<p align="justify">
Com o rápido avanço tecnológico e a popularização da internet, o volume de dados gerados diariamente cresceu exponencialmente. Essa vasta quantidade de informação, embora rica, apresenta um desafio significativo: como processá-la e transformá-la em algo útil e significativo para os usuários?

Essa sobrecarga de informação pode levar a uma sensação de desorientação. Em plataformas de streaming de vídeo, por exemplo, a imensa disponibilidade de títulos, apesar de ser uma vantagem, pode gerar insatisfação, pois o usuário se sente sobrecarregado com tantas opções de escolha. Nesse contexto, os sistemas de recomendação surgem como uma solução. Eles atuam como filtros inteligentes, avaliando as informações disponíveis sobre o histórico e os prováveis interesses dos usuários para, então, sugerir conteúdos. As recomendações geradas por esses sistemas não apenas reduzem a sobrecarga de escolha, mas também ajudam os usuários a descobrir o que é mais relevante e adequado aos seus gostos. Assim, transformam a abundância de dados em uma experiência personalizada e satisfatória.

Este projeto tem como objetivo desenvolver um protótipo de algoritmo de recomendação de filmes, baseado na análise dos perfis dos usuários e nas características dos itens disponíveis. A abordagem adotada busca identificar padrões de preferências e comportamentos para oferecer sugestões personalizadas, aprimorando a experiência do usuário e a relevância das recomendações.

</p>

## 📊DataSets
<p align="justify">

O conjunto de dados utilizados no sistema de recomendação está disponível na plataforma Kaggle [`ml-25m`](https://www.kaggle.com/datasets/garymk/movielens-25m-dataset), e contém os registros de classificação de filmes por estrelas além da marcação com texto livre realizadas na plataforma [MovieLens](http://movielens.org), um serviço de recomendação de filmes. 
O conjunto de dados possui as seguintes características:

- **Total de avaliações:** 25.000.095 classificações
- **Total de marcações (tags):** 1.093.360 tags aplicadas
- **Quantidade de filmes:** 62.423 filmes incluídos
- **Período de coleta:** 9 de janeiro de 1995 a 21 de novembro de 2019
- **Usuários:** 162.541 usuários selecionados aleatoriamente

## 📂 Arquivos Contidos
Os dados são organizados nos seguintes arquivos:
- `genome-scores.csv` – Contém pontuações de relevância de tags.
- `genome-tags.csv` – Contém descrições das tags aplicadas.
- `links.csv` – Identificadores que vinculam filmes a fontes externas (IMDb e TMDb).
- `movies.csv` – Lista de filmes, incluindo títulos e gêneros.
- `ratings.csv` – Registra classificações dos filmes pelos usuários.
- `tags.csv` – Contém tags atribuídas pelos usuários.

## 👥 Usuários e Estrutura dos Dados
- Todos os usuários selecionados avaliaram pelo menos **20 filmes**.
- Não há informações demográficas disponíveis.
- Cada usuário é representado por um **ID anônimo**.
- Os arquivos do conjunto de dados são escritos no formato .csv, contendo uma única linha de cabeçalho.  
- Colunas que contêm vírgulas (`,`) são escapadas usando **aspas duplas** (`"`).  
- Os arquivos são codificados em **UTF-8**.    

Para alimentar o sistema de recomendação os datasets utilizados foram: `movies.csv` e `ratings.csv`

### Estrutura do Arquivo de Avaliações e Filmes (`ratings.csv`, `movies.csv`)
- O arquivo `ratings.csv` contém todas as avaliações dos usuários.  
Cada linha do arquivo após a linha de cabeçalho representa uma **avaliação de um filme por um usuário**, no seguinte formato:

```csv
userId,movieId,rating,timestamp
```
📌 Descrição dos Campos:

- userId → Integer: Identificador único e anônimo do usuário que realizou a avaliação.
- movieId → Integer: Identificador único do filme avaliado, consistente entre os arquivos do dataset.
- rating → Float:  Nota atribuída pelo usuário ao filme, variando de 0.5 a 5.0 estrelas, com incrementos de 0.5.
- timestamp → Integer: 
Representa a data e hora da avaliação em segundos desde 1º de janeiro de 1970 (UTC).

 O arquivo `movies.csv` contém informações sobre os filmes presentes no conjunto de dados MovieLens 25M.  
Cada linha do arquivo, após a linha de cabeçalho, representa um filme e segue o seguinte formato:

```csv
movieId,title,genres
```
📌 Descrição dos Campos:

- movieId → Integer:  Identificador único do filme, consistente entre os arquivos do dataset.
- title → String: Título do filme, que inclui o ano de lançamento entre parênteses. Os títulos podem conter erros e inconsistências.
- genres → String : Lista de gêneros separados separados por |. Os filmes no conjunto de dados podem pertencer a um ou mais dos seguintes gêneros:

```md
- 🎬 Action (Ação)
- 🏔️ Adventure (Aventura)
- 🐭 Animation (Animação)
- 👶 Children's (Infantil)
- 🤣 Comedy (Comédia)
- 🕵️‍♂️ Crime (Crime)
- 🎥 Documentary (Documentário)
- 🎭 Drama (Drama)
- 🧙 Fantasy (Fantasia)
- 🎞️ Film-Noir (Cinema Noir)
- 👻 Horror (Terror)
- 🎶 Musical (Musical)
- 🔍 Mystery (Mistério)
- ❤️ Romance (Romance)
- 🚀 Sci-Fi (Ficção Científica)
- 😱 Thriller (Suspense)
- ⚔️ War (Guerra)
- 🤠 Western (Faroeste)
- ❌ (no genres listed) (Sem gênero especificado)
```

</p>


## Implementação
<p align="justify">
O conjunto de dados - arquivo ratings.csv - apresenta um desafio quanto a dimensionalidade. Contendo mais de 25 milhões de linhas, a leitura e o tratamento do arquivo pode representar um gargalho caso não houver uma escolhada da estrutura de dados adequada para lidar com quantidade de dados.

Analisando a estrutura dos dados percebe-se que não há ordenação dos dados, o que torna o uso de uma estrtura do tipo vector inviável. Aliado a isso, o pré-processamento que inclui:
- apenas usuários que tenham realizado pelo menos 50 avaliações distintas;
- apenas filmes avaliados por pelo menos 50 usuários;
- remover registros duplicados ou inconsistentes.

O que confirma que o uso de vector insustentável.  A estrutura unorderd_map baseado nas tabelas hash oferecem vantagens sobre a vector uma vez que o tempo de acesso é considerado constante O(1) para inserseção, remoção e busca; evita duplicações desncessárias e recuperação eficiente de elementos associados [Colocar referência].

O algoritmo inicia-se partindo da leitura do arquivo ratings.csv. A leitura é 
diferenaçe entre unordedred_map e map

## LSH
<p>

</p>

## Referências

JAYALAKSHMI, S.; GANESH, N.; ČEP, R.; MURUGAN, J. S. Movie Recommender Systems: Concepts, Methods, Challenges, and Future Directions. Sensors (Basel, Switzerland), v. 22, n. 13, p. 4904, 2022. Disponível em: https://doi.org/10.3390/s22134904. Acesso em: 5 jun. 2025.

GARYMK. MovieLens 25M Dataset. Kaggle, 2021. Disponível em: https://www.kaggle.com/datasets/garymk/movielens-25m-dataset. Acesso em: 5 jun. 2025.

ZHANG, Yilin; ZHANG, Lingling. Movie Recommendation Algorithm Based on Sentiment Analysis and LDA. Procedia Computer Science, v. 199, p. 871–878, 2022. Disponível em: ScienceDirect. Acesso em: 5 jun. 2025.


AHUJA, Rishabh; SOLANKI, Arun; NAYYAR, Anand. Movie recommender system using k-means clustering and k-nearest neighbor. In: 2019 9th International Conference on Cloud Computing, Data Science & Engineering (Confluence). IEEE, 2019. p. 263-268.