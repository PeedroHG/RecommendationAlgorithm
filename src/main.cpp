#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>

// WARNING: Testar outros valores
const size_t BUFFER_SIZE = 16 * 1024 * 1024; // 16 MB

void processCSV(const std::string& input, const std::string& output) {

    // Abre o arquivo para leitura em modo binário
    // Reduz erros dee tradução entre SO
    std::ifstream inFile(input, std::ios::in | std::ios::binary);
    std::ofstream outFile(output);

    if (!inFile || !outFile) {
        std::cerr << "Error" << std::endl;
        return;
    }

    std::unordered_map<int, std::vector<std::pair<int, float>>> usersData;
    std::string line;
    // Reserva memória antecipadamente da hash para usuários únicos
    // WARNING: Testar outros valores
    usersData.reserve(500000); 

    std::getline(inFile, line);
    // Leitura linha por linha
    while (std::getline(inFile, line)) {
        // stringstream ajuda a manipular dados e trabalhar com tipos diferentes
        std::stringstream ss(line);
        std::string token;
        int user, movie;
        float rating;

        std::getline(ss, token, ',');
        user = std::stoi(token);

        std::getline(ss, token, ',');
        movie = std::stoi(token);

        std::getline(ss, token, ',');
        rating = std::stof(token);

        usersData[user].emplace_back(movie, rating);
    }

    // Escrita do novo CSV
    for (const auto& [user, movies] : usersData) {
        outFile << user;
        for (const auto& [movie, rating] : movies) {
            outFile << " " << movie << ":" << rating;
        }
        outFile << "\n";
    }

    usersData.clear();
    usersData.rehash(0);
    std::cout << "Process done." << std::endl;
}


int main()
{
    processCSV("datasets/ratings.csv", "datasets/output.csv");
    return 0;
}