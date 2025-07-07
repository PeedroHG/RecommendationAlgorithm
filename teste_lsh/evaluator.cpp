#include "evaluator.hpp"
#include <iostream>

Evaluator::Evaluator(const UserRatingsLog& test_ratings) : total_users(0), hits(0) {
    for (const auto& [user_id, ratings_vec] : test_ratings) {
        for (const auto& [movie_id, _] : ratings_vec) {
            user_ratings[user_id].insert(movie_id);
        }
    }
}

void Evaluator::update_hit_rate(const std::vector<int>& recommendations, int user_id) {
    total_users++;
    if (user_ratings.count(user_id)) {
        for (int recommended_movie : recommendations) {
            if (user_ratings.at(user_id).count(recommended_movie)) {
                hits++;
                break;
            }
        }
    }
}

void Evaluator::update_hit_rate_neighbors(const std::vector<int>& recommendations, const std::vector<int>& neighbor_ids, const UserRatingsLog& ratings_log) {
    total_users++;
    std::unordered_set<int> neighbor_movies;
    for (int neighbor_id : neighbor_ids) {
        if (ratings_log.count(neighbor_id)) {
            for (const auto& pair : ratings_log.at(neighbor_id)) {
                neighbor_movies.insert(pair.first);
            }
        }
    }
    for (int recommended_movie : recommendations) {
        if (neighbor_movies.count(recommended_movie)) {
            hits++;
            break;
        }
    }
}

double Evaluator::get_hit_rate() const {
    if (total_users == 0) return 0.0;
    return static_cast<double>(hits) / total_users;
}
