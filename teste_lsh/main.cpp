#include "config.hpp"
#include "types.hpp"
#include "csv_parser.hpp"
#include "recommender_engine.hpp"
//#include "evaluator.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>    
#include <iomanip>   
#include <random>   // Para std::mt19937
#include <set>      // Para coletar movie_ids únicos

#ifdef _OPENMP
#include <omp.h>
#endif

int main() {
    auto program_start_time = std::chrono::high_resolution_clock::now();

    #ifdef _OPENMP
        std::cout << "OpenMP está ATIVADO. Número máximo de threads: " << omp_get_max_threads() << std::endl;
    #else
        std::cout << "OpenMP NÃO está ativado/disponível." << std::endl;
    #endif

    std::cout << "LSH Movie Recommender System Initializing..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // --- 1. Data Loading and Preprocessing ---
    std::cout << "\n[Phase 1: Data Loading and Preprocessing]" << std::endl;
    auto phase_start_time = std::chrono::high_resolution_clock::now();

    UserRatingsLog raw_users_ratings; 
    std::cout << "Reading ratings from: " << RATINGS_CSV_PATH << std::endl;
    readRatingsCSV(RATINGS_CSV_PATH, raw_users_ratings);
    std::cout << "Initial ratings loaded for " << raw_users_ratings.size() << " users." << std::endl;

    MovieTitlesMap movie_titles;
    std::cout << "Reading movie titles from: " << MOVIES_CSV_PATH << std::endl;
    movie_titles = readMovieTitles(MOVIES_CSV_PATH);
    std::cout << "Loaded " << movie_titles.size() << " movie titles." << std::endl;

    std::unordered_map<int, int> user_rating_counts, movie_rating_counts;
    countEntityRatings(raw_users_ratings, user_rating_counts, movie_rating_counts);

    std::unordered_set<int> valid_user_ids, valid_movie_ids_set; // Renomeado para evitar conflito
    identifyValidEntities(user_rating_counts, movie_rating_counts,
                          valid_user_ids, valid_movie_ids_set, MIN_RATINGS_PER_ENTITY);
    std::cout << "Filtering entities: Keeping users/movies with at least " << MIN_RATINGS_PER_ENTITY << " ratings." << std::endl;
    std::cout << "Valid users found: " << valid_user_ids.size() << ", Valid movies found: " << valid_movie_ids_set.size() << std::endl;

    UserRatingsLog filtered_users_ratings = raw_users_ratings; 
    filterUserRatingsLog(filtered_users_ratings, valid_user_ids, valid_movie_ids_set);
    std::cout << "Filtered dataset contains " << filtered_users_ratings.size() << " users." << std::endl;

    std::cout << "Writing filtered dataset to: " << FILTERED_DATASET_PATH << std::endl;
    writeFilteredRatingsToFile(FILTERED_DATASET_PATH, filtered_users_ratings);

    std::cout << "Writing random users to explore to: " << EXPLORE_USERS_PATH << std::endl;
    writeRandomUserIdsToExplore(filtered_users_ratings, NUM_RANDOM_USERS_TO_EXPLORE, EXPLORE_USERS_PATH);

    auto phase_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> phase_elapsed = phase_end_time - phase_start_time;
    std::cout << "Phase 1 completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;

    // --- 2. Building Recommendation Model (User-Item Matrix, Norms) ---
    std::cout << "\n[Phase 2: Building Core Model Components]" << std::endl;
    phase_start_time = std::chrono::high_resolution_clock::now();

    if (filtered_users_ratings.empty()) {
        std::cerr << "Error: No data available after filtering. Cannot proceed." << std::endl;
        return 1;
    }

    std::cout << "Converting data to user-item matrix format..." << std::endl;
    UserItemMatrix user_item_matrix = convertToUserItemMatrix(filtered_users_ratings);

    std::cout << "Computing user norms..." << std::endl;
    UserNormsMap user_norms = computeUserNorms(user_item_matrix);
    std::cout << "Norms computed for " << user_norms.size() << " users." << std::endl;

    phase_end_time = std::chrono::high_resolution_clock::now();
    phase_elapsed = phase_end_time - phase_start_time;
    std::cout << "Phase 2 completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;

    // --- 2.5 Building LSH Model ---
    std::cout << "\n[Phase 2.5: Building LSH Model]" << std::endl;
    phase_start_time = std::chrono::high_resolution_clock::now();

    MovieIdToDenseIdxMap movie_to_idx;
    DenseIdxToMovieIdVec idx_to_movie; // Opcional, para referência
    std::set<int> unique_movie_ids_in_matrix;
    for(const auto& user_entry : user_item_matrix) {
        for(const auto& movie_entry : user_entry.second) {
            unique_movie_ids_in_matrix.insert(movie_entry.first);
        }
    }
    int dense_idx_counter = 0;
    for(int movie_id : unique_movie_ids_in_matrix) {
        movie_to_idx[movie_id] = dense_idx_counter++;
        idx_to_movie.push_back(movie_id);
    }
    int dimensionality = unique_movie_ids_in_matrix.size();
    std::cout << "LSH dimensionality (unique movies in matrix): " << dimensionality << std::endl;
    
    if (dimensionality == 0 && !user_item_matrix.empty()) {
         std::cerr << "Error: Dimensionality is 0 but user_item_matrix is not empty. Check movie ID collection." << std::endl;
        return 1;
    }
     if (dimensionality == 0 && user_item_matrix.empty()) {
        std::cout << "Warning: Dimensionality is 0 because dataset is empty after filtering. Skipping LSH." << std::endl;
    }


    std::vector<HyperplaneSet> all_hyperplane_sets(NUM_LSH_TABLES);
    std::vector<LSHBucketMap> lsh_tables(NUM_LSH_TABLES);

    if (dimensionality > 0) {
        std::mt19937 rng(std::chrono::system_clock::now().time_since_epoch().count()); // Seed RNG
        std::cout << "Generating " << NUM_LSH_TABLES << " LSH hyperplane sets ("
                << NUM_HYPERPLANES_PER_TABLE << " hyperplanes/table)..." << std::endl;
        for (int i = 0; i < NUM_LSH_TABLES; ++i) {
            all_hyperplane_sets[i] = generateSingleHyperplaneSet(NUM_HYPERPLANES_PER_TABLE, dimensionality, rng);
        }

        std::cout << "Building " << NUM_LSH_TABLES << " LSH tables..." << std::endl;
        buildLSHTables(user_item_matrix, all_hyperplane_sets, movie_to_idx, lsh_tables);
    }


    phase_end_time = std::chrono::high_resolution_clock::now();
    phase_elapsed = phase_end_time - phase_start_time;
    std::cout << "Phase 2.5 (LSH Model) completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;


    // --- 3. Generating Recommendations ---
    std::cout << "\n[Phase 3: Generating LSH Recommendations]" << std::endl;
    phase_start_time = std::chrono::high_resolution_clock::now();

    std::ifstream exploreFile(EXPLORE_USERS_PATH);
    if (!exploreFile.is_open()) {
        std::cerr << "Error: Could not open explore users file: " << EXPLORE_USERS_PATH << std::endl;
    }

    std::ofstream recommendationsOutputFile(OUTPUT_RECOMMENDATIONS_PATH);
    if (!recommendationsOutputFile.is_open()) {
        std::cerr << "Error: Could not create output recommendations file: " << OUTPUT_RECOMMENDATIONS_PATH << std::endl;
        return 1;
    }
    
    recommendationsOutputFile << "LSH Movie Recommendations (Top " << TOP_N_RECOMMENDATIONS 
                              << " using K_approx=" << K_NEIGHBORS 
                              << ", L=" << NUM_LSH_TABLES << ", k_hash=" << NUM_HYPERPLANES_PER_TABLE << ")\n";
    recommendationsOutputFile << "-----------------------------------------------------------\n\n";

    int target_user_id_to_recommend;
    int users_processed_for_recommendation = 0;
    if (exploreFile.is_open()) {
         std::cout << "Generating LSH recommendations for users in " << EXPLORE_USERS_PATH << "..." << std::endl;
        while (exploreFile >> target_user_id_to_recommend) {
            users_processed_for_recommendation++;
            recommendationsOutputFile << "User ID: " << target_user_id_to_recommend << "\n";

            if (user_item_matrix.find(target_user_id_to_recommend) == user_item_matrix.end() || dimensionality == 0) {
                recommendationsOutputFile << "  User not found or LSH model not built (empty dataset/0-dim).\n\n";
                std::cout << "  User " << target_user_id_to_recommend << " not found or LSH model not ready." << std::endl;
                continue;
            }
            
            RecommendationList recommendations = generateRecommendationsLSH(
                target_user_id_to_recommend, K_NEIGHBORS, user_item_matrix, user_norms,
                all_hyperplane_sets, lsh_tables, movie_to_idx);

            if (recommendations.empty()) {
                recommendationsOutputFile << "  No LSH recommendations could be generated for this user.\n\n";
            } else {
                recommendationsOutputFile << "  Recommended Movies (MovieID: Score | Title):\n";
                int count = 0;
                for (const auto& rec : recommendations) {
                    if (count++ >= TOP_N_RECOMMENDATIONS) break;
                    recommendationsOutputFile << "  - " << rec.first << ": " << std::fixed << std::setprecision(3) << rec.second;
                    if (movie_titles.count(rec.first)) {
                        recommendationsOutputFile << " | " << movie_titles.at(rec.first);
                    } else {
                        recommendationsOutputFile << " | (Title not found)";
                    }
                    recommendationsOutputFile << "\n";
                }
                recommendationsOutputFile << "\n";
            }
            //  std::cout << "  Generated LSH recommendations for User ID: " << target_user_id_to_recommend << std::endl;
        }
        exploreFile.close();
    } else {
        std::cout << "Explore file (" << EXPLORE_USERS_PATH << ") not found. No specific user LSH recommendations generated." << std::endl;
    }
    std::cout << users_processed_for_recommendation << " users processed from explore file for LSH recommendations." << std::endl;
    std::cout << "LSH Recommendations written to: " << OUTPUT_RECOMMENDATIONS_PATH << std::endl;

    phase_end_time = std::chrono::high_resolution_clock::now();
    phase_elapsed = phase_end_time - phase_start_time;
    std::cout << "Phase 3 (LSH Recommendations) completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;

    


    // --- Total Execution Time ---
    auto program_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_elapsed_time = program_end_time - program_start_time;
    std::cout << "\n----------------------------------------" << std::endl;
    std::cout << "Total execution time: " << std::fixed << std::setprecision(2) << total_elapsed_time.count() << " seconds." << std::endl;
    std::cout << "LSH Movie Recommender System Finished." << std::endl;

    return 0;
}