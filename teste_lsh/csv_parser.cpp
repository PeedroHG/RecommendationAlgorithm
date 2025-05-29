#include "csv_parser.hpp"
#include "config.hpp" // Para CSV_READ_BUFFER_SIZE
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm> // Para std::remove_if, std::shuffle
#include <random>    // Para std::random_device, std::mt19937

MovieTitlesMap readMovieTitles(const std::string& movies_csv_path) {
    MovieTitlesMap movie_titles;
    std::ifstream file(movies_csv_path);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open movies file: " << movies_csv_path << std::endl;
        return movie_titles;
    }

    std::string line;
    std::getline(file, line); 

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string id_str;
        std::string title;
        
        if (!std::getline(ss, id_str, ',')) continue;
        std::getline(ss, title); 

        if (!title.empty() && title.front() == '"' && title.back() == '"') {
            title = title.substr(1, title.length() - 2);
        }
        
        try {
            int movie_id = std::stoi(id_str);
            movie_titles[movie_id] = title;
        } catch (const std::invalid_argument& ia) {
            std::cerr << "Warning: Invalid movie ID format in movies.csv: " << id_str << " in line: " << line << std::endl;
        } catch (const std::out_of_range& oor) {
            std::cerr << "Warning: Movie ID out of range in movies.csv: " << id_str << " in line: " << line << std::endl;
        }
    }
    return movie_titles;
}

void readRatingsCSV(const std::string& ratings_csv_path, UserRatingsLog& users_ratings_log) {
    std::ifstream inFile(ratings_csv_path, std::ios::in | std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open ratings input file: " << ratings_csv_path << std::endl;
        return;
    }

    std::vector<char> buffer(CSV_READ_BUFFER_SIZE);
    inFile.rdbuf()->pubsetbuf(buffer.data(), CSV_READ_BUFFER_SIZE);

    users_ratings_log.clear();
    std::string line;
    std::getline(inFile, line); 

    while (std::getline(inFile, line)) {
        if (line.empty() || line.back() == '\r') { 
             if (!line.empty()) line.pop_back();
             if (line.empty()) continue;
        }

        std::stringstream ss(line);
        std::string token_user_id, token_movie_id, token_rating;
        int user_id, movie_id;
        float rating_value;

        if (!std::getline(ss, token_user_id, ',')) continue;
        if (!std::getline(ss, token_movie_id, ',')) continue;
        if (!std::getline(ss, token_rating, ',')) continue;

        try {
            user_id = std::stoi(token_user_id);
            movie_id = std::stoi(token_movie_id);
            rating_value = std::stof(token_rating);
            users_ratings_log[user_id].emplace_back(movie_id, rating_value);
        } catch (const std::invalid_argument& ia) {
            std::cerr << "Warning: Invalid data format in ratings.csv line: " << line << " (" << ia.what() << ")" << std::endl;
        } catch (const std::out_of_range& oor) {
            std::cerr << "Warning: Data out of range in ratings.csv line: " << line << " (" << oor.what() << ")" << std::endl;
        }
    }
}

void countEntityRatings(const UserRatingsLog& users_ratings_log,
                        std::unordered_map<int, int>& user_rating_counts,
                        std::unordered_map<int, int>& movie_rating_counts) {
    user_rating_counts.clear();
    movie_rating_counts.clear();

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

void filterUserRatingsLog(UserRatingsLog& users_ratings_log,
                          const std::unordered_set<int>& valid_user_ids,
                          const std::unordered_set<int>& valid_movie_ids) {
    for (auto it_user = users_ratings_log.begin(); it_user != users_ratings_log.end();) {
        if (valid_user_ids.find(it_user->first) == valid_user_ids.end()) {
            it_user = users_ratings_log.erase(it_user);
        } else {
            auto& movie_ratings_vector = it_user->second;
            movie_ratings_vector.erase(
                std::remove_if(movie_ratings_vector.begin(), movie_ratings_vector.end(),
                               [&valid_movie_ids](const std::pair<int, float>& p) {
                                   return valid_movie_ids.find(p.first) == valid_movie_ids.end();
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
    std::ofstream outFile(output_path);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file for filtered data: " << output_path << std::endl;
        return;
    }

    for (const auto& user_entry : users_ratings_log) {
        if (user_entry.second.empty()) continue; 

        outFile << user_entry.first; 
        for (const auto& movie_rating_pair : user_entry.second) {
            outFile << " " << movie_rating_pair.first << ":" << movie_rating_pair.second;
        }
        outFile << "\n";
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
        std::cerr << "Warning: No users available to select for exploration file." << std::endl;
        std::ofstream outFile(output_path); 
        return;
    }
    
    if (num_users_to_select > all_user_ids.size()) {
        num_users_to_select = all_user_ids.size(); 
        std::cout << "Warning: Requested " << num_users_to_select << " users for explore file, but only "
                  << all_user_ids.size() << " are available. Selecting all." << std::endl;
    }

    std::random_device rd;
    std::mt19937 generator(rd());
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