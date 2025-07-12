#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <cstddef> // Para size_t

// Caminhos dos arquivos (ajuste para os locais reais dos seus arquivos)
// Assumindo que o executável roda da raiz do projeto
const std::string RATINGS_CSV_PATH = "dataset/ratings.csv";
const std::string MOVIES_CSV_PATH = "dataset/movies.csv";
const std::string FILTERED_DATASET_PATH = "src/filtered_dataset.dat";
const std::string EXPLORE_USERS_PATH = "src/explore.dat";
const std::string OUTPUT_RECOMMENDATIONS_PATH = "src/output.dat";

// Parâmetros de Recomendação e Filtragem
const int MIN_RATINGS_PER_ENTITY = 50;
const int K_NEIGHBORS = 10;
const size_t NUM_RANDOM_USERS_TO_EXPLORE = 50;
const int TOP_N_RECOMMENDATIONS = 5;

// Processamento de CSV
// Um buffer maior pode ajudar em I/O, mas consome mais RAM. 256MB é um bom começo.
const size_t CSV_READ_BUFFER_SIZE = 700 * 1024 * 1024; 
// Pré-alocar ajuda a evitar rehashes de mapa, um grande ganho de performance.
// Estes números são baseados no dataset MovieLens 25M após filtragem.
const size_t NUM_EXPECTED_UNIQUE_USERS = 163000;
const size_t NUM_EXPECTED_UNIQUE_MOVIES = 62500;

// Parâmetros LSH
const int NUM_LSH_TABLES = 5;
const int NUM_HYPERPLANES_PER_TABLE = 16; // k, o número de bits no hash. Deve ser <= 64

#endif // CONFIG_HPP