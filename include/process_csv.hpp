#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>

// using está associado a um alias que retornna um tipo de dado
// UsersData é um tipo de dado que representa um mapa não ordenado (unordered_map) onde a chave é um inteiro (ID do usuário) e o valor é um vetor de pares (int, float)
// O par (int, float) representa o ID do filme e a nota dada pelo usuário
using UsersData = std::unordered_map<int, std::vector<std::pair<int, float>>>;

void readCSV(const std::string& input, UsersData& usersData);
void countRatings(const UsersData& usersData, std::unordered_map<int, int>& user_count, std::unordered_map<int, int>& movie_count);
void filterValid(const std::unordered_map<int, int>& user_count, const std::unordered_map<int, int>& movie_count, std::unordered_set<int>& user_validated, std::unordered_set<int>& movie_validated, int min_ratings = 50);
void filterUsersMovies(UsersData& usersData, const std::unordered_set<int>& user_validated, const std::unordered_set<int>& movie_validated);
void writeCSV(const std::string& output, const UsersData& usersData);
void writeRandomUsersToExplore(const UsersData& usersData, size_t n, const std::string& outputPath);
