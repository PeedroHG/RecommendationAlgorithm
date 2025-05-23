#ifndef RECOMMENDER_ENGINE_HPP
#define RECOMMENDER_ENGINE_HPP

#include "types.hpp" // Inclui UserRatingsLog, UserItemMatrix, UserNormsMap, etc.

/**
 * @brief Converte o log bruto de avaliações de usuários (UserID -> vetor de pares)
 * em uma matriz usuário-item (UserID -> (MovieID -> Mapa de Rating)).
 * @param users_ratings_log Os dados brutos de avaliações de usuários.
 * @return UserItemMatrix A estrutura de dados convertida.
 */
UserItemMatrix convertToUserItemMatrix(const UserRatingsLog& users_ratings_log);

/**
 * @brief Calcula a norma L2 (norma Euclidiana) do vetor de avaliações de cada usuário.
 * Norma(u) = sqrt(soma(rating_ui^2 para todos os itens i avaliados por u)).
 * @param user_item_matrix A matriz usuário-item.
 * @return UserNormsMap Mapeando UserID para sua norma L2.
 */
UserNormsMap computeUserNorms(const UserItemMatrix& user_item_matrix);

/**
 * @brief Calcula a similaridade de cosseno entre os vetores de avaliação de dois usuários.
 * CosSim(a, b) = produto_escalar(a, b) / (norma(a) * norma(b)).
 * Itera sobre os itens do usuário com menos avaliações para eficiência.
 * @param ratings_user_a Mapa de avaliações para o usuário A (MovieID -> Rating).
 * @param ratings_user_b Mapa de avaliações para o usuário B (MovieID -> Rating).
 * @param norm_user_a Norma L2 pré-calculada para o usuário A.
 * @param norm_user_b Norma L2 pré-calculada para o usuário B.
 * @return float A pontuação de similaridade de cosseno (entre -1 e 1, tipicamente 0 a 1 para avaliações positivas).
 * Retorna 0 se alguma norma for zero ou não houver itens em comum.
 */
float calculateCosineSimilarity(const std::unordered_map<int, float>& ratings_user_a,
                                const std::unordered_map<int, float>& ratings_user_b,
                                float norm_user_a,
                                float norm_user_b);

/**
 * @brief Encontra os K vizinhos mais próximos para um usuário alvo com base na similaridade de cosseno.
 * @param target_user_id O ID do usuário para quem encontrar vizinhos.
 * @param all_user_ratings A matriz usuário-item completa de todos os usuários.
 * @param all_user_norms Normas pré-calculadas para todos os usuários.
 * @param K O número de vizinhos mais próximos a retornar.
 * @return NeighborList Uma lista de pares (neighbor_user_id, similarity_score), ordenada por similaridade decrescente.
 * O tamanho da lista será no máximo K.
 */
NeighborList findKNearestNeighbors(int target_user_id,
                                   const UserItemMatrix& all_user_ratings,
                                   const UserNormsMap& all_user_norms,
                                   int K);

/**
 * @brief Gera recomendações de filmes para um usuário alvo com base em seus K vizinhos mais próximos.
 * Usa uma média ponderada das avaliações dos vizinhos para filmes que o usuário alvo não viu.
 * PredictedRating(u,i) = soma(sim(u,v) * rating(v,i)) / soma(abs(sim(u,v))) para vizinhos v que avaliaram i.
 * @param target_user_id O ID do usuário para gerar recomendações.
 * @param K_neighbors_count O número de vizinhos a usar (parâmetro K para getKNN).
 * @param all_user_ratings A matriz usuário-item completa.
 * @param all_user_norms Normas pré-calculadas para todos os usuários.
 * @return RecommendationList Uma lista de pares (movie_id, predicted_score), ordenada por pontuação prevista decrescente.
 */
RecommendationList generateRecommendations(int target_user_id,
                                           int K_neighbors_count,
                                           const UserItemMatrix& all_user_ratings,
                                           const UserNormsMap& all_user_norms);

#endif // RECOMMENDER_ENGINE_HPP