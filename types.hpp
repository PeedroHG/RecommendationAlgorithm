#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <utility> // Para std::pair
#include <cstdint> // Para uint64_t

// Alias de tipo para dados brutos de avaliação do usuário: UserID -> vetor de pares (MovieID, Rating)
using UserRatingsLog = std::unordered_map<int, std::vector<std::pair<int, float>>>;

// Alias de tipo para nomes de filmes: MovieID -> MovieTitle
using MovieTitlesMap = std::unordered_map<int, std::string>;

// Alias de tipo para matriz de avaliação usuário-item: UserID -> (MovieID -> Rating)
using UserItemMatrix = std::unordered_map<int, std::unordered_map<int, float>>;

// Alias de tipo para normas de usuário: UserID -> Norma L2 do vetor de avaliações
using UserNormsMap = std::unordered_map<int, float>;

// Alias de tipo para uma lista de vizinhos: Vetor de pares (UserID, SimilarityScore)
using NeighborList = std::vector<std::pair<int, float>>;

// Alias de tipo para uma lista de recomendações: Vetor de pares (MovieID, PredictedScore)
using RecommendationList = std::vector<std::pair<int, float>>;

// LSH Related Types
using Hyperplane = std::vector<float>; // Um único hiperplano
using HyperplaneSet = std::vector<Hyperplane>; // Um conjunto de hiperplanos para uma tabela hash LSH
using LSHHashValue = uint64_t; // Tipo do valor hash LSH (se k <= 64)

// Estrutura para uma tabela hash LSH: HashValue -> Lista de UserIDs
using LSHBucketMap = std::unordered_map<LSHHashValue, std::vector<int>>;

// Mapeamento de MovieID original para um índice denso (0 a D-1)
using MovieIdToDenseIdxMap = std::unordered_map<int, int>;
// Mapeamento reverso (opcional, mas pode ser útil)
using DenseIdxToMovieIdVec = std::vector<int>;


#endif // TYPES_HPP