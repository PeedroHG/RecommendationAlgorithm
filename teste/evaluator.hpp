#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "types.hpp" // Para UserItemMatrix

/**
 * @brief Avalia o recomendador K-NN usando validação cruzada Leave-One-Out.
 * Para cada usuário, e para cada item avaliado por esse usuário:
 * 1. Remove temporariamente esse item (o item "deixado de fora") do perfil do usuário.
 * 2. Recalcula a norma do usuário com base em seu perfil modificado.
 * 3. Encontra K-Vizinhos Mais Próximos para este perfil de usuário modificado contra todos os outros usuários (usando seus perfis completos).
 * 4. Prediz a avaliação para o item "deixado de fora" usando esses vizinhos.
 * 5. Calcula o erro (diferença entre a avaliação real e a prevista).
 * Acumula erros para reportar Root Mean Square Error (RMSE) e Mean Absolute Error (MAE).
 *
 * @param user_item_matrix A matriz usuário-item completa (UserID -> MovieID -> Rating).
 * @param K_neighbors_for_eval O número de vizinhos (K) a usar para predição durante a avaliação.
 */
void evaluateKNNLeaveOneOut(const UserItemMatrix& user_item_matrix, int K_neighbors_for_eval);

#endif // EVALUATOR_HPP