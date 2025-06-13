#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <utility> // Para std::pair

// Alias de tipo para dados brutos de avaliação do usuário: UserID -> vetor de pares (MovieID, Rating)
// Esta é a estrutura primária após a leitura do CSV inicial de avaliações.
using UserRatingsLog = std::unordered_map<int, std::vector<std::pair<int, float>>>;

// Alias de tipo para nomes de filmes: MovieID -> MovieTitle
using MovieTitlesMap = std::unordered_map<int, std::string>;

// Alias de tipo para matriz de avaliação usuário-item: UserID -> (MovieID -> Rating)
// Esta estrutura é mais conveniente para algoritmos de recomendação.
using UserItemMatrix = std::unordered_map<int, std::unordered_map<int, float>>;

// Alias de tipo para normas de usuário: UserID -> Norma L2 do vetor de avaliações
using UserNormsMap = std::unordered_map<int, float>;

// Alias de tipo para uma lista de vizinhos: Vetor de pares (UserID, SimilarityScore)
using NeighborList = std::vector<std::pair<int, float>>;

// Alias de tipo para uma lista de recomendações: Vetor de pares (MovieID, PredictedScore)
using RecommendationList = std::vector<std::pair<int, float>>;

#endif // TYPES_HPP