#ifndef CONFIG_HPP
#define CONFIG_HPP

// alias for the data structure used to store user ratings
using UsersData = std::unordered_map<int, std::vector<std::pair<int, float>>>;

// Values for the configuration of the recommender system
constexpr int min_rating = 50;
constexpr int max_recomendations_users = 5;
constexpr int max_similar_neighbor = 5;
constexpr size_t users_explore  = 50;

// Path to the dataset and output files
constexpr const char* ratings_path = "dataset/ratings.csv";
constexpr const char* movies_path = "dataset/movies.csv";
constexpr const char* explore_path = "dataset/explore.dat";
constexpr const char* filtred_dataset_path = "dataset/input.dat";
//constexpr const char* recommendations_path = "outcome/recommendations.dat";

#endif // CONFIG_HPP