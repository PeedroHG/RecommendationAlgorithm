#include "config.hpp"
#include "types.hpp"
#include "csv_parser.hpp"
#include "recommender_engine.hpp"
#include "evaluator.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>    
#include <iomanip>   
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

    std::cout << "Movie Recommender System Initializing..." << std::endl;
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

    std::unordered_set<int> valid_user_ids, valid_movie_ids;
    identifyValidEntities(user_rating_counts, movie_rating_counts,
                          valid_user_ids, valid_movie_ids, MIN_RATINGS_PER_ENTITY);
    std::cout << "Filtering entities: Keeping users/movies with at least " << MIN_RATINGS_PER_ENTITY << " ratings." << std::endl;
    std::cout << "Valid users found: " << valid_user_ids.size() << ", Valid movies found: " << valid_movie_ids.size() << std::endl;

    UserRatingsLog filtered_users_ratings = raw_users_ratings; 
    filterUserRatingsLog(filtered_users_ratings, valid_user_ids, valid_movie_ids);
    std::cout << "Filtered dataset contains " << filtered_users_ratings.size() << " users." << std::endl;

    std::cout << "Writing filtered dataset to: " << FILTERED_DATASET_PATH << std::endl;
    writeFilteredRatingsToFile(FILTERED_DATASET_PATH, filtered_users_ratings);

    std::cout << "Writing random users to explore to: " << EXPLORE_USERS_PATH << std::endl;
    writeRandomUserIdsToExplore(filtered_users_ratings, NUM_RANDOM_USERS_TO_EXPLORE, EXPLORE_USERS_PATH);

    auto phase_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> phase_elapsed = phase_end_time - phase_start_time;
    std::cout << "Phase 1 completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;

    // --- 2. Building Recommendation Model ---
    std::cout << "\n[Phase 2: Building Recommendation Model]" << std::endl;
    phase_start_time = std::chrono::high_resolution_clock::now();

    if (filtered_users_ratings.empty()) {
        std::cerr << "Error: No data available after filtering. Cannot proceed with recommendation." << std::endl;
        return 1;
    }

    std::cout << "Converting data to user-item matrix format..." << std::endl;
    UserItemMatrix user_item_matrix = convertToUserItemMatrix(filtered_users_ratings);

    std::cout << "Computing user norms..." << std::endl;
    UserNormsMap user_norms = computeUserNorms(user_item_matrix); // Esta função agora é paralelizada
    std::cout << "Norms computed for " << user_norms.size() << " users." << std::endl;

    phase_end_time = std::chrono::high_resolution_clock::now();
    phase_elapsed = phase_end_time - phase_start_time;
    std::cout << "Phase 2 completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;

    // --- 3. Generating Recommendations ---
    std::cout << "\n[Phase 3: Generating Recommendations]" << std::endl;
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
    
    recommendationsOutputFile << "Movie Recommendations (Top " << TOP_N_RECOMMENDATIONS << " using K=" << K_NEIGHBORS << " neighbors)\n";
    recommendationsOutputFile << "-----------------------------------------------------------\n\n";

    int target_user_id_to_recommend;
    int users_processed_for_recommendation = 0;
    if (exploreFile.is_open()) {
         std::cout << "Generating recommendations for users in " << EXPLORE_USERS_PATH << "..." << std::endl;
        while (exploreFile >> target_user_id_to_recommend) {
            users_processed_for_recommendation++;
            recommendationsOutputFile << "User ID: " << target_user_id_to_recommend << "\n";

            if (user_item_matrix.find(target_user_id_to_recommend) == user_item_matrix.end()) {
                recommendationsOutputFile << "  User not found in the filtered dataset or has no ratings.\n\n";
                std::cout << "  User " << target_user_id_to_recommend << " not found for recommendations." << std::endl;
                continue;
            }
            // findKNearestNeighbors dentro de generateRecommendations é paralelizado
            RecommendationList recommendations = generateRecommendations(target_user_id_to_recommend, K_NEIGHBORS,
                                                                       user_item_matrix, user_norms);

            if (recommendations.empty()) {
                recommendationsOutputFile << "  No recommendations could be generated for this user.\n\n";
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
             //std::cout << "  Generated recommendations for User ID: " << target_user_id_to_recommend << std::endl;
        }
        exploreFile.close();
    } else {
        std::cout << "Explore file (" << EXPLORE_USERS_PATH << ") not found. No specific user recommendations generated." << std::endl;
    }
    std::cout << users_processed_for_recommendation << " users processed from explore file." << std::endl;
    std::cout << "Recommendations written to: " << OUTPUT_RECOMMENDATIONS_PATH << std::endl;

    phase_end_time = std::chrono::high_resolution_clock::now();
    phase_elapsed = phase_end_time - phase_start_time;
    std::cout << "Phase 3 completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;

    // --- 4. Evaluation (Optional) ---
    // Descomente para executar a avaliação. CUIDADO: Isso pode consumir muito tempo.
    /*
    std::cout << "\n[Phase 4: Model Evaluation (Leave-One-Out)]" << std::endl;
    phase_start_time = std::chrono::high_resolution_clock::now();
    if (!user_item_matrix.empty()) {
        evaluateKNNLeaveOneOut(user_item_matrix, K_NEIGHBORS); // Esta função agora é paralelizada
    } else {
        std::cout << "Skipping evaluation as user_item_matrix is empty." << std::endl;
    }
    phase_end_time = std::chrono::high_resolution_clock::now();
    phase_elapsed = phase_end_time - phase_start_time;
    std::cout << "Phase 4 completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;
    */

    // --- Total Execution Time ---
    auto program_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_elapsed_time = program_end_time - program_start_time;
    std::cout << "\n----------------------------------------" << std::endl;
    std::cout << "Total execution time: " << std::fixed << std::setprecision(2) << total_elapsed_time.count() << " seconds." << std::endl;
    std::cout << "Movie Recommender System Finished." << std::endl;

    return 0;
}