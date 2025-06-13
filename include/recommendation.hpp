#ifndef RECOMMENDATION_HPP
#define RECOMMENDATION_HPP
#include "config.hpp"


void recommendation(
    const UsersData& usersData,
    const char* explore_path,
    const char* recommendations_path,
    int max_similar_neighbor,
    int max_recomendations_users
);
#endif // RECOMMENDATION_HPP