#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "types.hpp" 
#include "config.hpp" // Para NUM_HYPERPLANES_PER_TABLE

// A função de avaliação original, pode ser mantida para comparar KNN exato
void evaluateKNNLeaveOneOut(const UserItemMatrix& user_item_matrix, int K_neighbors_for_eval);


/**
 * @brief Avalia o recomendador LSH usando Leave-One-Out.
 * As tabelas LSH são construídas uma vez com os dados completos.
 * Para cada item de teste de um usuário, o perfil *modificado* do usuário é hasheado
 * para encontrar candidatos nas tabelas LSH pré-construídas.
 * Similaridade exata é então calculada para esses candidatos.
 */
void evaluateLSHLeaveOneOut(
    const UserItemMatrix& user_item_matrix,
    const UserNormsMap& original_user_norms, // Normas de todos os usuários nos dados completos
    const std::vector<HyperplaneSet>& all_hyperplane_sets,
    const std::vector<LSHBucketMap>& lsh_tables, // Tabelas LSH construídas com dados completos
    const MovieIdToDenseIdxMap& movie_to_idx,
    int K_neighbors_for_eval);


#endif // EVALUATOR_HPP