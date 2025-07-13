#include "../include/recommender_engine.hpp"
#include "../include/config.hpp" // Para NUM_HYPERPLANES_PER_TABLE
#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>
#include <set> // Para coletar candidatos únicos
#include <fstream>
#include <sstream>
#include <iomanip>

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
    norms_map.reserve(user_item_matrix.size());

    std::vector<int> user_ids;
    user_ids.reserve(user_item_matrix.size());
    for (const auto& pair : user_item_matrix) {
        user_ids.push_back(pair.first);
    }

    std::vector<float> norms(user_ids.size());
    
    #pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < user_ids.size(); ++i) {
        const auto& ratings = user_item_matrix.at(user_ids[i]);
        float sum = 0.0f;
        for (const auto& r : ratings) sum += r.second * r.second;
        norms[i] = std::sqrt(sum);
    }

    for (size_t i = 0; i < user_ids.size(); ++i) {
        norms_map[user_ids[i]] = norms[i];
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
                if (dense_idx < static_cast<int>(plane.size())) {
                    dot_product += rating * plane[dense_idx];
                }
            }
        }
        if (dot_product >= 0) {
            hash |= (1ULL << i);
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

        int n_threads = 1;
        #ifdef _OPENMP
        n_threads = omp_get_max_threads();
        #endif
        std::vector<std::vector<std::pair<LSHHashValue, int>>> thread_local_hashes(n_threads);

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
    const auto& target_ratings = it_target_user_ratings->second; // O campo .second desse iterador contém o mapa de avaliações desse usuário, onde cada chave é o ID de um filme e o valor é a nota dada.
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
    std::vector<int> candidate_vec(candidate_user_ids.begin(), candidate_user_ids.end());
    std::vector<std::pair<int, float>> local_neighbors(candidate_vec.size());

#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < candidate_vec.size(); ++i) {
        int candidate_id = candidate_vec[i];
        auto it_candidate_ratings = user_item_matrix.find(candidate_id);
        if (it_candidate_ratings != user_item_matrix.end()) {
            float candidate_norm = user_norms.at(candidate_id);
            float similarity = calculateCosineSimilarity(target_ratings, it_candidate_ratings->second, target_norm, candidate_norm);
            if (similarity > 0.0f) {
                local_neighbors[i] = std::make_pair(candidate_id, similarity);
            } else {
                local_neighbors[i] = std::make_pair(-1, 0.0f);
            }
        } else {
            local_neighbors[i] = std::make_pair(-1, 0.0f);
        }
    }
    for (const auto& p : local_neighbors) {
        if (p.first != -1) {
            potential_neighbors.push_back(p);
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
    const MovieIdToDenseIdxMap& movie_to_idx,
    const NeighborList* precomputed_neighbors,
    float similarity_threshold,
    bool use_user_mean_filter
) {
    NeighborList k_approx_neighbors;
    if (precomputed_neighbors) {
        k_approx_neighbors = *precomputed_neighbors;
    } else {
        k_approx_neighbors = findApproximateKNearestNeighborsLSH(
            target_user_id, all_user_ratings, user_norms, 
            all_hyperplane_sets, lsh_tables, movie_to_idx, K_neighbors_for_recs);
    }
    if (k_approx_neighbors.empty()) {
        return {};
    }

    auto it_target_ratings_map = all_user_ratings.find(target_user_id);
    if (it_target_ratings_map == all_user_ratings.end()) {
        return {}; // Usuário alvo não encontrado
    }
    const auto& target_user_seen_movies = it_target_ratings_map->second;

    // Calcular média do usuário alvo
    float user_mean = 0.0f;
    if (!target_user_seen_movies.empty()) {
        float sum = 0.0f;
        for (const auto& p : target_user_seen_movies) sum += p.second;
        user_mean = sum / target_user_seen_movies.size();
    }

    int n_threads = 1;
    #ifdef _OPENMP
    n_threads = omp_get_max_threads();
    #endif
    std::vector<std::unordered_map<int, float>> movie_weighted_score_sum_local(n_threads);
    std::vector<std::unordered_map<int, float>> movie_similarity_sum_local(n_threads);

    #pragma omp parallel for schedule(dynamic)
    for (size_t idx = 0; idx < k_approx_neighbors.size(); ++idx) {
        int thread_id = 0;
        #ifdef _OPENMP
        thread_id = omp_get_thread_num();
        #endif
        int neighbor_id = k_approx_neighbors[idx].first;
        float similarity_score = k_approx_neighbors[idx].second;
        if (similarity_score < similarity_threshold) continue; // Ignorar vizinhos pouco similares
        const auto& neighbor_ratings = all_user_ratings.at(neighbor_id);
        for (const auto& movie_rating_entry : neighbor_ratings) {
            int movie_id = movie_rating_entry.first;
            float rating = movie_rating_entry.second;
            if (target_user_seen_movies.count(movie_id)) {
                continue;
            }
            movie_weighted_score_sum_local[thread_id][movie_id] += rating * similarity_score;
            movie_similarity_sum_local[thread_id][movie_id] += similarity_score;
        }
    }
    // Redução dos mapas locais para globais
    std::unordered_map<int, float> movie_weighted_score_sum;
    std::unordered_map<int, float> movie_similarity_sum;
    for (int t = 0; t < n_threads; ++t) {
        for (const auto& p : movie_weighted_score_sum_local[t]) {
            movie_weighted_score_sum[p.first] += p.second;
        }
        for (const auto& p : movie_similarity_sum_local[t]) {
            movie_similarity_sum[p.first] += p.second;
        }
    }
    RecommendationList recommendations;
    if (true) { // bloco principal
        for (const auto& score_entry : movie_weighted_score_sum) {
            int movie_id = score_entry.first;
            float total_weighted_score = score_entry.second;
            float total_similarity = movie_similarity_sum.at(movie_id);
            if (total_similarity > 1.0f) {
                float predicted_rating = total_weighted_score / total_similarity;
                if (!use_user_mean_filter || predicted_rating > user_mean) {
                    recommendations.emplace_back(movie_id, predicted_rating);
                }
            }
        }
        if (recommendations.empty()) {
            // Fallback: recomenda todos com total_similarity > 0
            for (const auto& score_entry : movie_weighted_score_sum) {
                int movie_id = score_entry.first;
                float total_weighted_score = score_entry.second;
                float total_similarity = movie_similarity_sum.at(movie_id);
                if (total_similarity > 0.0f) {
                    float predicted_rating = total_weighted_score / total_similarity;
                    recommendations.emplace_back(movie_id, predicted_rating);
                }
            }
        }
    }
    std::sort(recommendations.begin(), recommendations.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    return recommendations;
}

// --- Fase 3: Recommendation Generation Functions ---

std::vector<int> loadExploreUserIds(const std::string& file_path) {
    std::vector<int> user_ids;
    std::ifstream file(file_path);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open explore users file: " << file_path << std::endl;
        return user_ids;
    }
    
    int uid;
    while (file >> uid) {
        user_ids.push_back(uid);
    }
    
    file.close();
    return user_ids;
}

std::ofstream prepareRecommendationsOutput(const std::string& output_path, 
                                         int top_n, int k_neighbors, 
                                         int num_tables, int num_hyperplanes) {
    std::ofstream output_file(output_path);
    
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not create output recommendations file: " << output_path << std::endl;
        return output_file;
    }
    
    output_file << "LSH Movie Recommendations (Top " << top_n 
                << " using K_approx=" << k_neighbors 
                << ", L=" << num_tables << ", k_hash=" << num_hyperplanes << ")\n";
    output_file << "-----------------------------------------------------------\n\n";
    
    return output_file;
}

std::string processUserRecommendations(
    int target_user_id,
    const UserItemMatrix& user_item_matrix,
    const UserNormsMap& user_norms,
    const std::vector<HyperplaneSet>& all_hyperplane_sets,
    const std::vector<LSHBucketMap>& lsh_tables,
    const MovieIdToDenseIdxMap& movie_to_idx,
    const MovieTitlesMap& movie_titles,
    int k_neighbors,
    int top_n) {
    
    // Obter os k vizinhos mais próximos via LSH
    NeighborList neighbors = findApproximateKNearestNeighborsLSH(
        target_user_id, user_item_matrix, user_norms,
        all_hyperplane_sets, lsh_tables, movie_to_idx, k_neighbors);
    
    // Calcular a similaridade média dos vizinhos
    float mean_similarity = 0.0f;
    if (!neighbors.empty()) {
        float sum = 0.0f;
        for (const auto& p : neighbors) sum += p.second;
        mean_similarity = sum / neighbors.size();
    }
    
    // Gerar recomendações
    RecommendationList recommendations = generateRecommendationsLSH(
        target_user_id, k_neighbors, user_item_matrix, user_norms,
        all_hyperplane_sets, lsh_tables, movie_to_idx, &neighbors, 0.1f, true);
    
    // Formatar saída
    std::ostringstream oss;
    oss << "User ID: " << target_user_id << " | Similaridade Media: " 
        << std::fixed << std::setprecision(2) << mean_similarity << "\n";
    oss << "  Recommended Movies (MovieID: Score | Title):\n";
    
    int count = 0;
    for (const auto& rec : recommendations) {
        if (count++ >= top_n) break;
        
        int movie_id = rec.first;
        double score = rec.second;
        double score_percent = (score / 5.0) * 100.0;
        
        oss << "  - " << movie_id << ": " << std::fixed << std::setprecision(1) 
            << score_percent << "% | ";
        
        if (movie_titles.count(movie_id)) {
            oss << movie_titles.at(movie_id);
        } else {
            oss << "(Title not found)";
        }
        oss << "\n";
    }
    oss << "\n";
    
    return oss.str();
}

std::vector<std::string> generateRecommendationsForUsers(
    const std::vector<int>& explore_user_ids,
    const UserItemMatrix& user_item_matrix,
    const UserNormsMap& user_norms,
    const std::vector<HyperplaneSet>& all_hyperplane_sets,
    const std::vector<LSHBucketMap>& lsh_tables,
    const MovieIdToDenseIdxMap& movie_to_idx,
    const MovieTitlesMap& movie_titles,
    int k_neighbors,
    int top_n) {
    
    std::vector<std::string> user_outputs(explore_user_ids.size());
    
    #pragma omp parallel for schedule(dynamic)
    for (size_t idx = 0; idx < explore_user_ids.size(); ++idx) {
        int target_user_id = explore_user_ids[idx];
        user_outputs[idx] = processUserRecommendations(
            target_user_id, user_item_matrix, user_norms,
            all_hyperplane_sets, lsh_tables, movie_to_idx, movie_titles,
            k_neighbors, top_n);
    }
    
    return user_outputs;
}