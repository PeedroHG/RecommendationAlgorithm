#include "config.hpp"
#include "types.hpp"
#include "csv_parser.hpp"
#include "recommender_engine.hpp"
#include "evaluator.hpp"
#include "../include/cossineSimilarity.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>    
#include <iomanip>   
#include <random>   // Para std::mt19937
#include <set>      // Para coletar movie_ids únicos
#include <unordered_set>

#ifdef _OPENMP
#include <omp.h>
#endif

int main() {
    auto program_start_time = std::chrono::high_resolution_clock::now();

    #ifdef _OPENMP
        //std::cout << "OpenMP está ATIVADO. Número máximo de threads: " << omp_get_max_threads() -  1 << std::endl;
    #else
        //std::cout << "OpenMP NÃO está ativado/disponível." << std::endl;
    #endif

    //std::cout << "LSH Movie Recommender System Initializing..." << std::endl;
    //std::cout << "----------------------------------------" << std::endl;

    // --- 1. Data Loading and Preprocessing ---
    //std::cout << "\n[Phase 1: Data Loading and Preprocessing]" << std::endl;
    auto phase_start_time = std::chrono::high_resolution_clock::now();
    UserRatingsLog raw_users_ratings; 
    //std::cout << "Reading ratings from: " << RATINGS_CSV_PATH << std::endl;
    readRatingsCSV(RATINGS_CSV_PATH, raw_users_ratings);
    //std::cout << "Initial ratings loaded for " << raw_users_ratings.size() << " users." << std::endl;

    MovieTitlesMap movie_titles;
    //std::cout << "Reading movie titles from: " << MOVIES_CSV_PATH << std::endl;
    movie_titles = readMovieTitles(MOVIES_CSV_PATH);
    //std::cout << "Loaded " << movie_titles.size() << " movie titles." << std::endl;

    std::unordered_map<int, int> user_rating_counts, movie_rating_counts;
    countEntityRatings(raw_users_ratings, user_rating_counts, movie_rating_counts);

    std::unordered_set<int> valid_user_ids, valid_movie_ids_set; // Renomeado para evitar conflito
    identifyValidEntities(user_rating_counts, movie_rating_counts,
                          valid_user_ids, valid_movie_ids_set, MIN_RATINGS_PER_ENTITY);
    //std::cout << "Filtering entities: Keeping users/movies with at least " << MIN_RATINGS_PER_ENTITY << " ratings." << std::endl;
    //std::cout << "Valid users found: " << valid_user_ids.size() << ", Valid movies found: " << valid_movie_ids_set.size() << std::endl;

    UserRatingsLog filtered_users_ratings = raw_users_ratings; 
    filterUserRatingsLog(filtered_users_ratings, valid_user_ids, valid_movie_ids_set);
    //std::cout << "Filtered dataset contains " << filtered_users_ratings.size() << " users." << std::endl;

    //std::cout << "Writing filtered dataset to: " << FILTERED_DATASET_PATH << std::endl;
    writeFilteredRatingsToFile(FILTERED_DATASET_PATH, filtered_users_ratings);

    //std::cout << "Writing random users to explore to: " << EXPLORE_USERS_PATH << std::endl;
    //writeRandomUserIdsToExplore(filtered_users_ratings, NUM_RANDOM_USERS_TO_EXPLORE, EXPLORE_USERS_PATH);

    auto phase_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> phase_elapsed = phase_end_time - phase_start_time;
    //std::cout << "Phase 1 completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;

    // --- 2. Building Recommendation Model (User-Item Matrix, Norms) ---
    //std::cout << "\n[Phase 2: Building Core Model Components]" << std::endl;
    phase_start_time = std::chrono::high_resolution_clock::now();

    if (filtered_users_ratings.empty()) {
        std::cerr << "Error: No data available after filtering. Cannot proceed." << std::endl;
        return 1;
    }

    //std::cout << "Converting data to user-item matrix format..." << std::endl;
    UserItemMatrix user_item_matrix = convertToUserItemMatrix(filtered_users_ratings);

    //std::cout << "Computing user norms..." << std::endl;
    UserNormsMap user_norms = computeUserNorms(user_item_matrix);
    //std::cout << "Norms computed for " << user_norms.size() << " users." << std::endl;

    phase_end_time = std::chrono::high_resolution_clock::now();
    phase_elapsed = phase_end_time - phase_start_time;
    //std::cout << "Phase 2 completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;

    // --- 2.5 Building LSH Model ---
    //std::cout << "\n[Phase 2.5: Building LSH Model]" << std::endl;
    phase_start_time = std::chrono::high_resolution_clock::now();

    MovieIdToDenseIdxMap movie_to_idx;
    std::set<int> unique_movie_ids_in_matrix;
    for(const auto& user_entry : user_item_matrix) {
        for(const auto& movie_entry : user_entry.second) {
            unique_movie_ids_in_matrix.insert(movie_entry.first);
        }
    }
    int dense_idx_counter = 0;
    for(int movie_id : unique_movie_ids_in_matrix) {
        movie_to_idx[movie_id] = dense_idx_counter++;
    }
    int dimensionality = unique_movie_ids_in_matrix.size();
    //std::cout << "LSH dimensionality (unique movies in matrix): " << dimensionality << std::endl;
    
    if (dimensionality == 0 && !user_item_matrix.empty()) {
         std::cerr << "Error: Dimensionality is 0 but user_item_matrix is not empty. Check movie ID collection." << std::endl;
        return 1;
    }


    std::vector<HyperplaneSet> all_hyperplane_sets(NUM_LSH_TABLES);
    std::vector<LSHBucketMap> lsh_tables(NUM_LSH_TABLES);

    if (dimensionality > 0) {
        //std::mt19937 rng(std::chrono::system_clock::now().time_since_epoch().count()); // Seed RNG
        std::mt19937 rng(42); // Seed fixa para reprodutibilidade
        //std::cout << "Generating " << NUM_LSH_TABLES << " LSH hyperplane sets ("
        //        << NUM_HYPERPLANES_PER_TABLE << " hyperplanes/table)..." << std::endl;
        for (int i = 0; i < NUM_LSH_TABLES; ++i) {
            all_hyperplane_sets[i] = generateSingleHyperplaneSet(NUM_HYPERPLANES_PER_TABLE, dimensionality, rng);
        }

        //std::cout << "Building " << NUM_LSH_TABLES << " LSH tables..." << std::endl;
        buildLSHTables(user_item_matrix, all_hyperplane_sets, movie_to_idx, lsh_tables);
    }
    

    phase_end_time = std::chrono::high_resolution_clock::now();
    phase_elapsed = phase_end_time - phase_start_time;
    //std::cout << "Phase 2.5 (LSH Model) completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;


    // --- 3. Generating Recommendations ---
    //std::cout << "\n[Phase 3: Generating LSH Recommendations]" << std::endl;
    phase_start_time = std::chrono::high_resolution_clock::now();

    // Carregar ground truth para avaliação (usando o dataset filtrado como exemplo)
    //Evaluator evaluator(filtered_users_ratings);

    // Carregar IDs de usuários para processar
    std::vector<int> explore_user_ids = loadExploreUserIds(EXPLORE_USERS_PATH);
    if (explore_user_ids.empty()) {
        std::cerr << "Error: No users to process. Check explore file." << std::endl;
        return 1;
    }

    // Preparar arquivo de saída
    std::ofstream recommendationsOutputFile = prepareRecommendationsOutput(
        OUTPUT_RECOMMENDATIONS_PATH, TOP_N_RECOMMENDATIONS, K_NEIGHBORS, 
        NUM_LSH_TABLES, NUM_HYPERPLANES_PER_TABLE);
    
    if (!recommendationsOutputFile.is_open()) {
        std::cerr << "Error: Could not create output file." << std::endl;
        return 1;
    }

    // Gerar recomendações para todos os usuários
    //std::cout << "Generating LSH recommendations for " << explore_user_ids.size() 
    //          << " users in " << EXPLORE_USERS_PATH << "..." << std::endl;
    
    std::vector<std::string> user_outputs = generateRecommendationsForUsers(
        explore_user_ids, user_item_matrix, user_norms,
        all_hyperplane_sets, lsh_tables, movie_to_idx, movie_titles,
        K_NEIGHBORS, TOP_N_RECOMMENDATIONS);

    // Escrever resultados no arquivo
    for (const auto& out : user_outputs) {
        recommendationsOutputFile << out;
    }

    //std::cout << "LSH Recommendations written to: " << OUTPUT_RECOMMENDATIONS_PATH << std::endl;

    phase_end_time = std::chrono::high_resolution_clock::now();
    phase_elapsed = phase_end_time - phase_start_time;
    //std::cout << "Phase 3 (LSH Recommendations) completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;

   
    // --- Total Execution Time ---
    auto program_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_elapsed_time = program_end_time - program_start_time;
    //std::cout << "\n----------------------------------------" << std::endl;
    std::cout << "Total execution time: " << std::fixed << std::setprecision(2) << total_elapsed_time.count() << " seconds." << std::endl;
    //std::cout << "LSH Movie Recommender System Finished." << std::endl;

    return 0;
}