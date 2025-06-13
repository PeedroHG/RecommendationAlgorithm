#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <cstddef> // Para size_t

// File paths (ajuste estes para os locais reais dos seus arquivos)
const std::string RATINGS_CSV_PATH = "../dataset/ratings.csv";
const std::string MOVIES_CSV_PATH = "../dataset/movies.csv";
const std::string FILTERED_DATASET_PATH = "filtered_dataset.dat";
const std::string EXPLORE_USERS_PATH = "explore.dat";
const std::string OUTPUT_RECOMMENDATIONS_PATH = "output.dat";

// Recommendation & Filtering Parameters
const int MIN_RATINGS_PER_ENTITY = 5;
const int K_NEIGHBORS = 10; // K for final recommendation list from candidates
const size_t NUM_RANDOM_USERS_TO_EXPLORE = 1000;
const int TOP_N_RECOMMENDATIONS = 10;

// CSV Processing
const size_t CSV_READ_BUFFER_SIZE = 16 * 1024 * 1024;

// LSH Parameters
const int NUM_LSH_TABLES = 10;        // Number of hash tables (L)
const int NUM_HYPERPLANES_PER_TABLE = 16; // Number of bits in each hash (k)
                                         // Hash value will be uint64_t, so k <= 64

#endif // CONFIG_HPP