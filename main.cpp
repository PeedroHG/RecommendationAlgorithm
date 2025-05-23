#include "process_csv.hpp"
#include "config.hpp"
#include <chrono>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>

// Função para ler o arquivo movies.csv e retornar um mapa: movie_id -> movie_title
std::unordered_map<int, std::string> readMoviesCSV(const std::string& movies_path) {
    std::unordered_map<int, std::string> movie_names;
    std::ifstream file(movies_path);
    if (!file) return movie_names;

    std::string line;
    std::getline(file, line); // Pula o cabeçalho
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string id_str, title;
        std::getline(ss, id_str, ',');
        std::getline(ss, title); // O resto da linha é o título
        int movie_id = std::stoi(id_str);
        movie_names[movie_id] = title;
    }
    return movie_names;
}

// Funções de KNN (as mesmas do seu projeto)
std::unordered_map<int, std::unordered_map<int, float>> convertToMap(const UsersData& usersData) {
    std::unordered_map<int, std::unordered_map<int, float>> result;
    for (const auto& [user, movies] : usersData) {
        for (const auto& [movie, rating] : movies) {
            result[user][movie] = rating;
        }
    }
    return result;
}

std::unordered_map<int, float> computeNorms(const std::unordered_map<int, std::unordered_map<int, float>>& user_map) {
    std::unordered_map<int, float> norms;
    for (const auto& [user, ratings] : user_map) {
        float sum = 0.0;
        for (const auto& [_, rating] : ratings) {
            sum += rating * rating;
        }
        norms[user] = std::sqrt(sum);
    }
    return norms;
}

float cosineSimilarity(const std::unordered_map<int, float>& a,
                        const std::unordered_map<int, float>& b,
                        float normA, float normB) 
{
    float dot = 0.0;

    if (a.size() < b.size()) {
        for (const auto& [movie, ratingA] : a) {
            if (b.count(movie)) {
                dot += ratingA * b.at(movie);
            }
        }
    } else {
        for (const auto& [movie, ratingB] : b) {
            if (a.count(movie)) {
                dot += ratingB * a.at(movie);
            }
        }
    }

    if (normA == 0.0 || normB == 0.0) return 0.0;

    return dot / (normA * normB);
}

std::vector<std::pair<int, float>> getKNN(int target_user,
                                           const std::unordered_map<int, std::unordered_map<int, float>>& user_map,
                                           const std::unordered_map<int, float>& norms,
                                           int K) 
{
    std::vector<std::pair<int, float>> neighbors;
    const auto& target = user_map.at(target_user);
    float normA = norms.at(target_user);

    for (const auto& [user_id, ratings] : user_map) {
        if (user_id == target_user) continue;

        float sim = cosineSimilarity(target, ratings, normA, norms.at(user_id));
        if (sim > 0.0) {
            neighbors.emplace_back(user_id, sim);
        }
    }

    std::sort(neighbors.begin(), neighbors.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    if ((int)neighbors.size() > K) neighbors.resize(K);

    return neighbors;
}

std::vector<std::pair<int, float>> recommendMovies(int target_user, int K,
    const std::unordered_map<int, std::unordered_map<int, float>>& user_map,
    const std::unordered_map<int, float>& norms) 
{
    auto neighbors = getKNN(target_user, user_map, norms, K);
    const auto& seen = user_map.at(target_user);

    std::unordered_map<int, float> total_score;
    std::unordered_map<int, float> sim_sum;

    for (const auto& [neighbor_id, similarity] : neighbors) {
        const auto& neighbor_ratings = user_map.at(neighbor_id);

        for (const auto& [movie, rating] : neighbor_ratings) {
            if (seen.count(movie)) continue;

            total_score[movie] += rating * similarity;
            sim_sum[movie] += similarity;
        }
    }

    std::vector<std::pair<int, float>> recommendations;
    for (const auto& [movie, score] : total_score) {
        recommendations.emplace_back(movie, score / sim_sum[movie]);
    }

    std::sort(recommendations.begin(), recommendations.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    return recommendations;
}

int main()
{
    auto start = std::chrono::high_resolution_clock::now();

    UsersData usersData;
    std::unordered_map<int, int> user_count, movie_count;
    std::unordered_set<int> user_validated, movie_validated;

    readCSV(ratings_path, usersData);
    countRatings(usersData, user_count, movie_count);
    filterValid(user_count, movie_count, user_validated, movie_validated, min_rating);
    filterUsersMovies(usersData, user_validated, movie_validated);
    writeCSV(filtred_dataset_path, usersData);
    writeRandomUsersToExplore(usersData, users_explore, explore_path);

    // Lê nomes dos filmes
    auto movie_names = readMoviesCSV(movies_path);

    // KNN
    auto user_map = convertToMap(usersData);
    auto norms = computeNorms(user_map);

    // Recomendações para cada usuário de explore.dat
    std::ifstream exploreFile(explore_path);
    std::ofstream outputFile("output.dat");
    int K = max_similar_neighbor;

    if (!exploreFile) {
        std::cerr << "Não foi possível abrir explore.dat\n";
        return 1;
    }
    if (!outputFile) {
        std::cerr << "Não foi possível criar output.dat\n";
        return 1;
    }

    int target_user;
    while (exploreFile >> target_user) {
        if (!user_map.count(target_user)) {
            outputFile << target_user << "\nUsuário não encontrado no dataset filtrado.\n\n";
            continue;
        }
        auto recommendations = recommendMovies(target_user, K, user_map, norms);

        outputFile << target_user << "\n";
        int count = 0;
        for (const auto& [movie, predicted] : recommendations) {
            if (count++ >= K) break;
            if (movie_names.count(movie))
                outputFile << movie_names[movie] << "\n";
            else
                outputFile << movie << "\n";
        }
        outputFile << "\n";
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time taken: " << elapsed.count() << " seconds" << std::endl;

    return 0;
}