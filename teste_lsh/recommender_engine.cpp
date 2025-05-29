#include "recommender_engine.hpp"
#include "config.hpp" // Para NUM_HYPERPLANES_PER_TABLE
#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>
#include <set> // Para coletar candidatos únicos

#ifdef _OPENMP
#include <omp.h>
#endif

// convertToUserItemMatrix e computeUserNorms permanecem como na versão OpenMP anterior
UserItemMatrix convertToUserItemMatrix(const UserRatingsLog& users_ratings_log) {
    UserItemMatrix user_item_matrix;
    for (const auto& user_entry : users_ratings_log) {
        for (const auto& movie_rating_pair : user_entry.second) {
            user_item_matrix[user_entry.first][movie_rating_pair.first] = movie_rating_pair.second;
        }
    }
    return user_item_matrix;
}

UserNormsMap computeUserNorms(const UserItemMatrix& user_item_matrix) {
    UserNormsMap norms_map;
    std::vector<std::pair<int, const std::unordered_map<int, float>*>> user_entries_vec;
    user_entries_vec.reserve(user_item_matrix.size());
    for (const auto& pair : user_item_matrix) {
        user_entries_vec.push_back({pair.first, &pair.second});
    }
    std::vector<std::pair<int, float>> norms_results_vec(user_entries_vec.size());
    #pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < user_entries_vec.size(); ++i) {
        const auto& user_id = user_entries_vec[i].first;
        const auto& ratings = *(user_entries_vec[i].second);
        float sum_of_squares = 0.0f;
        for (const auto& rating_entry : ratings) {
            sum_of_squares += rating_entry.second * rating_entry.second;
        }
        norms_results_vec[i] = {user_id, std::sqrt(sum_of_squares)};
    }
    norms_map.reserve(norms_results_vec.size());
    for (const auto& pair : norms_results_vec) {
        norms_map[pair.first] = pair.second;
    }
    return norms_map;
}

float calculateCosineSimilarity(const std::unordered_map<int, float>& ratings_user_a,
                                const std::unordered_map<int, float>& ratings_user_b,
                                float norm_user_a,
                                float norm_user_b) {
    if (norm_user_a == 0.0f || norm_user_b == 0.0f) {
        return 0.0f;
    }
    float dot_product = 0.0f;
    if (ratings_user_a.size() < ratings_user_b.size()) {
        for (const auto& rating_entry_a : ratings_user_a) {
            auto it_b = ratings_user_b.find(rating_entry_a.first);
            if (it_b != ratings_user_b.end()) {
                dot_product += rating_entry_a.second * it_b->second;
            }
        }
    } else {
        for (const auto& rating_entry_b : ratings_user_b) {
            auto it_a = ratings_user_a.find(rating_entry_b.first);
            if (it_a != ratings_user_a.end()) {
                dot_product += it_a->second * rating_entry_b.second;
            }
        }
    }
    if (dot_product == 0.0f) return 0.0f;
    return dot_product / (norm_user_a * norm_user_b);
}

// --- LSH Implementations ---

HyperplaneSet generateSingleHyperplaneSet(int num_hyperplanes, int dimensionality, std::mt19937& rng) {
    HyperplaneSet hyperplane_set;
    hyperplane_set.reserve(num_hyperplanes);
    std::normal_distribution<float> distribution(0.0, 1.0); // Distribuição normal padrão

    for (int i = 0; i < num_hyperplanes; ++i) {
        Hyperplane plane(dimensionality);
        for (int j = 0; j < dimensionality; ++j) {
            plane[j] = distribution(rng);
        }
        hyperplane_set.push_back(plane);
    }
    return hyperplane_set;
}

LSHHashValue computeLSHHash(const std::unordered_map<int, float>& user_ratings,
                            const HyperplaneSet& hyperplane_set,
                            const MovieIdToDenseIdxMap& movie_to_idx) {
    LSHHashValue hash = 0;
    if (hyperplane_set.empty()) return hash; // Nenhum hiperplano, hash 0

    // Verifica se NUM_HYPERPLANES_PER_TABLE (tamanho de hyperplane_set) excede os bits de LSHHashValue
    if (hyperplane_set.size() > sizeof(LSHHashValue) * 8) {
        std::cerr << "Error: Number of hyperplanes exceeds bits in LSHHashValue type!" << std::endl;
        // Lida com o erro, talvez truncando ou usando um tipo de hash maior se necessário.
        // Por agora, vamos prosseguir, mas isso é uma verificação importante.
    }

    for (size_t i = 0; i < hyperplane_set.size(); ++i) {
        const auto& plane = hyperplane_set[i];
        float dot_product = 0.0f;
        for (const auto& rating_entry : user_ratings) {
            int movie_id = rating_entry.first;
            float rating = rating_entry.second;
            
            auto it_idx = movie_to_idx.find(movie_id);
            if (it_idx != movie_to_idx.end()) {
                int dense_idx = it_idx->second;
                if (dense_idx < static_cast<int>(plane.size())) { // Checagem de limite
                    dot_product += rating * plane[dense_idx];
                }
            }
        }
        if (dot_product >= 0) {
            hash |= (1ULL << i); // Define o i-ésimo bit
        }
    }
    return hash;
}

void buildLSHTables(const UserItemMatrix& user_item_matrix,
                    const std::vector<HyperplaneSet>& all_hyperplane_sets,
                    const MovieIdToDenseIdxMap& movie_to_idx,
                    std::vector<LSHBucketMap>& lsh_tables) {
    if (all_hyperplane_sets.empty()) return;

    lsh_tables.assign(all_hyperplane_sets.size(), LSHBucketMap());

    // Converte user_item_matrix para um vetor para paralelização mais fácil
    std::vector<std::pair<int, const std::unordered_map<int, float>*>> user_matrix_vec;
    user_matrix_vec.reserve(user_item_matrix.size());
    for(const auto& entry : user_item_matrix) {
        user_matrix_vec.push_back({entry.first, &entry.second});
    }

    // Paraleliza a construção de cada tabela LSH individualmente,
    // ou paraleliza o loop sobre os usuários para todas as tabelas.
    // Paralelizar o loop sobre usuários parece mais direto aqui.
    // Cada thread precisará de acesso de escrita sincronizado a lsh_tables.
    // Alternativamente, cada thread preenche suas próprias tabelas LSH e depois mescla.
    // Para simplificar, vamos paralelizar a iteração do usuário e usar locks para inserções de bucket.
    // No entanto, isso pode levar a contenção.
    // Um padrão melhor: cada thread calcula hashes e os armazena temporariamente com user_id,
    // depois, uma fase sequencial ou mais cuidadosamente paralelizada preenche os buckets.

    // Vamos tentar uma abordagem onde cada thread calcula hashes para um subconjunto de usuários
    // e os adiciona às tabelas LSH (que precisam ser thread-safe ou preenchidas sequencialmente depois).

    // Para este exemplo, vamos iterar sobre as tabelas LSH sequencialmente
    // e paralelizar a inserção de usuários em cada tabela.
    for (size_t table_idx = 0; table_idx < all_hyperplane_sets.size(); ++table_idx) {
        const auto& current_hyperplane_set = all_hyperplane_sets[table_idx];
        LSHBucketMap& current_bucket_map = lsh_tables[table_idx];

        // Estrutura temporária para armazenar (hash, user_id) para evitar locks no loop principal
        std::vector<std::vector<std::pair<LSHHashValue, int>>> thread_local_hashes(omp_get_max_threads());

        #pragma omp parallel for schedule(dynamic)
        for (size_t i = 0; i < user_matrix_vec.size(); ++i) {
            int user_id = user_matrix_vec[i].first;
            const auto& user_ratings = *(user_matrix_vec[i].second);
            LSHHashValue hash = computeLSHHash(user_ratings, current_hyperplane_set, movie_to_idx);
            
            int thread_id = 0;
            #ifdef _OPENMP
            thread_id = omp_get_thread_num();
            #endif
            thread_local_hashes[thread_id].emplace_back(hash, user_id);
        }

        // Fase sequencial para preencher o bucket_map a partir dos resultados das threads
        for (const auto& local_hashes : thread_local_hashes) {
            for (const auto& hash_user_pair : local_hashes) {
                current_bucket_map[hash_user_pair.first].push_back(hash_user_pair.second);
            }
        }
    }
}


NeighborList findApproximateKNearestNeighborsLSH(
    int target_user_id,
    const UserItemMatrix& user_item_matrix,
    const UserNormsMap& user_norms,
    const std::vector<HyperplaneSet>& all_hyperplane_sets,
    const std::vector<LSHBucketMap>& lsh_tables,
    const MovieIdToDenseIdxMap& movie_to_idx,
    int K) {

    auto it_target_user_ratings = user_item_matrix.find(target_user_id);
    if (it_target_user_ratings == user_item_matrix.end()) {
        std::cerr << "Warning: Target user " << target_user_id << " not found for LSH KNN." << std::endl;
        return {};
    }
    const auto& target_ratings = it_target_user_ratings->second;
    float target_norm = user_norms.at(target_user_id);

    std::set<int> candidate_user_ids; // Usar std::set para obter candidatos únicos automaticamente

    for (size_t table_idx = 0; table_idx < lsh_tables.size(); ++table_idx) {
        LSHHashValue target_hash = computeLSHHash(target_ratings, all_hyperplane_sets[table_idx], movie_to_idx);
        
        const auto& current_bucket_map = lsh_tables[table_idx];
        auto bucket_it = current_bucket_map.find(target_hash);
        if (bucket_it != current_bucket_map.end()) {
            for (int candidate_id : bucket_it->second) {
                if (candidate_id != target_user_id) {
                    candidate_user_ids.insert(candidate_id);
                }
            }
        }
        // Opcional: Multi-probe LSH - verifica buckets vizinhos (hashes com pequena distância de Hamming)
    }

    NeighborList potential_neighbors;
    for (int candidate_id : candidate_user_ids) {
        auto it_candidate_ratings = user_item_matrix.find(candidate_id);
        if (it_candidate_ratings != user_item_matrix.end()) {
            float candidate_norm = user_norms.at(candidate_id);
            float similarity = calculateCosineSimilarity(target_ratings, it_candidate_ratings->second, target_norm, candidate_norm);
            if (similarity > 0.0f) { // Considera apenas similaridade positiva
                potential_neighbors.emplace_back(candidate_id, similarity);
            }
        }
    }

    std::sort(potential_neighbors.begin(), potential_neighbors.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    if (potential_neighbors.size() > static_cast<size_t>(K)) {
        potential_neighbors.resize(K);
    }
    return potential_neighbors;
}

RecommendationList generateRecommendationsLSH(
    int target_user_id,
    int K_neighbors_for_recs,
    const UserItemMatrix& all_user_ratings,
    const UserNormsMap& user_norms,
    const std::vector<HyperplaneSet>& all_hyperplane_sets,
    const std::vector<LSHBucketMap>& lsh_tables,
    const MovieIdToDenseIdxMap& movie_to_idx) {
    
    NeighborList k_approx_neighbors = findApproximateKNearestNeighborsLSH(
        target_user_id, all_user_ratings, user_norms, 
        all_hyperplane_sets, lsh_tables, movie_to_idx, K_neighbors_for_recs);

    if (k_approx_neighbors.empty()) {
        return {};
    }

    auto it_target_ratings_map = all_user_ratings.find(target_user_id);
    if (it_target_ratings_map == all_user_ratings.end()) {
        return {}; // Usuário alvo não encontrado
    }
    const auto& target_user_seen_movies = it_target_ratings_map->second;

    std::unordered_map<int, float> movie_weighted_score_sum;
    std::unordered_map<int, float> movie_similarity_sum;

    for (const auto& neighbor_pair : k_approx_neighbors) {
        int neighbor_id = neighbor_pair.first;
        float similarity_score = neighbor_pair.second;

        const auto& neighbor_ratings = all_user_ratings.at(neighbor_id);

        for (const auto& movie_rating_entry : neighbor_ratings) {
            int movie_id = movie_rating_entry.first;
            float rating = movie_rating_entry.second;

            if (target_user_seen_movies.count(movie_id)) {
                continue;
            }
            movie_weighted_score_sum[movie_id] += rating * similarity_score;
            movie_similarity_sum[movie_id] += similarity_score;
        }
    }

    RecommendationList recommendations;
    for (const auto& score_entry : movie_weighted_score_sum) {
        int movie_id = score_entry.first;
        float total_weighted_score = score_entry.second;
        float total_similarity = movie_similarity_sum.at(movie_id);

        if (total_similarity > 0.0f) {
            float predicted_rating = total_weighted_score / total_similarity;
            recommendations.emplace_back(movie_id, predicted_rating);
        }
    }

    std::sort(recommendations.begin(), recommendations.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    return recommendations;
}