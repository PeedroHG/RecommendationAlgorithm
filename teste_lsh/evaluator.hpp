#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "types.hpp"
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>

class Evaluator {
public:
    Evaluator(const UserRatingsLog& test_ratings);
    void update_hit_rate(const std::vector<int>& recommendations, int user_id);
    void update_hit_rate_neighbors(const std::vector<int>& recommendations, const std::vector<int>& neighbor_ids, const UserRatingsLog& ratings_log);
    double get_hit_rate() const;

private:
    std::unordered_map<int, std::unordered_set<int>> user_ratings;
    int total_users;
    int hits;
};

#endif // EVALUATOR_HPP