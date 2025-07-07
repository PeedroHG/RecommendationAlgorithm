#include "csv_parser.hpp"
#include "config.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <unordered_set>
#include <unordered_map>
#include <string_view>
#include <charconv>
#include <system_error>
#include <random>
#include <algorithm>

// --- Funções Auxiliares com Máxima Performance ---

void readRatingsCSV(const std::string& ratings_csv_path, UserRatingsLog& users_ratings_log) {
    std::ifstream inFile(ratings_csv_path, std::ios::in);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open ratings input file: " << ratings_csv_path << std::endl;
        return;
    }

    std::vector<char> buffer(CSV_READ_BUFFER_SIZE);
    inFile.rdbuf()->pubsetbuf(buffer.data(), CSV_READ_BUFFER_SIZE);

    users_ratings_log.clear();
    users_ratings_log.reserve(NUM_EXPECTED_UNIQUE_USERS);

    std::string line;
    std::getline(inFile, line); // Ignora o cabeçalho

    while (std::getline(inFile, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        std::string_view sv_line(line);
        auto first_comma = sv_line.find(',');
        if (first_comma == std::string_view::npos) continue;
        auto second_comma = sv_line.find(',', first_comma + 1);
        if (second_comma == std::string_view::npos) continue;
        auto timestamp_comma = sv_line.find(',', second_comma + 1);

        std::string_view user_str = sv_line.substr(0, first_comma);
        std::string_view movie_str = sv_line.substr(first_comma + 1, second_comma - first_comma - 1);
        std::string_view rating_str = sv_line.substr(second_comma + 1, (timestamp_comma == std::string_view::npos) ? std::string_view::npos : timestamp_comma - (second_comma + 1));
        
        int user_id, movie_id;
        float rating_value;

        auto [p1, ec1] = std::from_chars(user_str.data(), user_str.data() + user_str.size(), user_id);
        auto [p2, ec2] = std::from_chars(movie_str.data(), movie_str.data() + movie_str.size(), movie_id);
        auto [p3, ec3] = std::from_chars(rating_str.data(), rating_str.data() + rating_str.size(), rating_value);

        if (ec1 != std::errc() || ec2 != std::errc() || ec3 != std::errc()) {
             continue;
        }

        auto [it, inserted] = users_ratings_log.try_emplace(user_id);
        it->second.emplace_back(movie_id, rating_value);
    }
}

MovieTitlesMap readMovieTitles(const std::string& movies_csv_path) {
    MovieTitlesMap movie_titles;
    std::ifstream file(movies_csv_path);
    if (!file.is_open()) return movie_titles;

    std::vector<char> buffer(CSV_READ_BUFFER_SIZE);
    file.rdbuf()->pubsetbuf(buffer.data(), buffer.size());
    movie_titles.reserve(NUM_EXPECTED_UNIQUE_MOVIES);

    std::string line;
    std::getline(file, line); // Pula o cabeçalho

    while (std::getline(file, line)) {
        if(line.empty()) continue;
        if(line.back() == '\r') line.pop_back();
        
        std::string_view sv_line(line);
        auto comma_pos = sv_line.find(',');
        if (comma_pos == std::string_view::npos) continue;

        std::string_view id_str = sv_line.substr(0, comma_pos);
        std::string_view title = sv_line.substr(comma_pos + 1);

        if (!title.empty() && title.front() == '"') {
            auto a = title.find('"', 1);
            if(a != std::string_view::npos){
                title = title.substr(1, a - 1);
            }
        }
        
        int movie_id;
        auto [ptr, ec] = std::from_chars(id_str.data(), id_str.data() + id_str.size(), movie_id);
        if (ec == std::errc()) {
            movie_titles.try_emplace(movie_id, title);
        }
    }
    return movie_titles;
}

void countEntityRatings(const UserRatingsLog& users_ratings_log,
                        std::unordered_map<int, int>& user_rating_counts,
                        std::unordered_map<int, int>& movie_rating_counts) {
    user_rating_counts.clear();
    movie_rating_counts.clear();

    user_rating_counts.reserve(users_ratings_log.size());
    movie_rating_counts.reserve(NUM_EXPECTED_UNIQUE_MOVIES);

    for (const auto& user_entry : users_ratings_log) {
        user_rating_counts[user_entry.first] = user_entry.second.size();
        for (const auto& movie_rating_pair : user_entry.second) {
            movie_rating_counts[movie_rating_pair.first]++;
        }
    }
}

void identifyValidEntities(const std::unordered_map<int, int>& user_rating_counts,
                           const std::unordered_map<int, int>& movie_rating_counts,
                           std::unordered_set<int>& valid_user_ids,
                           std::unordered_set<int>& valid_movie_ids,
                           int min_ratings) {
    valid_user_ids.clear();
    valid_movie_ids.clear();

    valid_user_ids.reserve(user_rating_counts.size());
    valid_movie_ids.reserve(movie_rating_counts.size());

    for (const auto& pair : user_rating_counts) {
        if (pair.second >= min_ratings) {
            valid_user_ids.insert(pair.first);
        }
    }
    for (const auto& pair : movie_rating_counts) {
        if (pair.second >= min_ratings) {
            valid_movie_ids.insert(pair.first);
        }
    }
}

void writeFilteredRatingsToFile(const std::string& output_path, const UserRatingsLog& users_ratings_log) {
    std::ofstream outFile(output_path, std::ios::out | std::ios::binary);
    if (!outFile.is_open()) return;

    const size_t WRITE_BUFFER_SIZE = 300 * 1024 * 1024;
    std::vector<char> buffer(WRITE_BUFFER_SIZE);
    char* buffer_ptr = buffer.data();
    char* const buffer_end = buffer.data() + buffer.size();

    for (const auto& [user_id, ratings] : users_ratings_log) {
        if (ratings.empty()) continue;

        if (buffer_end - buffer_ptr < 4096) { // Descarga de segurança
            outFile.write(buffer.data(), buffer_ptr - buffer.data());
            buffer_ptr = buffer.data();
        }

        auto [p1, ec1] = std::to_chars(buffer_ptr, buffer_end, user_id);
        if (ec1 != std::errc()) continue;
        buffer_ptr = p1;

        for (const auto& [movie_id, rating] : ratings) {
            *buffer_ptr++ = ' ';
            auto [p2, ec2] = std::to_chars(buffer_ptr, buffer_end, movie_id);
            if (ec2 != std::errc()) break;
            buffer_ptr = p2;
            *buffer_ptr++ = ':';
            auto [p3, ec3] = std::to_chars(buffer_ptr, buffer_end, rating, std::chars_format::fixed, 1);
            if (ec3 != std::errc()) break;
            buffer_ptr = p3;
        }
        *buffer_ptr++ = '\n';
    }

    if (buffer_ptr > buffer.data()) {
        outFile.write(buffer.data(), buffer_ptr - buffer.data());
    }
}

void writeRandomUserIdsToExplore(const UserRatingsLog& users_ratings_log,
                                 size_t num_users_to_select,
                                 const std::string& output_path) {
    std::vector<int> all_user_ids;
    all_user_ids.reserve(users_ratings_log.size());
    for (const auto& user_entry : users_ratings_log) {
        all_user_ids.push_back(user_entry.first);
    }

    if (all_user_ids.empty()) {
        std::ofstream outFile(output_path);
        return;
    }

    if (num_users_to_select > all_user_ids.size()) {
        num_users_to_select = all_user_ids.size();
    }
    
    std::mt19937 generator(42);
    std::shuffle(all_user_ids.begin(), all_user_ids.end(), generator);

    std::string output_buffer;
    output_buffer.reserve(num_users_to_select * 8);

    for (size_t i = 0; i < num_users_to_select; ++i) {
        output_buffer.append(std::to_string(all_user_ids[i]));
        output_buffer.push_back('\n');
    }

    std::ofstream outFile(output_path, std::ios::out | std::ios::binary);
    if (!outFile.is_open()) return;
    outFile.write(output_buffer.data(), output_buffer.size());
}

// --- VERSÃO COM PERFORMANCE MAXIMIZADA DA FASE 1 ---
bool executePreProcessing(UserRatingsLog& filtered_ratings_out, MovieTitlesMap& movie_titles_out) {
    auto phase_start_time = std::chrono::high_resolution_clock::now();
    std::cout << "\n[Phase 1: Data Loading and Preprocessing]" << std::endl;

    UserRatingsLog raw_users_ratings;
    std::cout << "Reading ratings from: " << RATINGS_CSV_PATH << std::endl;
    readRatingsCSV(RATINGS_CSV_PATH, raw_users_ratings);
    std::cout << "Initial ratings loaded for " << raw_users_ratings.size() << " users." << std::endl;

    std::cout << "Reading movie titles from: " << MOVIES_CSV_PATH << std::endl;
    movie_titles_out = readMovieTitles(MOVIES_CSV_PATH);
    std::cout << "Loaded " << movie_titles_out.size() << " movie titles." << std::endl;

    std::unordered_map<int, int> user_rating_counts, movie_rating_counts;
    countEntityRatings(raw_users_ratings, user_rating_counts, movie_rating_counts);

    std::unordered_set<int> valid_user_ids, valid_movie_ids_set;
    identifyValidEntities(user_rating_counts, movie_rating_counts, valid_user_ids, valid_movie_ids_set, MIN_RATINGS_PER_ENTITY);
    std::cout << "Filtering entities: Keeping users/movies with at least " << MIN_RATINGS_PER_ENTITY << " ratings." << std::endl;
    std::cout << "Valid users found: " << valid_user_ids.size() << ", Valid movies found: " << valid_movie_ids_set.size() << std::endl;

    std::cout << "Filtering raw ratings log to create final dataset..." << std::endl;
    auto func_start_time = std::chrono::high_resolution_clock::now();

    filtered_ratings_out.clear();
    filtered_ratings_out.reserve(valid_user_ids.size());

    for (const auto& user_entry : raw_users_ratings) {
        if (valid_user_ids.count(user_entry.first) == 0) continue;

        std::vector<std::pair<int, float>> valid_ratings_for_user;
        for (const auto& movie_rating_pair : user_entry.second) {
            if (valid_movie_ids_set.count(movie_rating_pair.first)) {
                valid_ratings_for_user.push_back(movie_rating_pair);
            }
        }

        if (!valid_ratings_for_user.empty()) {
            filtered_ratings_out[user_entry.first] = std::move(valid_ratings_for_user);
        }
    }
    
    auto func_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> func_elapsed = func_end_time - func_start_time;
    std::cout << "  -> Filtering completed in " << std::fixed << std::setprecision(6) << func_elapsed.count() << " seconds." << std::endl;
    std::cout << "Filtered dataset contains " << filtered_ratings_out.size() << " users." << std::endl;

    std::cout << "Writing filtered dataset to: " << FILTERED_DATASET_PATH << std::endl;
    writeFilteredRatingsToFile(FILTERED_DATASET_PATH, filtered_ratings_out);
    
    std::cout << "Writing random users to explore to: " << EXPLORE_USERS_PATH << std::endl;
    writeRandomUserIdsToExplore(filtered_ratings_out, NUM_RANDOM_USERS_TO_EXPLORE, EXPLORE_USERS_PATH);
    
    auto phase_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> phase_elapsed = phase_end_time - phase_start_time;
    std::cout << "Phase 1 completed in " << std::fixed << std::setprecision(2) << phase_elapsed.count() << " seconds." << std::endl;
    
    return true;
}