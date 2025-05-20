#include "cossineSimilarity.hpp"
#include <unordered_map>
#include <cmath>

float cossineSimilarity(const std::unordered_map<int, float>& user1, const std::unordered_map<int, float>& user2) {
    // A função calcula a similaridade do cosseno entre dois usuários representados por seus ratings de filmes.
    // user1 e user2 são mapas que associam o ID do filme ao rating dado pelo usuário.
    // A similaridade do cosseno é calculada como o produto escalar dos dois vetores dividido pelo produto de suas normas.
    // Se o vetor for nulo, a similaridade é 0.
    // A função retorna um valor entre -1 e 1, onde 1 significa que os usuários são idênticos, 0 significa que não há similaridade e -1 significa que eles são opostos.

    // A função cosseno e rigida pela seguinte fórmula:
    // cos(θ) = (A . B) / (||A|| * ||B||)
    // onde A e B são os vetores de ratings dos usuários, A . B é o produto escalar e ||A|| e ||B|| são as normas dos vetores.
    // Para calcular a norma, usamos a fórmula ||A|| = sqrt(A1^2 + A2^2 + ... + An^2), onde Ai é o rating do filme i dado pelo usuário A.
    // O produto escalar é calculado como A . B = A1*B1 + A2*B2 + ... + An*Bn, onde Ai e Bi são os ratings do filme i dados pelos usuários A e B, respectivamente.



    float dotProduct = 0.0f;
    float normA = 0.0f;
    float normB = 0.0f;

    for (const auto& [movie, rating] : user1) {
        if (user2.find(movie) != user2.end()) {
            dotProduct += rating * user2.at(movie);
        }
        normA += rating * rating;
    }

    for (const auto& [movie, rating] : user2) {
        normB += rating * rating;
    }

    if (normA == 0.0f || normB == 0.0f) {
        return 0.0f; // Retorna 0 se um dos vetores for nulo
    }

    return dotProduct / (std::sqrt(normA) * std::sqrt(normB));
}