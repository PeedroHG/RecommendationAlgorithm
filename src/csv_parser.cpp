#include "../include/csv_parser.hpp"
#include "../include/config.hpp"
#include <fstream>      
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <string_view>
#include <charconv>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>     

#ifdef _OPENMP
#include <omp.h>
#endif

// --- Funções Auxiliares de Parsing ---

/**
 * @brief Converte eficientemente uma substring (string_view) para um inteiro.
 * @return true se a conversão for bem-sucedida, false caso contrário.
 */
bool parseInt(std::string_view sv, int& out_val) {
    // std::from_chars é uma forma moderna e de alta performance para converter strings em números
    // sem as alocações de memória e overhead de funções como std::stoi.
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), out_val);
    return ec == std::errc(); // Retorna true se não houver erro.
}

/**
 * @brief Converte eficientemente uma substring (string_view) para um float.
 * @return true se a conversão for bem-sucedida, false caso contrário.
 */
bool parseFloat(std::string_view sv, float& out_val) {
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), out_val);
    return ec == std::errc();
}

// --- Implementações das Funções de Pré-processamento ---

MovieTitlesMap readMovieTitles(const std::string& movies_csv_path) {
    MovieTitlesMap movie_titles;
    std::ifstream file(movies_csv_path, std::ios::binary);
    if (!file) {
        std::cerr << "Erro: Não foi possível abrir o arquivo de filmes: " << movies_csv_path << std::endl;
        return movie_titles;
    }

    // Pré-aloca memória para o mapa para evitar realocações custosas durante a inserção.
    movie_titles.reserve(NUM_EXPECTED_UNIQUE_MOVIES);

    std::string line;
    std::getline(file, line); // Pula a linha do cabeçalho (header).

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        // Remove o caractere de retorno de carro '\r' se o arquivo for do formato Windows.
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::string_view sv(line);
        auto first_comma = sv.find(',');
        if (first_comma == std::string_view::npos) continue;

        int movie_id;
        if (!parseInt(sv.substr(0, first_comma), movie_id)) continue;
        
        // Remove o ID e a primeira vírgula para isolar o título e os gêneros.
        sv.remove_prefix(first_comma + 1);

        // Lida com títulos que contêm vírgulas e, por isso, estão entre aspas.
        std::string title_str;
        if (!sv.empty() && sv.front() == '"') {
            sv.remove_prefix(1); // Remove a aspa inicial.
            auto last_quote = sv.find_last_of('"');
            if (last_quote != std::string_view::npos) {
                title_str = std::string(sv.substr(0, last_quote));
            } else {
                 title_str = std::string(sv); // Caso de aspa não fechada.
            }
        } else {
            // Se não há aspas, o título vai até a última vírgula (que separa dos gêneros).
            auto last_comma = sv.find_last_of(',');
            if(last_comma != std::string_view::npos) {
                title_str = std::string(sv.substr(0, last_comma));
            } else {
                title_str = std::string(sv);
            }
        }
        movie_titles[movie_id] = std::move(title_str);
    }
    return movie_titles;
}

void readRatingsCSV(const std::string& ratings_csv_path, UserRatingsLog& users_ratings_log) {
    // --- OTIMIZAÇÃO DE I/O: Memory-Mapped File (mmap) ---
    // 1. Abrimos o arquivo usando um descritor de arquivo de baixo nível (C-style).
    int fd = open(ratings_csv_path.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "Erro: Não foi possível abrir o arquivo de avaliações com open(): " << ratings_csv_path << std::endl;
        return;
    }

    // 2. Obtemos o tamanho do arquivo para saber a quantidade de memória a mapear.
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        std::cerr << "Erro: Não foi possível obter o tamanho do arquivo com fstat()." << std::endl;
        close(fd);
        return;
    }
    size_t file_size = sb.st_size;

    // 3. Mapeamos o arquivo na memória virtual do processo.
    // Isso evita cópias de dados entre o kernel e o espaço do usuário, sendo a forma mais rápida de ler um arquivo.
    // O sistema operacional gerencia o carregamento das páginas do arquivo sob demanda.
    char* file_content = static_cast<char*>(mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (file_content == MAP_FAILED) {
        std::cerr << "Erro: Falha no mmap()." << std::endl;
        close(fd);
        return;
    }
    close(fd); // O descritor de arquivo pode ser fechado após o mmap.

    // Pula o cabeçalho (header) encontrando a primeira quebra de linha.
    // Usamos memchr por ser mais rápido que std::string::find para buffers de char.
    char* header_end = static_cast<char*>(memchr(file_content, '\n', file_size));
    if (!header_end) {
        munmap(file_content, file_size);
        return; 
    }
    char* content_start = header_end + 1;
    size_t content_size = file_size - (content_start - file_content);

    users_ratings_log.clear();

    // --- Lógica de Processamento Paralelo ---
    // Cria um log de avaliações local para cada thread para evitar "race conditions".
    std::vector<UserRatingsLog> thread_local_logs;
    int num_threads = 1;
    #ifdef _OPENMP
        #pragma omp parallel
        {
            #pragma omp single
            num_threads = omp_get_num_threads();
        }
    #endif
    thread_local_logs.resize(num_threads);
    for(auto& log : thread_local_logs) {
        log.reserve(NUM_EXPECTED_UNIQUE_USERS / num_threads);
    }
    
    // Inicia a região paralela. Cada thread processará um "chunk" (pedaço) do arquivo.
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        size_t chunk_size = content_size / num_threads;
        size_t start_offset = thread_id * chunk_size;
        size_t end_offset = (thread_id == num_threads - 1) ? content_size : (thread_id + 1) * chunk_size;

        // Ajusta os offsets de início e fim para garantir que não cortemos linhas ao meio.
        // A thread 0 começa no início. As outras threads buscam a próxima quebra de linha a partir do seu 'start_offset'.
        if (thread_id > 0 && start_offset > 0 && content_start[start_offset - 1] != '\n') {
            char* EOL = static_cast<char*>(memchr(content_start + start_offset, '\n', content_size - start_offset));
            if (EOL) {
                start_offset = EOL - content_start + 1;
            }
        }
        if (thread_id < num_threads - 1 && end_offset < content_size) {
             char* EOL = static_cast<char*>(memchr(content_start + end_offset, '\n', content_size - end_offset));
            if (EOL) {
                end_offset = EOL - content_start + 1;
            }
        }
        
        if (start_offset < end_offset) {
            UserRatingsLog& local_log = thread_local_logs[thread_id];
            char* current_pos = content_start + start_offset;
            char* end_pos = content_start + end_offset;

            // Itera sobre as linhas dentro do chunk da thread
            while(current_pos < end_pos) {
                char* next_newline = static_cast<char*>(memchr(current_pos, '\n', end_pos - current_pos));
                char* line_end = next_newline ? next_newline : end_pos;
                
                std::string_view line(current_pos, line_end - current_pos);

                if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
                if (!line.empty()) {
                    auto first_comma = line.find(',');
                    if (first_comma != std::string_view::npos) {
                        auto second_comma = line.find(',', first_comma + 1);
                        if (second_comma != std::string_view::npos) {
                            int user_id, movie_id;
                            float rating;

                            if (parseInt(line.substr(0, first_comma), user_id) &&
                                parseInt(line.substr(first_comma + 1, second_comma - first_comma - 1), movie_id) &&
                                parseFloat(line.substr(second_comma + 1), rating)) {
                                local_log[user_id].emplace_back(movie_id, rating);
                            }
                        }
                    }
                }
                if (!next_newline) break;
                current_pos = next_newline + 1;
            }
        }
    }

    // Fase de Redução (Merge): Combina os resultados dos logs locais no log final.
    // Esta fase é sequencial, mas rápida, pois apenas move ponteiros.
    users_ratings_log.reserve(NUM_EXPECTED_UNIQUE_USERS);
    for (const auto& local_log : thread_local_logs) {
        for (const auto& [user_id, ratings] : local_log) {
            users_ratings_log[user_id].insert(users_ratings_log[user_id].end(), ratings.begin(), ratings.end());
        }
    }
    
    // Libera a memória mapeada.
    munmap(file_content, file_size);
}


void countEntityRatings(const UserRatingsLog& users_ratings_log,
                        std::unordered_map<int, int>& user_rating_counts,
                        std::unordered_map<int, int>& movie_rating_counts) {
    user_rating_counts.clear();
    movie_rating_counts.clear();
    user_rating_counts.reserve(users_ratings_log.size());

    // Converte o mapa para um vetor de ponteiros para facilitar a iteração paralela.
    std::vector<const std::pair<const int, std::vector<std::pair<int, float>>>*> user_vec;
    user_vec.reserve(users_ratings_log.size());
    for (const auto& pair : users_ratings_log) {
        user_vec.push_back(&pair);
    }

    // Em vez de usar `#pragma omp critical` que cria um gargalo (serializa o acesso),
    // cada thread escreve em suas próprias estruturas de dados locais.
    
    // Vetor para contagens de usuário. Acesso por índice (i) é seguro em paralelo.
    std::vector<std::pair<int, int>> user_counts_vec(user_vec.size());
    
    int num_threads = 1;
    #ifdef _OPENMP
        #pragma omp parallel
        {
            #pragma omp single
            num_threads = omp_get_num_threads();
        }
    #endif
    // Vetor de mapas locais para contagens de filmes. Cada thread tem seu próprio mapa.
    std::vector<std::unordered_map<int, int>> local_movie_counts(num_threads);

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        local_movie_counts[thread_id].reserve(NUM_EXPECTED_UNIQUE_MOVIES / num_threads);

        // O loop 'for' é paralelizado. Cada thread pega um conjunto de usuários.
        #pragma omp for schedule(dynamic)
        for (size_t i = 0; i < user_vec.size(); ++i) {
            const auto& user_entry = *user_vec[i];
            int user_id = user_entry.first;
            const auto& ratings = user_entry.second;
            
            // Escrita segura no vetor de contagem de usuários.
            user_counts_vec[i] = {user_id, static_cast<int>(ratings.size())};

            // Escrita no mapa local de filmes da thread (sem concorrência).
            for (const auto& movie_rating_pair : ratings) {
                local_movie_counts[thread_id][movie_rating_pair.first]++;
            }
        }
    }

    // Após o loop paralelo, os resultados locais são combinados sequencialmente.
    // Esta parte é muito rápida.
    for(const auto& p : user_counts_vec) {
        user_rating_counts[p.first] = p.second;
    }
    
    movie_rating_counts.reserve(NUM_EXPECTED_UNIQUE_MOVIES);
    for (const auto& local_map : local_movie_counts) {
        for (const auto& pair : local_map) {
            movie_rating_counts[pair.first] += pair.second;
        }
    }
}

void identifyValidEntities(const std::unordered_map<int, int>& user_rating_counts,
                           const std::unordered_map<int, int>& movie_rating_counts,
                           int min_ratings,
                           std::unordered_set<int>& out_valid_user_ids,
                           std::unordered_set<int>& out_valid_movie_ids) {
    out_valid_user_ids.clear();
    out_valid_movie_ids.clear();
    out_valid_user_ids.reserve(user_rating_counts.size());
    out_valid_movie_ids.reserve(movie_rating_counts.size());

    // Itera sobre os contadores e insere os IDs que atendem ao critério mínimo.
    for (const auto& pair : user_rating_counts) {
        if (pair.second >= min_ratings) {
            out_valid_user_ids.insert(pair.first);
        }
    }
    for (const auto& pair : movie_rating_counts) {
        if (pair.second >= min_ratings) {
            out_valid_movie_ids.insert(pair.first);
        }
    }
}

void filterUserRatingsLog(const UserRatingsLog& original_log,
                        const std::unordered_set<int>& valid_user_ids,
                        const std::unordered_set<int>& valid_movie_ids,
                        UserRatingsLog& filtered_log) {
    filtered_log.clear();
    filtered_log.reserve(valid_user_ids.size());

    // Cria um vetor com os usuários a serem processados para paralelização.
    std::vector<int> users_to_process;
    users_to_process.reserve(valid_user_ids.size());
    for(int user_id : valid_user_ids) {
        users_to_process.push_back(user_id);
    }
    
    int num_threads = 1;
    #ifdef _OPENMP
        #pragma omp parallel
        {
            #pragma omp single
            num_threads = omp_get_num_threads();
        }
    #endif
    std::vector<UserRatingsLog> local_filtered_logs(num_threads);

    // As threads dividem o trabalho de filtrar as avaliações de cada usuário.
    #pragma omp parallel for schedule(dynamic)
    for(size_t i = 0; i < users_to_process.size(); ++i) {
        int user_id = users_to_process[i];
        int thread_id = 0;
        #ifdef _OPENMP
            thread_id = omp_get_thread_num();
        #endif

        const auto& original_ratings = original_log.at(user_id);
        std::vector<std::pair<int, float>> new_ratings;
        new_ratings.reserve(original_ratings.size());

        // Mantém apenas as avaliações de filmes que estão na lista de filmes válidos.
        for (const auto& rating_pair : original_ratings) {
            if (valid_movie_ids.count(rating_pair.first)) {
                new_ratings.push_back(rating_pair);
            }
        }
        // Se o usuário ainda tiver avaliações após o filtro, adiciona ao log local.
        if (!new_ratings.empty()) {
            local_filtered_logs[thread_id][user_id] = std::move(new_ratings);
        }
    }
    
    // Combina os logs filtrados locais em um único log global.
    for(const auto& local_log : local_filtered_logs) {
        for(const auto& pair : local_log) {
            filtered_log.insert(pair);
        }
    }
}

void writeFilteredRatingsToFile(const std::string& output_path, const UserRatingsLog& users_ratings_log) {
    // Cria um vetor de ponteiros para os usuários para paralelizar a criação das strings.
    std::vector<const std::pair<const int, std::vector<std::pair<int, float>>>*> user_ptrs;
    user_ptrs.reserve(users_ratings_log.size());
    for (const auto& pair : users_ratings_log) {
        user_ptrs.push_back(&pair);
    }

    std::vector<std::string> output_lines(user_ptrs.size());

    // Cada thread gera a string de saída para um subconjunto de usuários.
    #pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < user_ptrs.size(); ++i) {
        const auto& user_entry = *user_ptrs[i];
        std::string line;
        line.reserve(user_entry.second.size() * 15); // Pre-alocação para performance.
        line += std::to_string(user_entry.first);
        for (const auto& rating_pair : user_entry.second) {
            line += " " + std::to_string(rating_pair.first) + ":" + std::to_string(rating_pair.second);
        }
        output_lines[i] = std::move(line);
    }

    std::ofstream outFile(output_path, std::ios::binary);
    if (!outFile) {
        std::cerr << "Erro: não foi possível abrir o arquivo de saída: " << output_path << std::endl;
        return;
    }
    
    // Concatena todas as linhas geradas em uma única string gigante e escreve no arquivo de uma só vez.
    // Isso minimiza as operações de escrita no disco.
    std::string final_output;
    size_t total_size = 0;
    for(const auto& line : output_lines) {
        total_size += line.size() + 1; // +1 para o '\n'
    }
    final_output.reserve(total_size);
    
    for (const auto& line : output_lines) {
        final_output += line;
        final_output += '\n';
    }
    outFile.write(final_output.data(), final_output.size());
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
        std::cerr << "Aviso: Nenhum usuário disponível para selecionar para o arquivo de exploração." << std::endl;
        std::ofstream outFile(output_path);
        return;
    }

    if (num_users_to_select > all_user_ids.size()) {
        std::cout << "Aviso: Solicitados " << num_users_to_select << " usuários, mas apenas "
                  << all_user_ids.size() << " estão disponíveis. Selecionando todos." << std::endl;
        num_users_to_select = all_user_ids.size();
    }
    
    // Usa uma semente fixa (42) para o gerador de números aleatórios,
    // garantindo que a mesma seleção de usuários seja feita toda vez que o programa rodar.
    // Isso é crucial para a reprodutibilidade dos experimentos.
    std::mt19937 generator(42); 
    std::shuffle(all_user_ids.begin(), all_user_ids.end(), generator);

    std::ofstream outFile(output_path);
    if (!outFile) {
        std::cerr << "Erro: não foi possível abrir o arquivo de exploração: " << output_path << std::endl;
        return;
    }
    for (size_t i = 0; i < num_users_to_select; ++i) {
        outFile << all_user_ids[i] << "\n";
    }
}