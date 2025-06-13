#include "recommender_engine.hpp"
#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>
#ifdef _OPENMP
#include <omp.h> // Inclui o cabeçalho OpenMP
#endif

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

NeighborList findKNearestNeighbors(int target_user_id,
                                   const UserItemMatrix& all_user_ratings,
                                   const UserNormsMap& all_user_norms,
                                   int K) {
    NeighborList all_potential_neighbors;

    auto it_target_ratings = all_user_ratings.find(target_user_id);
    auto it_target_norm = all_user_norms.find(target_user_id);

    if (it_target_ratings == all_user_ratings.end() || it_target_norm == all_user_norms.end()) {
        std::cerr << "Warning: Target user " << target_user_id << " not found in ratings or norms map for KNN." << std::endl;
        return {};
    }

    const auto& target_user_ratings_ref = it_target_ratings->second;
    float target_user_norm_val = it_target_norm->second;

    std::vector<std::pair<int, const std::unordered_map<int, float>*>> other_users_vec;
    other_users_vec.reserve(all_user_ratings.size());
    for (const auto& entry : all_user_ratings) {
        if (entry.first != target_user_id) { 
            other_users_vec.push_back({entry.first, &entry.second});
        }
    }
    
    std::vector<NeighborList> thread_local_neighbor_lists;
    #ifdef _OPENMP
    thread_local_neighbor_lists.resize(omp_get_max_threads());
    #else
    thread_local_neighbor_lists.resize(1); 
    #endif

    #pragma omp parallel 
    {
        int current_thread_id = 0;
        #ifdef _OPENMP
        current_thread_id = omp_get_thread_num();
        #endif

        #pragma omp for schedule(dynamic) 
        for (size_t i = 0; i < other_users_vec.size(); ++i) {
            int other_user_id = other_users_vec[i].first;
            const auto& other_user_ratings_ref = *(other_users_vec[i].second);

            auto it_other_norm = all_user_norms.find(other_user_id);
            if (it_other_norm == all_user_norms.end()) continue; 
            float other_user_norm_val = it_other_norm->second;

            float similarity = calculateCosineSimilarity(target_user_ratings_ref, other_user_ratings_ref,
                                                       target_user_norm_val, other_user_norm_val);

            if (similarity > 0.0f) {
                 thread_local_neighbor_lists[current_thread_id].emplace_back(other_user_id, similarity);
            }
        }
    } 

    for(const auto& local_list : thread_local_neighbor_lists) {
        all_potential_neighbors.insert(all_potential_neighbors.end(), local_list.begin(), local_list.end());
    }

    std::sort(all_potential_neighbors.begin(), all_potential_neighbors.end(),
              [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
                  return a.second > b.second;
              });

    if (all_potential_neighbors.size() > static_cast<size_t>(K)) {
        all_potential_neighbors.resize(K);
    }

    return all_potential_neighbors;
}

RecommendationList generateRecommendations(int target_user_id,
                                           int K_neighbors_count,
                                           const UserItemMatrix& all_user_ratings,
                                           const UserNormsMap& all_user_norms) {
    NeighborList k_nearest_neighbors = findKNearestNeighbors(target_user_id, all_user_ratings, all_user_norms, K_neighbors_count);

    if (k_nearest_neighbors.empty()) {
        return {};
    }

    auto it_target_ratings = all_user_ratings.find(target_user_id);
    if (it_target_ratings == all_user_ratings.end()) {
        std::cerr << "Error: Target user " << target_user_id << " ratings not found for generating recommendations." << std::endl;
        return {};
    }
    const auto& target_user_seen_movies = it_target_ratings->second;

    std::unordered_map<int, float> movie_weighted_score_sum;
    std::unordered_map<int, float> movie_similarity_sum;

    for (const auto& neighbor_pair : k_nearest_neighbors) {
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
              [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
                  return a.second > b.second;
              });

    return recommendations;
}