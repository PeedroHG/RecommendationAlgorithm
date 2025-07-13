#ifndef CSV_PARSER_HPP
#define CSV_PARSER_HPP

/**
 * @file csv_parser.hpp
 * @brief Declarações de funções para o parsing e pré-processamento de dados de avaliações de filmes.
 *
 * Este arquivo contém as assinaturas das funções responsáveis por ler, filtrar,
 * e preparar os dados brutos dos arquivos CSV para serem usados pelo motor de recomendação.
 * As otimizações de performance, como paralelismo, são aplicadas nas implementações
 * correspondentes em csv_parser.cpp.
 */

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "types.hpp"

/**
 * @brief Lê os títulos dos filmes de um arquivo CSV.
 * @param movies_csv_path Caminho para o arquivo movies.csv (formato esperado: movieId,title,genres).
 * @return MovieTitlesMap Um mapa que associa o ID de cada filme ao seu título.
 */
MovieTitlesMap readMovieTitles(const std::string& movies_csv_path);

/**
 * @brief Lê as avaliações de um grande arquivo CSV de forma otimizada e paralela.
 * @details Esta é a função mais crítica para a performance do pré-processamento.
 * Ela lê o arquivo em grandes blocos, distribui o processamento desses blocos
 * entre múltiplas threads usando OpenMP, e depois combina os resultados.
 * @param ratings_csv_path Caminho para o arquivo ratings.csv.
 * @param users_ratings_log Referência para o mapa que será preenchido com os dados das avaliações.
 * O mapa é limpo antes da leitura.
 */
void readRatingsCSV(const std::string& ratings_csv_path, UserRatingsLog& users_ratings_log);

/**
 * @brief Conta o número total de avaliações feitas por cada usuário e recebidas por cada filme.
 * @details A contagem é paralelizada para acelerar o processo em grandes datasets.
 * @param users_ratings_log O log de avaliações, onde os dados brutos estão armazenados.
 * @param user_rating_counts Mapa de saída (UserID -> contagem de avaliações).
 * @param movie_rating_counts Mapa de saída (MovieID -> contagem de avaliações).
 */
void countEntityRatings(const UserRatingsLog& users_ratings_log,
                        std::unordered_map<int, int>& user_rating_counts,
                        std::unordered_map<int, int>& movie_rating_counts);

/**
 * @brief Identifica usuários e filmes que atendem a um critério mínimo de avaliações.
 * @details Esta função cria conjuntos de IDs "válidos" que serão usados para filtrar o dataset,
 * removendo usuários e filmes com pouca atividade para reduzir ruído.
 * @param user_rating_counts Mapa com a contagem de avaliações por usuário.
 * @param movie_rating_counts Mapa com a contagem de avaliações por filme.
 * @param min_ratings O número mínimo de avaliações que um usuário ou filme deve ter para ser considerado válido.
 * @param out_valid_user_ids Conjunto de saída para armazenar os IDs de usuários válidos.
 * @param out_valid_movie_ids Conjunto de saída para armazenar os IDs de filmes válidos.
 */
void identifyValidEntities(const std::unordered_map<int, int>& user_rating_counts,
                           const std::unordered_map<int, int>& movie_rating_counts,
                           int min_ratings,
                           std::unordered_set<int>& out_valid_user_ids,
                           std::unordered_set<int>& out_valid_movie_ids);

/**
 * @brief Filtra o log de avaliações para manter apenas os usuários e filmes válidos.
 * @details A operação de filtragem é executada em paralelo para maior eficiência.
 * @param original_log O log de avaliações original e completo.
 * @param valid_user_ids Um conjunto de IDs de usuários que devem ser mantidos.
 * @param valid_movie_ids Um conjunto de IDs de filmes cujas avaliações devem ser mantidas.
 * @param filtered_log O log de avaliações resultante, contendo apenas os dados filtrados.
 */
void filterUserRatingsLog(const UserRatingsLog& original_log,
                        const std::unordered_set<int>& valid_user_ids,
                        const std::unordered_set<int>& valid_movie_ids,
                        UserRatingsLog& filtered_log);

/**
 * @brief Escreve o log de avaliações filtrado em um arquivo de saída.
 * @details A geração das strings de texto para o arquivo é feita em paralelo.
 * O formato de saída é: userID movieId1:rating1 movieId2:rating2 ...
 * @param output_path O caminho do arquivo de saída.
 * @param users_ratings_log O log de avaliações filtrado a ser escrito.
 */
void writeFilteredRatingsToFile(const std::string& output_path, const UserRatingsLog& users_ratings_log);

/**
 * @brief Seleciona uma amostra aleatória de usuários e escreve seus IDs em um arquivo.
 * @details Este arquivo é útil para a fase de testes e geração de recomendações,
 * permitindo focar em um subconjunto de usuários.
 * @param users_ratings_log O log de avaliações de onde os IDs serão extraídos.
 * @param num_users_to_select O número de usuários aleatórios a serem selecionados.
 * @param output_path O caminho do arquivo de saída onde os IDs serão escritos, um por linha.
 */
void writeRandomUserIdsToExplore(const UserRatingsLog& users_ratings_log,
                                 size_t num_users_to_select,
                                 const std::string& output_path);

#endif // CSV_PARSER_HPP