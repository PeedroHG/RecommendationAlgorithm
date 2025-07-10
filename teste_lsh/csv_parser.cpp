#include "csv_parser.hpp"
#include "config.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <string_view>
#include <charconv>
#include <unordered_set>
#include <unordered_map>

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
    std::unordered_set<int> new_valid_users;
    std::unordered_set<int> new_valid_movies;

    for (const auto& pair : user_rating_counts) {
        if (pair.second >= min_ratings) {
            new_valid_users.insert(pair.first);
        }
    }
    for (const auto& pair : movie_rating_counts) {
        if (pair.second >= min_ratings) {
            new_valid_movies.insert(pair.first);
        }
    }

    valid_user_ids = std::move(new_valid_users);
    valid_movie_ids = std::move(new_valid_movies);
}

void filterUserRatingsLog(UserRatingsLog& users_ratings_log,
                          const std::unordered_set<int>& valid_user_ids,
                          const std::unordered_set<int>& valid_movie_ids) {
    for (auto it_user = users_ratings_log.begin(); it_user != users_ratings_log.end();) {
        if (!valid_user_ids.count(it_user->first)) {
            it_user = users_ratings_log.erase(it_user);
        } else {
            auto& movie_ratings_vector = it_user->second;
            movie_ratings_vector.erase(
                std::remove_if(movie_ratings_vector.begin(), movie_ratings_vector.end(),
                               [&valid_movie_ids](const std::pair<int, float>& p) {
                                   return !valid_movie_ids.count(p.first);
                               }),
                movie_ratings_vector.end());

            if (movie_ratings_vector.empty()) {
                it_user = users_ratings_log.erase(it_user);
            } else {
                ++it_user;
            }
        }
    }
}

void writeFilteredRatingsToFile(const std::string& output_path, const UserRatingsLog& users_ratings_log) {
    std::ofstream outFile(output_path, std::ios::out | std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file for filtered data: " << output_path << std::endl;
        return;
    }

    std::string output_buffer;
    output_buffer.reserve(300 * 1024 * 1024); // ajuste conforme necessário

    char rating_str[16]; // buffer temporário para rating formatado
    for (const auto& [user_id, ratings] : users_ratings_log) {
        if (ratings.empty()) continue;

        output_buffer.append(std::to_string(user_id));
        for (const auto& [movie_id, rating] : ratings) {
            output_buffer.push_back(' ');
            output_buffer.append(std::to_string(movie_id));
            output_buffer.push_back(':');

            // converte float com precisão fixa (1 casas decimais)
            std::snprintf(rating_str, sizeof(rating_str), "%.1f", rating);
            output_buffer.append(rating_str);
        }
        output_buffer.push_back('\n');
    }

    outFile.write(output_buffer.data(), output_buffer.size());
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
        std::cerr << "Warning: No users available to select for exploration file." << std::endl;
        std::ofstream outFile(output_path);
        return;
    }

    if (num_users_to_select > all_user_ids.size()) {
        num_users_to_select = all_user_ids.size();
        std::cout << "Warning: Requested " << num_users_to_select << " users for explore file, but only "
                  << all_user_ids.size() << " are available. Selecting all." << std::endl;
    }
    //std::random_device rd;
    //std::mt19937 generator(rd());
    std::mt19937 generator(42); // 42 é a semente fixa, pode escolher outro número
    std::shuffle(all_user_ids.begin(), all_user_ids.end(), generator);

    std::ofstream outFile(output_path);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file for explore.dat: " << output_path << std::endl;
        return;
    }

    for (size_t i = 0; i < num_users_to_select; ++i) {
        outFile << all_user_ids[i] << "\n";
    }
}