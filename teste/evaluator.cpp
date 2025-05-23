#include "evaluator.hpp"
#include "recommender_engine.hpp" 
#include <vector>
#include <cmath>
#include <numeric>
#include <iostream>
#include <algorithm>
#ifdef _OPENMP
#include <omp.h>
#endif

void evaluateKNNLeaveOneOut(const UserItemMatrix& user_item_matrix, int K_neighbors_for_eval) {
    std::vector<std::vector<float>> thread_local_squared_errors;
    std::vector<std::vector<float>> thread_local_absolute_errors;
    std::vector<long long> thread_local_predictions_made; 

    #ifdef _OPENMP
    int num_threads = omp_get_max_threads();
    thread_local_squared_errors.resize(num_threads);
    thread_local_absolute_errors.resize(num_threads);
    thread_local_predictions_made.assign(num_threads, 0);
    #else
    thread_local_squared_errors.resize(1);
    thread_local_absolute_errors.resize(1);
    thread_local_predictions_made.assign(1, 0);
    #endif

    UserNormsMap original_user_norms = computeUserNorms(user_item_matrix);

    std::vector<std::pair<int, const std::unordered_map<int, float>*>> user_matrix_vec;
    user_matrix_vec.reserve(user_item_matrix.size());
    for (const auto& entry : user_item_matrix) {
        user_matrix_vec.push_back({entry.first, &entry.second});
    }

    #pragma omp parallel
    {
        int current_thread_id = 0;
        #ifdef _OPENMP
        current_thread_id = omp_get_thread_num();
        #endif

        #pragma omp for schedule(dynamic)
        for (size_t i = 0; i < user_matrix_vec.size(); ++i) {
            int current_user_id = user_matrix_vec[i].first;
            const auto& current_user_all_ratings = *(user_matrix_vec[i].second);

            if (current_user_all_ratings.size() < 2) {
                continue;
            }

            for (const auto& test_item_entry : current_user_all_ratings) {
                int test_movie_id = test_item_entry.first;
                float actual_rating_for_test_movie = test_item_entry.second;

                std::unordered_map<int, float> current_user_ratings_copy = current_user_all_ratings;
                current_user_ratings_copy.erase(test_movie_id);

                if (current_user_ratings_copy.empty()) continue;

                float sum_sq_temp_profile = 0.0f;
                for (const auto& rating_pair : current_user_ratings_copy) {
                    sum_sq_temp_profile += rating_pair.second * rating_pair.second;
                }
                float norm_for_temp_profile = std::sqrt(sum_sq_temp_profile);

                NeighborList neighbors_for_test_item;
                for (const auto& other_user_entry : user_item_matrix) { 
                    int other_user_id = other_user_entry.first;
                    if (other_user_id == current_user_id) continue;

                    const auto& other_user_ratings = other_user_entry.second;
                    float norm_for_other_user = original_user_norms.at(other_user_id);

                    float similarity = calculateCosineSimilarity(current_user_ratings_copy,
                                                                 other_user_ratings,
                                                                 norm_for_temp_profile,
                                                                 norm_for_other_user);
                    if (similarity > 0.0f) {
                        neighbors_for_test_item.emplace_back(other_user_id, similarity);
                    }
                }

                std::sort(neighbors_for_test_item.begin(), neighbors_for_test_item.end(),
                          [](const auto& a, const auto& b) { return a.second > b.second; });

                if (neighbors_for_test_item.size() > static_cast<size_t>(K_neighbors_for_eval)) {
                    neighbors_for_test_item.resize(K_neighbors_for_eval);
                }

                if (neighbors_for_test_item.empty()) continue;

                float predicted_rating_numerator = 0.0f;
                float predicted_rating_denominator = 0.0f;

                for (const auto& neighbor : neighbors_for_test_item) {
                    int neighbor_id = neighbor.first;
                    float neighbor_similarity = neighbor.second;
                    const auto& neighbor_all_ratings = user_item_matrix.at(neighbor_id);

                    auto it_neighbor_rating = neighbor_all_ratings.find(test_movie_id);
                    if (it_neighbor_rating != neighbor_all_ratings.end()) {
                        predicted_rating_numerator += it_neighbor_rating->second * neighbor_similarity;
                        predicted_rating_denominator += neighbor_similarity;
                    }
                }

                if (predicted_rating_denominator > 0.0f) {
                    float predicted_rating = predicted_rating_numerator / predicted_rating_denominator;
                    float error = actual_rating_for_test_movie - predicted_rating;
                    thread_local_squared_errors[current_thread_id].push_back(error * error);
                    thread_local_absolute_errors[current_thread_id].push_back(std::abs(error));
                    thread_local_predictions_made[current_thread_id]++;
                }
            } 
        } 
    } 

    std::vector<float> final_squared_errors;
    std::vector<float> final_absolute_errors;
    long long final_total_predictions_made = 0;

    int num_threads_to_merge = 1;
    #ifdef _OPENMP
    num_threads_to_merge = omp_get_max_threads();
    #endif

    for(int i=0; i < num_threads_to_merge; ++i) {
        final_squared_errors.insert(final_squared_errors.end(),
                                    thread_local_squared_errors[i].begin(),
                                    thread_local_squared_errors[i].end());
        final_absolute_errors.insert(final_absolute_errors.end(),
                                     thread_local_absolute_errors[i].begin(),
                                     thread_local_absolute_errors[i].end());
        final_total_predictions_made += thread_local_predictions_made[i];
    }

    if (final_total_predictions_made == 0) {
        std::cout << "⚠️ Insufficient data or no predictions made during Leave-One-Out evaluation." << std::endl;
        return;
    }

    double sum_sq_err = std::accumulate(final_squared_errors.begin(), final_squared_errors.end(), 0.0);
    double sum_abs_err = std::accumulate(final_absolute_errors.begin(), final_absolute_errors.end(), 0.0);

    double rmse = std::sqrt(sum_sq_err / final_total_predictions_made);
    double mae = sum_abs_err / final_total_predictions_made;

    std::cout << "\n📊 K-NN Leave-One-Out Evaluation (K=" << K_neighbors_for_eval << ") [Parallel]:" << std::endl;
    std::cout << "➡️ RMSE (Root Mean Square Error): " << rmse << std::endl;
    std::cout << "➡️ MAE  (Mean Absolute Error):   " << mae << std::endl;
    std::cout << "🔢 Total predictions made:      " << final_total_predictions_made << std::endl;
}