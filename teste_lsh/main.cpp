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
#include <random> // Para std::mt19937
#include <set>    // Para coletar movie_ids únicos
#include <unordered_set>
#include <thread>

#ifdef _OPENMP
#include <omp.h>
#endif

int main()
{
    auto program_start_time = std::chrono::high_resolution_clock::now();

    #ifdef _OPENMP
        std::cout << "OpenMP está ATIVADO. Número máximo de threads: " << omp_get_max_threads() << std::endl;
    #else
        std::cout << "OpenMP NÃO está ativado/disponível." << std::endl;
    #endif

    std::cout << "LSH Movie Recommender System Initializing..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // --- Objetos que serão preenchidos pelo pré-processamento ---
    UserRatingsLog filtered_users_ratings;
    MovieTitlesMap movie_titles;

    // --- 1. Executa todo o pré-processamento ---
    if (!executePreProcessing(filtered_users_ratings, movie_titles))
    {
        std::cerr << "Falha no pré-processamento. Encerrando." << std::endl;
        return 1;
    }

    // --- 2. Building Recommendation Model (User-Item Matrix, Norms) ---
    std::cout << "\n[Phase 2: Building Core Model Components]" << std::endl;
    auto phase_start_time = std::chrono::high_resolution_clock::now();

    if (filtered_users_ratings.empty())
    {
        std::cerr << "Error: No data available after filtering. Cannot proceed." << std::endl;
        return 1;
    }

    std::cout << "Converting data to user-item matrix format..." << std::endl;
    UserItemMatrix user_item_matrix = convertToUserItemMatrix(filtered_users_ratings);

    std::cout << "Computing user norms..." << std::endl;
    UserNormsMap user_norms = computeUserNorms(user_item_matrix);
    std::cout << "Norms computed for " << user_norms.size() << " users." << std::endl;

    auto phase_end_time = std::chrono::high_resolution_clock::now();
    auto phase_elapsed = phase_end_time - phase_start_time;
    std::cout << "Phase 2 completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;

    // --- 2.5 Building LSH Model ---
    std::cout << "\n[Phase 2.5: Building LSH Model]" << std::endl;
    phase_start_time = std::chrono::high_resolution_clock::now();

    MovieIdToDenseIdxMap movie_to_idx;
    DenseIdxToMovieIdVec idx_to_movie; // Opcional, para referência
    std::set<int> unique_movie_ids_in_matrix;
    for (const auto &user_entry : user_item_matrix)
    {
        for (const auto &movie_entry : user_entry.second)
        {
            unique_movie_ids_in_matrix.insert(movie_entry.first);
        }
    }
    int dense_idx_counter = 0;
    for (int movie_id : unique_movie_ids_in_matrix)
    {
        movie_to_idx[movie_id] = dense_idx_counter++;
        idx_to_movie.push_back(movie_id);
    }
    int dimensionality = unique_movie_ids_in_matrix.size();
    std::cout << "LSH dimensionality (unique movies in matrix): " << dimensionality << std::endl;

    if (dimensionality == 0 && !user_item_matrix.empty())
    {
        std::cerr << "Error: Dimensionality is 0 but user_item_matrix is not empty. Check movie ID collection." << std::endl;
        return 1;
    }

    std::vector<HyperplaneSet> all_hyperplane_sets(NUM_LSH_TABLES);
    std::vector<LSHBucketMap> lsh_tables(NUM_LSH_TABLES);

    if (dimensionality > 0)
    {
        std::mt19937 rng(std::chrono::system_clock::now().time_since_epoch().count()); // Seed RNG
        std::cout << "Generating " << NUM_LSH_TABLES << " LSH hyperplane sets ("
                  << NUM_HYPERPLANES_PER_TABLE << " hyperplanes/table)..." << std::endl;
        for (int i = 0; i < NUM_LSH_TABLES; ++i)
        {
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

    // Carregar ground truth para avaliação (usando o dataset filtrado como exemplo)
    Evaluator evaluator(filtered_users_ratings);

    std::ifstream exploreFile(EXPLORE_USERS_PATH);
    if (!exploreFile.is_open())
    {
        std::cerr << "Error: Could not open explore users file: " << EXPLORE_USERS_PATH << std::endl;
    }

    std::ofstream recommendationsOutputFile(OUTPUT_RECOMMENDATIONS_PATH);
    if (!recommendationsOutputFile.is_open())
    {
        std::cerr << "Error: Could not create output recommendations file: " << OUTPUT_RECOMMENDATIONS_PATH << std::endl;
        return 1;
    }

    recommendationsOutputFile << "LSH Movie Recommendations (Top " << TOP_N_RECOMMENDATIONS
                              << " using K_approx=" << K_NEIGHBORS
                              << ", L=" << NUM_LSH_TABLES << ", k_hash=" << NUM_HYPERPLANES_PER_TABLE << ")\n";
    recommendationsOutputFile << "-----------------------------------------------------------\n\n";

    if (exploreFile.is_open())
    {
        std::cout << "Generating LSH recommendations for users in " << EXPLORE_USERS_PATH << "..." << std::endl;

        // Lê todos os usuários a serem processados
        std::vector<int> explore_user_ids;
        int uid;
        while (exploreFile >> uid)
            explore_user_ids.push_back(uid);
        exploreFile.close();

        // Buffer para saída paralela
        std::vector<std::string> user_outputs(explore_user_ids.size());

#pragma omp parallel for schedule(dynamic)
        for (size_t idx = 0; idx < explore_user_ids.size(); ++idx)
        {
            int target_user_id_to_recommend = explore_user_ids[idx];
            // Obter os k vizinhos mais próximos via LSH
            NeighborList neighbors = findApproximateKNearestNeighborsLSH(
                target_user_id_to_recommend, user_item_matrix, user_norms,
                all_hyperplane_sets, lsh_tables, movie_to_idx, K_NEIGHBORS);
            std::vector<int> neighbor_ids;
            for (const auto &n : neighbors)
                neighbor_ids.push_back(n.first);
            RecommendationList recommendations = generateRecommendationsLSH(
                target_user_id_to_recommend, K_NEIGHBORS, user_item_matrix, user_norms,
                all_hyperplane_sets, lsh_tables, movie_to_idx, &neighbors);
            // Converter para vetor de IDs de filmes
            std::vector<int> recommended_ids;
            int count = 0;
            for (const auto &rec : recommendations)
            {
                if (count++ >= TOP_N_RECOMMENDATIONS)
                    break;
                recommended_ids.push_back(rec.first);
            }
            // Calcular hit rate individual usando apenas os filmes dos vizinhos
            std::unordered_set<int> neighbor_movies;
            for (int neighbor_id : neighbor_ids)
            {
                if (filtered_users_ratings.count(neighbor_id))
                {
                    for (const auto &pair : filtered_users_ratings.at(neighbor_id))
                    {
                        neighbor_movies.insert(pair.first);
                    }
                }
            }
            int hits = 0;
            for (int recommended_movie : recommended_ids)
            {
                if (neighbor_movies.count(recommended_movie))
                {
                    hits++;
                }
            }
            double user_hit_rate = recommended_ids.empty() ? 0.0 : (100.0 * hits / recommended_ids.size());
            // Montar saída formatada
            std::ostringstream oss;
            oss << "User ID: " << target_user_id_to_recommend << " | Hit Rate: " << std::fixed << std::setprecision(1) << user_hit_rate << "%\n";
            oss << "  Recommended Movies (MovieID: Score | Title):\n";
            count = 0;
            for (const auto &rec : recommendations)
            {
                if (count++ >= TOP_N_RECOMMENDATIONS)
                    break;
                oss << "  - " << rec.first << ": " << std::fixed << std::setprecision(3) << rec.second << " | ";
                if (movie_titles.count(rec.first))
                {
                    oss << movie_titles.at(rec.first);
                }
                else
                {
                    oss << "(Title not found)";
                }
                oss << "\n";
            }
            oss << "\n";
            user_outputs[idx] = oss.str();
        }
        // Escrever tudo de uma vez (thread-safe)
        for (const auto &out : user_outputs)
        {
            recommendationsOutputFile << out;
        }

        std::cout << "LSH Recommendations written to: " << OUTPUT_RECOMMENDATIONS_PATH << std::endl;
    }
    else
    {
        std::cerr << "Error: Could not open explore users file: " << EXPLORE_USERS_PATH << std::endl;
    }

    phase_end_time = std::chrono::high_resolution_clock::now();
    phase_elapsed = phase_end_time - phase_start_time;
    std::cout << "Phase 3 (LSH Recommendations) completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;

    // --- Total Execution Time ---
    auto program_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_elapsed_time = program_end_time - program_start_time;
    std::cout << "\n----------------------------------------" << std::endl;

    std::cout << "Total execution time: " << std::fixed << std::setprecision(4) << total_elapsed_time.count() << " seconds." << std::endl;

    return 0;
}