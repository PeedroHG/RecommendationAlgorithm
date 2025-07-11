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
#include <format> 
#include <omp.h>
#include <tuple> // Para std::ignore

// --- NENHUMA MUDANÇA NECESSÁRIA AQUI ---
// Esta função já está bem otimizada.
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


// --- OTIMIZAÇÃO #1: LEITURA ULTRA-RÁPIDA DO CSV ---
// Substituímos o loop com std::getline por uma leitura em bloco (fread) e processamento manual do buffer.
// Isso minimiza as chamadas de sistema e é ordens de magnitude mais rápido para arquivos grandes.
void readRatingsCSV(const std::string& ratings_csv_path, UserRatingsLog& users_ratings_log) {
    FILE* file = fopen(ratings_csv_path.c_str(), "rb");
    if (!file) {
        std::cerr << "Error: Could not open ratings input file: " << ratings_csv_path << std::endl;
        return;
    }

    users_ratings_log.clear();
    users_ratings_log.reserve(NUM_EXPECTED_UNIQUE_USERS);

    std::vector<char> buffer(CSV_READ_BUFFER_SIZE);
    std::string leftover; // Guarda pedaços de linha entre leituras de buffer
    leftover.reserve(1024); 

    // Pula o cabeçalho, ignorando o valor de retorno de forma explícita e segura
    char header_buf[1024];
    std::ignore = fgets(header_buf, sizeof(header_buf), file);

    while (size_t bytes_read = fread(buffer.data(), 1, buffer.size(), file)) {
        std::string_view chunk(buffer.data(), bytes_read);
        size_t start_pos = 0;

        while (start_pos < chunk.size()) {
            size_t end_pos = chunk.find('\n', start_pos);
            
            // Se encontrou uma linha completa
            if (end_pos != std::string_view::npos) {
                std::string_view line_view = chunk.substr(start_pos, end_pos - start_pos);
                
                // Se havia um pedaço da leitura anterior, junte-os
                if (!leftover.empty()) {
                    leftover.append(line_view);
                    line_view = leftover;
                }

                // Processa a linha completa
                if (!line_view.empty()) {
                    if (line_view.back() == '\r') line_view.remove_suffix(1);

                    auto first_comma = line_view.find(',');
                    if (first_comma != std::string_view::npos) {
                        auto second_comma = line_view.find(',', first_comma + 1);
                        if (second_comma != std::string_view::npos) {
                            auto timestamp_comma = line_view.find(',', second_comma + 1);

                            std::string_view user_str = line_view.substr(0, first_comma);
                            std::string_view movie_str = line_view.substr(first_comma + 1, second_comma - first_comma - 1);
                            std::string_view rating_str = line_view.substr(second_comma + 1, (timestamp_comma == std::string_view::npos) ? std::string_view::npos : timestamp_comma - (second_comma + 1));
                            
                            int user_id, movie_id;
                            float rating_value;

                            auto [p1, ec1] = std::from_chars(user_str.data(), user_str.data() + user_str.size(), user_id);
                            auto [p2, ec2] = std::from_chars(movie_str.data(), movie_str.data() + movie_str.size(), movie_id);
                            auto [p3, ec3] = std::from_chars(rating_str.data(), rating_str.data() + rating_str.size(), rating_value);

                            if (ec1 == std::errc() && ec2 == std::errc() && ec3 == std::errc()) {
                                // --- OTIMIZAÇÃO #2: RESERVA DO VETOR INTERNO ---
                                // A primeira vez que um usuário é inserido, reservamos uma capacidade inicial
                                // para seu vetor de avaliações, evitando múltiplas realocações.
                                auto [it, inserted] = users_ratings_log.try_emplace(user_id);
                                if (inserted) {
                                    it->second.reserve(10); // Uma estimativa inicial conservadora
                                }
                                it->second.emplace_back(movie_id, rating_value);
                            }
                        }
                    }
                }
                if (!leftover.empty()) leftover.clear();
                start_pos = end_pos + 1;
            } 
            // Se não encontrou linha completa, o resto do chunk é um pedaço de linha
            else {
                leftover.append(chunk.substr(start_pos));
                break; 
            }
        }
    }
    fclose(file);
}

// --- NENHUMA MUDANÇA NECESSÁRIA AQUI ---
// Esta função já está bem otimizada.
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

// --- NENHUMA MUDANÇA NECESSÁRIA AQUI ---
// Esta função já está bem otimizada.
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


// --- OTIMIZAÇÃO #3: ALGORITMO DE FILTRAGEM EFICIENTE ---
// Em vez de apagar elementos da estrutura original (o que é muito lento),
// criamos uma nova estrutura de dados já filtrada. É um processo de uma passagem,
// muito mais rápido e que resulta em uma estrutura de dados mais compacta na memória.
void filterUserRatingsLog(UserRatingsLog& users_ratings_log,
                          const std::unordered_set<int>& valid_user_ids,
                          const std::unordered_set<int>& valid_movie_ids) {
    UserRatingsLog filtered_log;
    filtered_log.reserve(valid_user_ids.size());

    for (const auto& user_id : valid_user_ids) {
        // Encontra o usuário no log original
        auto it_user = users_ratings_log.find(user_id);
        if (it_user == users_ratings_log.end()) continue;

        // Cria um novo vetor de avaliações apenas com os filmes válidos
        std::vector<std::pair<int, float>> filtered_ratings;
        const auto& original_ratings = it_user->second;
        filtered_ratings.reserve(original_ratings.size());

        std::copy_if(original_ratings.begin(), original_ratings.end(),
                     std::back_inserter(filtered_ratings),
                     [&valid_movie_ids](const auto& rating_pair) {
                         return valid_movie_ids.count(rating_pair.first);
                     });
        
        // Se o usuário ainda tiver avaliações após a filtragem, adicione-o ao novo log
        if (!filtered_ratings.empty()) {
            filtered_log.try_emplace(user_id, std::move(filtered_ratings));
        }
    }

    // Substitui o log antigo pelo novo, filtrado e otimizado
    users_ratings_log = std::move(filtered_log);
}


// --- NENHUMA MUDANÇA NECESSÁRIA AQUI ---
// Sua implementação paralela já é uma excelente abordagem.
void writeFilteredRatingsToFile(const std::string& output_path, const UserRatingsLog& users_ratings_log) {
    std::ofstream outFile(output_path, std::ios::out | std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file: " << output_path << std::endl;
        return;
    }

    const size_t num_users = users_ratings_log.size();
    std::vector<const std::pair<const int, std::vector<std::pair<int, float>>>*> user_ptrs;
    user_ptrs.reserve(num_users);
    for (const auto& entry : users_ratings_log) user_ptrs.push_back(&entry);

    int num_threads = 1;
#ifdef _OPENMP
    num_threads = omp_get_max_threads();
#endif
    std::vector<std::string> thread_buffers(num_threads);

#pragma omp parallel num_threads(num_threads)
    {
        int tid = 0;
#ifdef _OPENMP
        tid = omp_get_thread_num();
#endif
        std::string& local_buffer = thread_buffers[tid];
        char temp_buffer[64];

#pragma omp for schedule(static)
        for (size_t idx = 0; idx < num_users; ++idx) {
            const auto& user_entry = *user_ptrs[idx];
            int user_id = user_entry.first;
            const auto& ratings = user_entry.second;
            if (ratings.empty()) continue;

            auto [ptr1, ec1] = std::to_chars(temp_buffer, temp_buffer + sizeof(temp_buffer), user_id);
            if (ec1 == std::errc()) {
                local_buffer.append(temp_buffer, ptr1);
            }

            for (const auto& [movie_id, rating] : ratings) {
                local_buffer.push_back(' ');
                auto [ptr2, ec2] = std::to_chars(temp_buffer, temp_buffer + sizeof(temp_buffer), movie_id);
                if (ec2 == std::errc()) {
                    local_buffer.append(temp_buffer, ptr2);
                }
                local_buffer.push_back(':');
                auto [ptr3, ec3] = std::to_chars(temp_buffer, temp_buffer + sizeof(temp_buffer), rating, std::chars_format::fixed, 1);
                if (ec3 == std::errc()) {
                    local_buffer.append(temp_buffer, ptr3);
                }
            }
            local_buffer.push_back('\n');
        }
    }

    std::string output_buffer;
    size_t total_size = 0;
    for (const auto& buf : thread_buffers) total_size += buf.size();
    output_buffer.reserve(total_size);
    for (const auto& buf : thread_buffers) output_buffer.append(buf);

    outFile.write(output_buffer.data(), output_buffer.size());
}


// --- NENHUMA MUDANÇA NECESSÁRIA AQUI ---
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
    }
    
    std::mt19937 generator(42); // Semente fixa para reprodutibilidade
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
