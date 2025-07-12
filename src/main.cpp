#include "config.hpp"
#include "types.hpp"
#include "csv_parser.hpp"
#include "recommender_engine.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>    
#include <iomanip>   
#include <random>
#include <set>
#include <unordered_set>
#include <numeric>

#ifdef _OPENMP
#include <omp.h>
#endif

int main() {
    auto program_start_time = std::chrono::high_resolution_clock::now();

    // #ifdef _OPENMP
    //     int num_threads = omp_get_max_threads();
    //     std::cout << "OpenMP ATIVADO. Usando " << num_threads << " threads." << std::endl;
    // #else
    //     std::cout << "OpenMP DESATIVADO. Executando em modo single-thread." << std::endl;
    // #endif

    // std::cout << "Iniciando Sistema de Recomendação de Filmes LSH..." << std::endl;
    // std::cout << "----------------------------------------" << std::endl;

    // --- 1. Carregamento e Pré-processamento de Dados ---
    auto phase_start_time = std::chrono::high_resolution_clock::now();
    std::cout << "\n[Fase 1: Carregamento e Pré-processamento de Dados]" << std::endl;
    
    UserRatingsLog raw_users_ratings;
    std::cout << "Lendo avaliações de: " << RATINGS_CSV_PATH << " (pode levar um momento)..." << std::endl;
    readRatingsCSV(RATINGS_CSV_PATH, raw_users_ratings);
    std::cout << "Carregados " << raw_users_ratings.size() << " usuários brutos." << std::endl;

    std::cout << "Lendo títulos de filmes de: " << MOVIES_CSV_PATH << "..." << std::endl;
    MovieTitlesMap movie_titles = readMovieTitles(MOVIES_CSV_PATH);
    std::cout << "Carregados " << movie_titles.size() << " títulos de filmes." << std::endl;

    std::unordered_map<int, int> user_rating_counts, movie_rating_counts;
    std::cout << "Contando avaliações por usuário e por filme..." << std::endl;
    countEntityRatings(raw_users_ratings, user_rating_counts, movie_rating_counts);
    
    std::unordered_set<int> valid_user_ids, valid_movie_ids;
    std::cout << "Identificando entidades com >= " << MIN_RATINGS_PER_ENTITY << " avaliações..." << std::endl;
    identifyValidEntities(user_rating_counts, movie_rating_counts, MIN_RATINGS_PER_ENTITY, valid_user_ids, valid_movie_ids);
    std::cout << "Encontrados " << valid_user_ids.size() << " usuários e " << valid_movie_ids.size() << " filmes válidos." << std::endl;
    
    UserRatingsLog filtered_users_ratings;
    std::cout << "Filtrando o log de avaliações..." << std::endl;
    filterUserRatingsLog(raw_users_ratings, valid_user_ids, valid_movie_ids, filtered_users_ratings);
    
    raw_users_ratings.clear(); 
    raw_users_ratings.rehash(0);
    
    std::cout << "Escrevendo conjunto de dados filtrado para: " << FILTERED_DATASET_PATH << std::endl;
    writeFilteredRatingsToFile(FILTERED_DATASET_PATH, filtered_users_ratings);
    
    std::cout << "Escrevendo usuários aleatórios para exploração em: " << EXPLORE_USERS_PATH << std::endl;
    writeRandomUserIdsToExplore(filtered_users_ratings, NUM_RANDOM_USERS_TO_EXPLORE, EXPLORE_USERS_PATH);

    auto phase_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> phase_elapsed = phase_end_time - phase_start_time;
    std::cout << "Fase 1 concluída em " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " segundos." << std::endl;
    
    // --- 2. Construção da Matriz e Indexação LSH ---
    phase_start_time = std::chrono::high_resolution_clock::now();
    // std::cout << "\n[Fase 2: Construção da Matriz e Indexação LSH]" << std::endl;
    
    // std::cout << "Convertendo log de avaliações para matriz usuário-item..." << std::endl;
    UserItemMatrix user_item_matrix = convertToUserItemMatrix(filtered_users_ratings);

    MovieIdToDenseIdxMap movie_to_idx;
    int dense_idx_counter = 0;
    movie_to_idx.reserve(valid_movie_ids.size());
    for(int movie_id : valid_movie_ids) {
        movie_to_idx[movie_id] = dense_idx_counter++;
    }
    const int D = movie_to_idx.size();
    // std::cout << "Dimensionalidade (filmes únicos): " << D << std::endl;
    
    // std::cout << "Computando normas dos vetores de usuário..." << std::endl;
    UserNormsMap user_norms = computeUserNorms(user_item_matrix);
    
    // std::cout << "Gerando hiperplanos LSH..." << std::endl;
    std::mt19937 rng(42);
    std::vector<HyperplaneSet> all_hyperplane_sets;
    all_hyperplane_sets.reserve(NUM_LSH_TABLES);
    for (int i = 0; i < NUM_LSH_TABLES; ++i) {
        all_hyperplane_sets.push_back(generateSingleHyperplaneSet(NUM_HYPERPLANES_PER_TABLE, D, rng));
    }
    
    // std::cout << "Preenchendo tabelas hash LSH..." << std::endl;
    std::vector<LSHBucketMap> lsh_tables;
    // CORREÇÃO: Chamando a função correta 'buildLSHTables'
    buildLSHTables(user_item_matrix, all_hyperplane_sets, movie_to_idx, lsh_tables);
    
    phase_end_time = std::chrono::high_resolution_clock::now();
    phase_elapsed = phase_end_time - phase_start_time;
    // std::cout << "Fase 2 concluída em " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " segundos." << std::endl;

    // --- 3. Geração de Recomendações ---
    phase_start_time = std::chrono::high_resolution_clock::now();
    // std::cout << "\n[Fase 3: Geração de Recomendações]" << std::endl;
    
    std::vector<int> explore_user_ids = loadExploreUserIds(EXPLORE_USERS_PATH);
    if (explore_user_ids.empty()) {
        std::cerr << "Nenhum usuário para processar no arquivo de exploração." << std::endl;
        return 1;
    }
    
    // std::cout << "Gerando recomendações LSH para " << explore_user_ids.size() 
    //           << " usuários de " << EXPLORE_USERS_PATH << "..." << std::endl;
    
    std::vector<std::string> user_outputs = generateRecommendationsForUsers(
        explore_user_ids, user_item_matrix, user_norms,
        all_hyperplane_sets, lsh_tables, movie_to_idx, movie_titles,
        K_NEIGHBORS, TOP_N_RECOMMENDATIONS);

    std::ofstream recommendationsOutputFile(OUTPUT_RECOMMENDATIONS_PATH);
    if (!recommendationsOutputFile) {
        std::cerr << "Erro: não foi possível abrir o arquivo de saída de recomendações: " << OUTPUT_RECOMMENDATIONS_PATH << std::endl;
        return 1;
    }

    for (const auto& out : user_outputs) {
        recommendationsOutputFile << out;
    }
    // std::cout << "Recomendações LSH escritas em: " << OUTPUT_RECOMMENDATIONS_PATH << std::endl;

    phase_end_time = std::chrono::high_resolution_clock::now();
    phase_elapsed = phase_end_time - phase_start_time;
    // std::cout << "Fase 3 concluída em " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " segundos." << std::endl;
   
    auto program_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_elapsed_time = program_end_time - program_start_time;
    // std::cout << "\n----------------------------------------" << std::endl;
    std::cout << "Tempo total de execução: " << std::fixed << std::setprecision(2) << total_elapsed_time.count() << " segundos." << std::endl;
    // std::cout << "----------------------------------------" << std::endl;

    return 0;
}