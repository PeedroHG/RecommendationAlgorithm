#ifndef RECOMMENDER_ENGINE_HPP
#define RECOMMENDER_ENGINE_HPP

#include "types.hpp" 
#include <random> // Para geração de números aleatórios

UserItemMatrix convertToUserItemMatrix(const UserRatingsLog& users_ratings_log);
UserNormsMap computeUserNorms(const UserItemMatrix& user_item_matrix); // Mantido

// Função de similaridade de cosseno exata (mantida)
float calculateCosineSimilarity(const std::unordered_map<int, float>& ratings_user_a,
                                const std::unordered_map<int, float>& ratings_user_b,
                                float norm_user_a,
                                float norm_user_b);

// --- LSH Functions ---

/**
 * @brief Gera um conjunto de hiperplanos aleatórios.
 * Cada hiperplano é um vetor com componentes de uma distribuição normal padrão.
 * @param num_hyperplanes Número de hiperplanos a gerar neste conjunto.
 * @param dimensionality Dimensionalidade dos vetores de item/usuário (número de filmes únicos).
 * @param rng Gerador de números aleatórios.
 * @return HyperplaneSet Um conjunto de hiperplanos.
 */
HyperplaneSet generateSingleHyperplaneSet(int num_hyperplanes, int dimensionality, std::mt19937& rng);

/**
 * @brief Calcula o hash LSH para um vetor de avaliações de usuário usando um conjunto de hiperplanos.
 * @param user_ratings Mapa de (MovieID -> Rating) para o usuário.
 * @param hyperplane_set O conjunto de hiperplanos para esta tabela hash.
 * @param movie_to_idx Mapeamento de MovieID para índice de vetor denso.
 * @param dimensionality A dimensionalidade total (para consistência, embora não diretamente usada se o hiperplano já tiver o tamanho certo).
 * @return LSHHashValue O valor do hash.
 */
LSHHashValue computeLSHHash(const std::unordered_map<int, float>& user_ratings,
                            const HyperplaneSet& hyperplane_set,
                            const MovieIdToDenseIdxMap& movie_to_idx);
/**
 * @brief Constrói múltiplas tabelas hash LSH.
 * @param user_item_matrix A matriz de avaliações usuário-item.
 * @param all_hyperplane_sets Vetor contendo conjuntos de hiperplanos para cada tabela LSH.
 * @param movie_to_idx Mapeamento de MovieID para índice de vetor denso.
 * @param lsh_tables Vetor de saída de tabelas hash LSH (mapas de bucket).
 */
void buildLSHTables(const UserItemMatrix& user_item_matrix,
                    const std::vector<HyperplaneSet>& all_hyperplane_sets,
                    const MovieIdToDenseIdxMap& movie_to_idx,
                    std::vector<LSHBucketMap>& lsh_tables);

/**
 * @brief Encontra K vizinhos mais próximos aproximados para um usuário alvo usando LSH.
 * Coleta candidatos de buckets LSH e então calcula similaridade de cosseno exata para eles.
 * @param target_user_id O ID do usuário.
 * @param user_item_matrix A matriz completa de avaliações.
 * @param user_norms Normas pré-calculadas para todos os usuários.
 * @param all_hyperplane_sets Todos os conjuntos de hiperplanos para todas as tabelas LSH.
 * @param lsh_tables As tabelas hash LSH construídas.
 * @param movie_to_idx Mapeamento de MovieID para índice de vetor denso.
 * @param K O número de vizinhos a retornar.
 * @return NeighborList Lista de vizinhos aproximados.
 */
NeighborList findApproximateKNearestNeighborsLSH(
    int target_user_id,
    const UserItemMatrix& user_item_matrix,
    const UserNormsMap& user_norms,
    const std::vector<HyperplaneSet>& all_hyperplane_sets,
    const std::vector<LSHBucketMap>& lsh_tables,
    const MovieIdToDenseIdxMap& movie_to_idx,
    int K);

// Função de recomendação agora usará LSH para encontrar vizinhos
RecommendationList generateRecommendationsLSH(
    int target_user_id,
    int K_neighbors_for_recs, // K para o LSH
    const UserItemMatrix& all_user_ratings,
    const UserNormsMap& user_norms,
    const std::vector<HyperplaneSet>& all_hyperplane_sets,
    const std::vector<LSHBucketMap>& lsh_tables,
    const MovieIdToDenseIdxMap& movie_to_idx);

#endif // RECOMMENDER_ENGINE_HPP