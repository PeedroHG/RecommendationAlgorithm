#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstddef> // Para size_t

// File paths (ajuste estes para os locais reais dos seus arquivos)
#define RATINGS_CSV_PATH "../dataset/ratings.csv"
#define MOVIES_CSV_PATH "../dataset/movies.csv"
#define FILTERED_DATASET_PATH "filtered_dataset.dat"
#define EXPLORE_USERS_PATH "explore.dat"
#define OUTPUT_RECOMMENDATIONS_PATH "output.dat"

// Recommendation & Filtering Parameters
#define MIN_RATINGS_PER_ENTITY 5
#define K_NEIGHBORS 10
#define NUM_RANDOM_USERS_TO_EXPLORE 50
#define TOP_N_RECOMMENDATIONS 10

// CSV Processing
#define CSV_READ_BUFFER_SIZE (16 * 1024 * 1024)

#endif // CONFIG_HPP