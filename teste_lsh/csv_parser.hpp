#ifndef CSV_PARSER_HPP
#define CSV_PARSER_HPP

#include "types.hpp" // Inclui UserRatingsLog, MovieTitlesMap
#include <string>
#include <unordered_map>
#include <unordered_set>

/**
 * @brief Lê dados de filmes de um arquivo CSV.
 * @param movies_csv_path Caminho para o arquivo movies.csv (espera formato: movieId,title,genres).
 * @return MovieTitlesMap mapeando IDs de filmes para títulos de filmes.
 */
MovieTitlesMap readMovieTitles(const std::string& movies_csv_path);

/**
 * @brief Lê dados de avaliações de usuários de um arquivo CSV.
 * Assume formato CSV: userId,movieId,rating[,timestamp] (timestamp é ignorado).
 * Usa um buffer para I/O potencialmente mais rápido.
 * @param ratings_csv_path Caminho para o arquivo CSV de avaliações.
 * @param users_ratings_log Mapa de saída para armazenar avaliações de usuários. Limpo antes da leitura.
 */
void readRatingsCSV(const std::string& ratings_csv_path, UserRatingsLog& users_ratings_log);

/**
 * @brief Conta o número de avaliações por usuário e por filme.
 * @param users_ratings_log Os dados brutos de avaliações de usuários.
 * @param user_rating_counts Mapa de saída: UserID -> contagem de avaliações por este usuário.
 * @param movie_rating_counts Mapa de saída: MovieID -> contagem de avaliações para este filme.
 */
void countEntityRatings(const UserRatingsLog& users_ratings_log,
                        std::unordered_map<int, int>& user_rating_counts,
                        std::unordered_map<int, int>& movie_rating_counts);

/**
 * @brief Identifica usuários e filmes que atendem a um limiar mínimo de contagem de avaliações.
 * @param user_rating_counts Mapa de UserID -> contagem de avaliações.
 * @param movie_rating_counts Mapa de MovieID -> contagem de avaliações.
 * @param valid_user_ids Conjunto de saída para armazenar IDs de usuários que atendem ao limiar.
 * @param valid_movie_ids Conjunto de saída para armazenar IDs de filmes que atendem ao limiar.
 * @param min_ratings O número mínimo de avaliações requerido.
 */
void identifyValidEntities(const std::unordered_map<int, int>& user_rating_counts,
                           const std::unordered_map<int, int>& movie_rating_counts,
                           std::unordered_set<int>& valid_user_ids,
                           std::unordered_set<int>& valid_movie_ids,
                           int min_ratings);

/**
 * @brief Filtra os dados principais de avaliações de usuários para manter apenas usuários válidos e suas avaliações para filmes válidos.
 * Modifica users_ratings_log in-place.
 * @param users_ratings_log Os dados de avaliações de usuários a serem filtrados (serão modificados).
 * @param valid_user_ids Conjunto de IDs de usuários a serem mantidos.
 * @param valid_movie_ids Conjunto de IDs de filmes para os quais manter avaliações.
 */
void filterUserRatingsLog(UserRatingsLog& users_ratings_log,
                          const std::unordered_set<int>& valid_user_ids,
                          const std::unordered_set<int>& valid_movie_ids);

/**
 * @brief Escreve os dados filtrados de avaliações de usuários para um arquivo em um formato específico.
 * Formato por linha: userID movieId1:rating1 movieId2:rating2 ...
 * @param output_path Caminho para o arquivo de saída.
 * @param users_ratings_log Os dados filtrados de avaliações de usuários.
 */
void writeFilteredRatingsToFile(const std::string& output_path, const UserRatingsLog& users_ratings_log);

/**
 * @brief Seleciona um número especificado de IDs de usuários aleatórios do conjunto de dados e os escreve em um arquivo.
 * @param users_ratings_log Os dados de avaliações de usuários (tipicamente filtrados).
 * @param num_users_to_select O número de IDs de usuários aleatórios a serem selecionados.
 * @param output_path Caminho para o arquivo onde os IDs de usuários serão escritos, um por linha.
 */
void writeRandomUserIdsToExplore(const UserRatingsLog& users_ratings_log,
                                 size_t num_users_to_select,
                                 const std::string& output_path);

#endif // CSV_PARSER_HPP